# Issue #14: Deadlock bei parallelen FTP-Dateiübertragungen

## Übersicht

Der FTP-Server bleibt bei parallelen Dateiübertragungen hängen. Das Problem tritt auf, wenn mehrere Dateien gleichzeitig hochgeladen werden (z.B. 5+ parallele Uploads).

## Symptom

Die Debug-Ausgabe zeigt, dass der Server bei der 5. parallelen Datei stecken bleibt:

```
DEBUG 1-5
DEBUG 2-5
--> Stuck at session->established_m.lock();
```

---

## Exkurs: shared_mutex und Lock-Typen

### Was ist ein shared_mutex?

Ein `shared_mutex` ist ein Mutex, der zwei Arten von Sperren unterstützt:

```mermaid
flowchart TB
    subgraph SharedMutex[shared_mutex]
        direction TB
        EX[Exklusiv-Modus<br/>unique_lock]
        SH[Shared-Modus<br/>shared_lock]
    end

    subgraph Exklusiv[Exklusiv-Modus]
        E1[Nur EIN Thread gleichzeitig]
        E2[Blockiert ALLE anderen]
        E3[Für Schreibzugriffe]
    end

    subgraph Shared[Shared-Modus]
        S1[MEHRERE Threads gleichzeitig]
        S2[Blockiert nur unique_lock]
        S3[Für Lesezugriffe]
    end

    EX --> Exklusiv
    SH --> Shared

    style EX fill:#ff9999
    style SH fill:#99ff99
```

### Kompatibilitätsmatrix

| Thread A hält | Thread B will shared_lock | Thread B will unique_lock |
|---------------|---------------------------|---------------------------|
| **shared_lock** | ✅ Erlaubt | ❌ Blockiert |
| **unique_lock** | ❌ Blockiert | ❌ Blockiert |

### Wichtig für dieses Issue

```mermaid
flowchart LR
    subgraph Erlaubt[Erlaubt: Mehrere shared_locks]
        T1[Thread 1<br/>shared_lock]
        T2[Thread 2<br/>shared_lock]
        T3[Thread 3<br/>shared_lock]
    end

    SM[session_m]
    T1 -->|shared| SM
    T2 -->|shared| SM
    T3 -->|shared| SM

    style Erlaubt fill:#90EE90
```

**Das bedeutet:** Wenn Thread A einen `shared_lock` auf `session_m` hält und Thread B ebenfalls einen `shared_lock` möchte, wird Thread B **NICHT blockiert**!

---

## Exkurs: Dynamisch erzeugte Mutexe

### Session-Struktur mit eigenen Mutexen

Jede `Session` wird dynamisch für jeden verbundenen Client erstellt und enthält **eigene Mutex-Instanzen**:

```cpp
struct Session {
    // ... andere Felder ...
    std::mutex established_m;    // Pro Session!
    std::mutex processed_m;      // Pro Session!
    std::mutex closed_m;         // Pro Session!
    std::shared_mutex modify_m;  // Pro Session!
};
```

```mermaid
flowchart TB
    subgraph Global[Globale Mutexe]
        SM[session_m<br/>shared_mutex]
        PM[tcpPort_m<br/>mutex]
    end

    subgraph Sessions[Session-Map: activeSessions]
        subgraph S1[Session Client 1]
            E1[established_m]
            P1[processed_m]
            C1[closed_m]
            M1[modify_m]
        end

        subgraph S2[Session Client 2]
            E2[established_m]
            P2[processed_m]
            C2[closed_m]
            M2[modify_m]
        end

        subgraph S3[Session Client 3]
            E3[established_m]
            P3[processed_m]
            C3[closed_m]
            M3[modify_m]
        end
    end

    SM -->|schützt Zugriff auf| Sessions

    style Global fill:#ffffcc
    style S1 fill:#ccffcc
    style S2 fill:#ccccff
    style S3 fill:#ffcccc
```

### Was bedeutet das?

1. **Isolation**: Client 1's `established_m` ist eine **komplett andere Mutex-Instanz** als Client 2's `established_m`
2. **Keine Cross-Session-Blockierung**: Ein Lock auf `session1->established_m` blockiert **niemals** `session2->established_m`
3. **Nur globale Mutexe** können Sessions gegenseitig blockieren: `session_m` und `tcpPort_m`

### Lebenszyklus dynamischer Mutexe

```mermaid
sequenceDiagram
    participant C as Client verbindet
    participant S as FtpServer
    participant M as Session-Map

    C->>S: TCP-Verbindung
    S->>S: on_newClient(clientId)
    S->>M: unique_lock(session_m)
    Note over M: Map gesperrt

    create participant Sess as Neue Session
    S->>Sess: make_unique<Session>()
    Note over Sess: established_m wird erstellt<br/>processed_m wird erstellt<br/>closed_m wird erstellt<br/>modify_m wird erstellt

    S->>M: activeSessions[clientId] = session
    S->>M: unlock(session_m)
    Note over M: Map freigegeben
```

