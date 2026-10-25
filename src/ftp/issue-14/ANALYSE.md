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

## Beteiligte Komponenten

### Mutex-Hierarchie in der Session

```mermaid
classDiagram
    class Session {
        +shared_mutex session_m
        +shared_mutex modify_m
        +mutex established_m
        +mutex processed_m
        +mutex closed_m
        ---
        +TcpServer tcpData
        +int dataClientId
        +DynamicOstream incomingStreamFwd
    }

    note for Session "session_m: Zugriff auf Session-Map (shared_mutex)
    modify_m: Änderungen an Session-Daten (shared_mutex)
    established_m: Datenverbindung etabliert (mutex)
    processed_m: Datenverarbeitung fertig (mutex)
    closed_m: Verbindung geschlossen (mutex)"
```

### Lock-Verwendung im Code

| Stelle | Mutex | Lock-Typ | Blockiert andere shared_locks? |
|--------|-------|----------|-------------------------------|
| `on_msg_fileUpload:698` | `session_m` | `shared_lock` | ❌ Nein |
| `on_msg_fileUpload:701` | `modify_m` | `shared_lock` | ❌ Nein |
| `on_msg_fileUpload:703` | `established_m` | `.lock()` | ✅ Ja (normaler mutex) |
| `on_msg_fileUpload:711` | `modify_m` | `unique_lock` | ✅ Ja |
| Lambda:439 | `session_m` | `shared_lock` | ❌ Nein |

---

## Korrigierte Deadlock-Analyse

Da `session_m` nur mit `shared_lock` gesperrt wird, kann der Deadlock **nicht** durch `session_m` entstehen. Das Problem liegt woanders:

### Der wahre Deadlock-Mechanismus

```mermaid
sequenceDiagram
    participant C as FTP Client
    participant CS as Control-Server<br/>(Thread A)
    participant DS as Data-Server<br/>(Thread B)

    C->>CS: 1. PASV
    CS->>DS: 2. established_m.try_lock()
    Note over DS: established_m GESPERRT

    CS-->>C: 3. 227 Entering Passive Mode

    Note over C: Client sendet STOR<br/>BEVOR Datenverbindung steht!
    C->>CS: 4. STOR file.txt (SCHNELL!)

    CS->>CS: 5. session_m.shared_lock() ✅
    CS->>CS: 6. modify_m.shared_lock() ✅

    CS->>CS: 7. established_m.lock()
    Note over CS: WARTET auf established_m...<br/>(wurde in PASV gesperrt)

    C->>DS: 8. Verbinde zu Data-Server

    DS->>DS: 9. Lambda wird aufgerufen
    DS->>DS: 10. session_m.shared_lock() ✅
    DS->>DS: 11. established_m.unlock()
    Note over DS: ✅ Sollte funktionieren!
```

### Aber warum blockiert es dann?

Das Problem ist subtiler. Schauen wir uns den Code genauer an:

```mermaid
flowchart TD
    subgraph PASV["on_msg_modePassive (Zeile 409-461)"]
        P1[tcpPort_m.lock]
        P2[session_m.shared_lock]
        P3[modify_m.shared_lock]
        P4[modify_m.unlock]
        P5[modify_m.unique_lock]
        P6[established_m.try_lock]
        P7[Server starten]
        P1 --> P2 --> P3 --> P4 --> P5 --> P6 --> P7
    end

    subgraph STOR["on_msg_fileUpload (Zeile 698-746)"]
        S1[session_m.shared_lock]
        S2[modify_m.shared_lock]
        S3[established_m.lock - WARTET]
        S4[modify_m.unlock]
        S5[modify_m.unique_lock]
        S1 --> S2 --> S3 --> S4 --> S5
    end

    style S3 fill:#ff6666
    style P6 fill:#ffcc66
```

### Das eigentliche Problem: `modify_m` unique_lock

Der Deadlock entsteht durch die Kombination von:

1. **Thread A (STOR)**: Hält `modify_m` als `shared_lock`, wartet auf `established_m`
2. **Thread C (anderer PASV/STOR)**: Will `modify_m` als `unique_lock`