### Wichtig: Pointer auf Mutexe in Lambdas

Im Code werden **Pointer auf die Session-Mutexe** in Lambdas gespeichert:

```cpp
// In on_msg_modePassive (Zeile 428-431):
mutex *p_established_m{&session->established_m};  // Pointer auf Session-Mutex!
mutex *p_processed_m{&session->processed_m};
mutex *p_closed_m{&session->closed_m};
```

```mermaid
flowchart LR
    subgraph Lambda[Lambda Closure]
        PE[p_established_m*]
        PP[p_processed_m*]
        PC[p_closed_m*]
    end

    subgraph Session[Session Objekt]
        EM[established_m]
        PM[processed_m]
        CM[closed_m]
    end

    PE -->|zeigt auf| EM
    PP -->|zeigt auf| PM
    PC -->|zeigt auf| CM

    style Lambda fill:#ffeecc
    style Session fill:#ccffee
```

**Gefahr:** Wenn die Session gelöscht oder verschoben wird, zeigen die Pointer auf ungültigen Speicher!

---

## Beteiligte Komponenten

### Mutex-Übersicht

```mermaid
classDiagram
    class FtpServer {
        +shared_mutex session_m
        +mutex tcpPort_m
        +map~int,Session~ activeSessions
    }

    class Session {
        +mutex established_m
        +mutex processed_m
        +mutex closed_m
        +shared_mutex modify_m
        +TcpServer tcpData
        +int dataClientId
    }

    FtpServer "1" --> "*" Session : enthält

    note for FtpServer "session_m: Global, schützt activeSessions-Map\ntcpPort_m: Global, schützt Port-Suche"
    note for Session "Alle Mutexe sind Session-lokal!\nKeine Cross-Session-Blockierung möglich"
```

---

## Korrigierte Deadlock-Analyse

Da jede Session ihre eigenen Mutexe hat, kann der Deadlock **nur innerhalb einer einzelnen Session** entstehen - zwischen dem STOR-Handler-Thread und dem Data-Connection-Lambda-Thread **desselben Clients**.

### Der Deadlock-Mechanismus

```mermaid
sequenceDiagram
    participant C as FTP Client 5
    participant STOR as STOR-Handler<br/>(Thread A)
    participant DC as Data-Connection<br/>(Thread B)

    Note over C,DC: Beide Threads arbeiten mit derselben Session!

    C->>STOR: PASV
    STOR->>STOR: established_m.try_lock()
    Note over STOR: established_m GESPERRT<br/>(für Client 5)

    STOR-->>C: 227 Entering Passive Mode

    Note over C: Client sendet STOR<br/>BEVOR Datenverbindung steht!
    C->>STOR: STOR file.txt

    STOR->>STOR: session_m.shared_lock() ✅
    STOR->>STOR: modify_m.shared_lock() ✅
    STOR->>STOR: established_m.lock()
    Note over STOR: WARTET auf established_m...<br/>(derselbe Mutex wie oben!)

    C->>DC: Verbinde zu Data-Server

    DC->>DC: session_m.shared_lock() ✅
    DC->>DC: established_m.unlock()
    Note over DC: Entsperrt established_m

    STOR->>STOR: established_m.lock() ✅
    Note over STOR: Kann weitermachen!
```

### Aber warum blockiert es dann?

Das Problem liegt im **Timing** und möglicherweise in der **Reihenfolge der Mutex-Operationen**:

```mermaid
flowchart TD
    subgraph Problem[Mögliches Problem]
        A[STOR-Handler wartet auf established_m]
        B[Data-Connection-Lambda wird nie aufgerufen?]
        C[Oder: Lambda-Aufruf blockiert woanders?]
    end

    A --> Q1{Wird Lambda<br/>überhaupt aufgerufen?}
    Q1 -->|Nein| B
    Q1 -->|Ja| Q2{Blockiert Lambda<br/>vor unlock?}
    Q2 -->|Ja| C

    style A fill:#ff9999
    style B fill:#ffcc99
    style C fill:#ffcc99
```

---

## Hypothese: Das Lambda wird nie aufgerufen

Schauen wir uns an, wann das Lambda aufgerufen wird:

```cpp
dataServer->setCreateForwardStream([...](const int dataClientId) -> DynamicOstream<...>* {
    shared_lock<shared_mutex> lck_session{*p_session_m};
    *p_dataClientId = dataClientId;
    *pp_incomingStreamFwd = new DynamicOstream<...>();
    p_established_m->unlock();  // <-- Entsperrt established_m
    return *pp_incomingStreamFwd;
});
```

Das Lambda wird aufgerufen, wenn ein **Client sich mit dem Data-Server verbindet**.

### Timing-Problem

```mermaid
sequenceDiagram
    participant C as FTP Client
    participant CS as Control-Server
    participant DS as Data-Server

    C->>CS: PASV
    CS->>DS: Starte Data-Server auf Port X
    CS->>CS: established_m.try_lock()
    CS-->>C: 227 Entering Passive Mode (Port X)

    Note over C: Client sendet STOR sofort!
    C->>CS: STOR file.txt

    CS->>CS: established_m.lock()
    Note over CS: WARTET...<br/>Client hat sich noch nicht<br/>zum Data-Server verbunden!

    Note over C: Erst JETZT verbindet<br/>sich Client zum Data-Server
    C->>DS: TCP Connect zu Port X

    DS->>DS: Lambda aufrufen
    DS->>DS: established_m.unlock()

    CS->>CS: established_m.lock() ✅
```

**Das sollte eigentlich funktionieren!** Der STOR-Handler wartet einfach, bis der Client sich verbindet.

---

## Neue Hypothese: tcpPort_m blockiert

In `on_msg_modePassive` gibt es einen `lock_guard` auf `tcpPort_m`:

```cpp
// Zeile 410
lock_guard<mutex> lck_port{tcpPort_m};
```

Dieser Lock wird **vor** dem Starten des Data-Servers genommen und erst am Ende des Scopes freigegeben.

```mermaid
flowchart TD
    subgraph PASV["on_msg_modePassive Scope"]
        L1[lock_guard tcpPort_m]
        L2[...]
        L3[dataServer->start]
        L4[established_m.try_lock]
        L5[session->tcpData = dataServer]
        L6["} // tcpPort_m wird hier freigegeben"]
    end

    L1 --> L2 --> L3 --> L4 --> L5 --> L6

    style L1 fill:#ffcc99
    style L6 fill:#99ff99
```

### Das Problem bei parallelen PASV-Befehlen

```mermaid
sequenceDiagram
    participant C1 as Client 1
    participant C2 as Client 2
    participant CS as Control-Server

    C1->>CS: PASV
    CS->>CS: tcpPort_m.lock()
    Note over CS: Client 1 hält tcpPort_m

    C2->>CS: PASV
    CS->>CS: tcpPort_m.lock()
    Note over CS: Client 2 WARTET auf tcpPort_m

    Note over CS: Client 1 startet Data-Server...
    Note over CS: Client 1 gibt tcpPort_m frei

    Note over CS: Client 2 kann jetzt PASV verarbeiten
```

**Das erklärt aber nicht den Deadlock**, da `tcpPort_m` sequentiell verarbeitet wird.

---

## Kritische Erkenntnis: Data-Server-Start im tcpPort_m-Lock

Das eigentliche Problem könnte sein:

```mermaid
flowchart TD
    subgraph Scope["tcpPort_m Lock-Scope (Zeile 410-461)"]
        A[tcpPort_m.lock]
        B[getFreePort]
        C[session_m.shared_lock]
        D[modify_m.shared_lock → unique_lock]
        E[dataServer->start]
        F[established_m.try_lock]
        G[session->tcpData = dataServer]
    end

    A --> B --> C --> D --> E --> F --> G

    H["} // Scope Ende"]
    G --> H

    style A fill:#ff9999
    style H fill:#99ff99
    style E fill:#ffcc99
```

Der Data-Server wird **innerhalb** des `tcpPort_m`-Locks gestartet. Wenn das Lambda aufgerufen wird, versucht es `session_m.shared_lock()` zu bekommen.

**Aber:** Der PASV-Handler hält bereits `session_m.shared_lock()` - das ist kein Problem, da beides `shared_lock` ist.

---

## Die wahrscheinlichste Ursache

Nach genauerer Analyse: **Das Lambda wird möglicherweise im falschen Kontext aufgerufen** oder **der Data-Server ist noch nicht bereit**.

Schauen wir uns die Reihenfolge an:

```mermaid
sequenceDiagram
    participant PASV as PASV-Handler
    participant DS as Data-Server
    participant STOR as STOR-Handler

    PASV->>DS: dataServer->start()
    Note over DS: Server startet asynchron?

    PASV->>PASV: established_m.try_lock()

    PASV->>PASV: session->tcpData = move(dataServer)
    Note over PASV: PASV-Handler fertig,<br/>tcpPort_m wird freigegeben

    STOR->>STOR: session_m.shared_lock()
    STOR->>STOR: modify_m.shared_lock()
    STOR->>STOR: established_m.lock()
    Note over STOR: WARTET...

    Note over DS: Data-Server ist bereit?<br/>Client verbindet sich

    DS->>DS: Lambda: session_m.shared_lock()
    DS->>DS: Lambda: established_m.unlock()

    STOR->>STOR: established_m.lock() ✅
```