```mermaid
sequenceDiagram
    participant A as Thread A<br/>(STOR Client 5)
    participant B as Thread B<br/>(Data-Conn Client 5)
    participant C as Thread C<br/>(PASV/STOR Client 6)

    Note over A: session_m.shared_lock ✅
    Note over A: modify_m.shared_lock ✅
    A->>A: established_m.lock()
    Note over A: WARTET...

    Note over C: session_m.shared_lock ✅
    C->>C: modify_m.unique_lock()
    Note over C: WARTET auf A's shared_lock!

    B->>B: Lambda: session_m.shared_lock ✅
    Note over B: Kann established_m.unlock() ausführen

    rect rgb(255, 200, 200)
        Note over A,C: Problem: A wartet auf B<br/>C blockiert möglicherweise B indirekt
    end
```

---

## Hypothese: Writer-Starvation Problem

Bei `shared_mutex` gibt es ein bekanntes Problem: **Writer-Starvation** oder **Reader-Writer-Fairness**.

```mermaid
flowchart TD
    subgraph Problem[Writer wartet auf Leser]
        R1[Reader 1 - shared_lock aktiv]
        R2[Reader 2 - shared_lock aktiv]
        W1[Writer - unique_lock WARTET]
        R3[Reader 3 - shared_lock WARTET?]
    end

    R1 -->|hält| SM[modify_m]
    R2 -->|hält| SM
    W1 -.->|wartet auf| SM
    R3 -.->|wartet hinter Writer?| SM

    style W1 fill:#ff9999
    style R3 fill:#ffcc99
```

**Je nach Implementierung** kann ein wartender `unique_lock` neue `shared_lock`-Anfragen blockieren, um Writer-Starvation zu vermeiden. Das könnte erklären, warum das Lambda blockiert!

---

## Detaillierte Code-Analyse

### 1. PASV-Befehl sperrt `established_m`

**Datei:** `FtpServer.cpp:456-459`

```cpp
bool _;
_ = p_established_m->try_lock(); // Sperrt established_m
_ = p_processed_m->try_lock();
_ = p_closed_m->try_lock();
```

### 2. Lambda-Callback soll `established_m` entsperren

**Datei:** `FtpServer.cpp:437-444`

```cpp
dataServer->setCreateForwardStream(
    [pp_incomingStreamFwd, p_established_m, p_session_m, p_dataClientId]
    (const int dataClientId) -> DynamicOstream<...>* {
        shared_lock<shared_mutex> lck_session{*p_session_m}; // shared_lock auf session_m
        *p_dataClientId = dataClientId;
        *pp_incomingStreamFwd = new DynamicOstream<...>();
        p_established_m->unlock();  // Entsperrt established_m
        return *pp_incomingStreamFwd;
    });
```

### 3. STOR-Befehl wartet auf `established_m`

**Datei:** `FtpServer.cpp:698-711`

```cpp
void FtpServer::on_msg_fileUpload(...) {
    shared_lock<shared_mutex> lck_session{session_m};         // shared_lock
    unique_ptr<Session> &session{activeSessions.at(clientId)};
    shared_lock<shared_mutex> lck_session_modify{session->modify_m}; // shared_lock

    session->established_m.lock();  // WARTET hier!

    // Nach established_m.lock():
    lck_session_modify.unlock();
    unique_lock<shared_mutex> lck_session_modify_unique{session->modify_m}; // unique_lock!
    ...
}
```

---

## Mögliche Deadlock-Szenarien

### Szenario 1: Parallele Sessions mit shared modify_m

```mermaid
flowchart LR
    subgraph Client5[Client 5 - STOR]
        A1[modify_m shared_lock ✅]
        A2[established_m.lock WARTET]
    end

    subgraph Client6[Client 6 - irgendein Befehl]
        B1[modify_m unique_lock WARTET]
    end

    subgraph DataConn[Data Connection Lambda]
        C1[session_m shared_lock]
        C2[established_m.unlock]
    end

    A1 -->|blockiert| B1
    B1 -.->|blockiert evtl.| C1
    C2 -.->|würde entsperren| A2

    style A2 fill:#ff6666
    style B1 fill:#ffcc66
```

### Szenario 2: tcpPort_m Mutex

In `on_msg_modePassive` wird `tcpPort_m` gelockt (Zeile 410):

```cpp
lock_guard<mutex> lck_port{tcpPort_m};
```

Wenn mehrere PASV-Befehle gleichzeitig verarbeitet werden, warten sie hier aufeinander.

---

## Normaler Ablauf (funktioniert)