### Mögliche Race Condition

Was passiert, wenn der **Client sich sehr schnell verbindet** - noch während PASV läuft?

```mermaid
sequenceDiagram
    participant PASV as PASV-Handler
    participant DS as Data-Server
    participant C as FTP Client

    PASV->>DS: dataServer->start()
    Note over DS: Server lauscht auf Port

    PASV-->>C: 227 Entering Passive Mode
    Note over PASV: Nachricht gesendet

    C->>DS: TCP Connect (sehr schnell!)

    DS->>DS: Lambda aufrufen
    DS->>DS: session_m.shared_lock() ✅
    DS->>DS: established_m.unlock()
    Note over DS: FEHLER: established_m<br/>wurde noch nicht gesperrt!

    PASV->>PASV: established_m.try_lock()
    Note over PASV: Lock erfolgreich,<br/>aber Lambda hat schon unlock gemacht!

    rect rgb(255, 200, 200)
        Note over PASV,DS: Undefined Behavior:<br/>unlock() auf nicht-gesperrtem Mutex!
    end
```

---

## Fazit: Race Condition bei schneller Verbindung

Das Problem ist eine **Race Condition**:

1. `dataServer->start()` wird aufgerufen (Zeile 451)
2. Die Antwort `227 Entering Passive Mode` wird gesendet (Zeile 486)
3. **Erst danach** wird `established_m.try_lock()` aufgerufen (Zeile 457)

Wenn der Client sich **zwischen Schritt 1 und 3** verbindet, ruft das Lambda `established_m.unlock()` auf einem **nicht gesperrten Mutex** auf!

```mermaid
flowchart TD
    subgraph Richtige_Reihenfolge[Erwartete Reihenfolge]
        R1[1. established_m.try_lock]
        R2[2. dataServer->start]
        R3[3. Client verbindet sich]
        R4[4. Lambda: established_m.unlock]
        R1 --> R2 --> R3 --> R4
    end

    subgraph Tatsächliche_Reihenfolge[Tatsächliche Reihenfolge im Code]
        T1[1. dataServer->start]
        T2[2. established_m.try_lock]
        T3[3. Client verbindet sich]
        T4[4. Lambda: established_m.unlock]
        T1 --> T2
        T1 -.->|Race!| T3
        T3 --> T4
    end

    style R1 fill:#99ff99
    style T1 fill:#ff9999
    style T3 fill:#ffcc99
```

---

## Zusammenfassung

| Aspekt | Beschreibung |
|--------|--------------|
| **Problem** | Deadlock/Undefined Behavior bei parallelen Dateiübertragungen |
| **Symptom** | Server bleibt bei `established_m.lock()` hängen |
| **Ursache** | Race Condition: `dataServer->start()` wird **vor** `established_m.try_lock()` aufgerufen |
| **Folge** | Lambda kann `unlock()` auf nicht-gesperrtem Mutex aufrufen → Undefined Behavior |
| **Betroffene Stelle** | `FtpServer.cpp:451-459` |
| **Session-Mutexe** | Sind pro Client isoliert, keine Cross-Session-Blockierung |

---

## Lösung

Die Reihenfolge muss korrigiert werden:

```mermaid
flowchart LR
    subgraph Vorher[Aktueller Code - FALSCH]
        V1[dataServer->start] --> V2[established_m.try_lock]
    end

    subgraph Nachher[Korrigiert - RICHTIG]
        N1[established_m.try_lock] --> N2[dataServer->start]
    end

    style V1 fill:#ff9999
    style N1 fill:#99ff99
```

**Konkret:** Zeile 457 (`established_m.try_lock()`) muss **vor** Zeile 451 (`dataServer->start()`) verschoben werden.

---

## Mögliche Lösungsansätze

```mermaid
flowchart LR
    subgraph Lösungen
        L1[1. Lock-Reihenfolge<br/>korrigieren]
        L2[2. Condition Variable<br/>statt Mutex-Signaling]
        L3[3. atomic bool<br/>für Status]
    end

    L1 --> D1[try_lock VOR start]
    L2 --> D2[CV für established-Signal]
    L3 --> D3[atomic für Status-Flag]
```

1. **Lock-Reihenfolge korrigieren**: `established_m.try_lock()` **vor** `dataServer->start()` aufrufen
2. **Condition Variable nutzen**: Statt Mutex-Missbrauch eine `condition_variable` für Signaling verwenden
3. **Atomic Flag**: `std::atomic<bool>` für den `established`-Status verwenden