```mermaid
sequenceDiagram
    participant C as FTP Client
    participant CS as Control-Server
    participant DS as Data-Server (PASV)

    C->>CS: 1. PASV
    CS->>DS: 2. established_m.try_lock()
    Note over DS: Mutex gesperrt
    CS-->>C: 3. 227 Entering Passive Mode

    C->>DS: 4. Verbinde zu Data-Server
    DS->>CS: 5. established_m.unlock() (Lambda)
    Note over CS: Mutex freigegeben

    C->>CS: 6. STOR file.txt
    CS->>CS: 7. established_m.lock()
    Note over CS: Sofort OK!

    C->>DS: 8. Dateiübertragung
    Note over C,DS: Upload erfolgreich
```

---

## Warum tritt das Problem nur bei parallelen Uploads auf?

```mermaid
gantt
    title Timing-Vergleich
    dateFormat X
    axisFormat %s

    section Einzelner Upload (OK)
    PASV           :a1, 0, 1
    Datenverbindung:a2, 1, 3
    STOR           :a3, 3, 4
    Übertragung    :a4, 4, 6

    section Parallele Uploads (Deadlock)
    PASV           :b1, 0, 1
    STOR (zu früh!):crit, b2, 1, 2
    Datenverbindung:b3, 2, 4
```

### Erklärung

```mermaid
flowchart TD
    subgraph Einzeln[Einzelner Upload - Funktioniert]
        S1[PASV] --> S2[Datenverbindung etabliert]
        S2 --> S3[STOR]
        S3 --> S4[Übertragung]
        style S2 fill:#90EE90
    end

    subgraph Parallel[Parallele Uploads - Deadlock-Risiko]
        P1[PASV] --> P2[STOR kommt sofort]
        P2 --> P3[Datenverbindung noch nicht da!]
        P3 --> P4[Mutex-Konflikte durch andere Clients]
        P4 --> P5[DEADLOCK]
        style P3 fill:#FFB6C1
        style P5 fill:#FF6347
    end
```

Bei parallelen Uploads konkurrieren mehrere Threads um dieselben Mutexe, was zu komplexen Warteabhängigkeiten führt.

---

## Zusammenfassung

| Aspekt | Beschreibung |
|--------|--------------|
| **Problem** | Deadlock bei parallelen Dateiübertragungen |
| **Symptom** | Server bleibt bei `established_m.lock()` hängen |
| **Ursache** | Komplexe Mutex-Interaktionen zwischen `established_m`, `modify_m` und möglicherweise `tcpPort_m` bei parallelen Requests |
| **shared_mutex Verhalten** | `shared_lock` erlaubt mehrere gleichzeitige Leser, aber `unique_lock` blockiert alle |
| **Betroffene Methoden** | `on_msg_fileUpload` (Zeile 703), `on_msg_modePassive` (Zeile 456), Lambda (Zeile 439) |
| **Reproduzierbar** | Bei 5+ parallelen Uploads mit schnellem Client |

---

## Mögliche Lösungsansätze

```mermaid
flowchart LR
    subgraph Lösungen
        L1[1. Lock-Reihenfolge<br/>standardisieren]
        L2[2. Condition Variable<br/>statt Mutex-Signaling]
        L3[3. Lock-free Design<br/>mit atomics]
        L4[4. Timeout mit Retry]
        L5[5. Session-spezifische<br/>Mutexe isolieren]
    end

    L1 --> D1[Immer gleiche Reihenfolge]
    L2 --> D2[CV für established-Signal]
    L3 --> D3[atomic bool für Status]
    L4 --> D4[try_lock_for mit Retry]
    L5 --> D5[Weniger globale Locks]
```

1. **Lock-Reihenfolge standardisieren**: Alle Threads müssen Mutexe in derselben Reihenfolge sperren
2. **Condition Variable nutzen**: Statt direktem Lock auf `established_m` mit `condition_variable` warten
3. **Lock-free Design**: Atomare Operationen (`std::atomic<bool>`) für den `established`-Status verwenden
4. **Timeout mit Retry**: `try_lock_for()` mit Timeout und Retry-Mechanismus
5. **Session-spezifische Mutexe isolieren**: Weniger Abhängigkeiten zwischen globalen und session-lokalen Locks

---

## Weiterführende Untersuchung

Um den genauen Deadlock zu identifizieren, wäre hilfreich:

1. **Thread-Dump** während des Deadlocks (z.B. mit `gdb`)
2. **Lock-Order-Checker** wie ThreadSanitizer (`-fsanitize=thread`)
3. **Logging** aller Lock/Unlock-Operationen mit Thread-ID und Zeitstempel
