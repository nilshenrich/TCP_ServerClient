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

    note for Session "session_m: Zugriff auf Session-Map
    modify_m: Änderungen an Session-Daten
    established_m: Datenverbindung etabliert
    processed_m: Datenverarbeitung fertig
    closed_m: Verbindung geschlossen"
```

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

## Fehlerhafter Ablauf (Deadlock)

Bei schnellen parallelen Anfragen kann folgendes passieren:

```mermaid
sequenceDiagram
    participant C as FTP Client
    participant CS as Control-Server<br/>(Thread A)
    participant DS as Data-Server<br/>(Thread B)

    C->>CS: 1. PASV
    CS->>DS: 2. established_m.try_lock()
    Note over DS: Mutex gesperrt
    CS-->>C: 3. 227 Entering Passive Mode

    Note over C: Client sendet STOR<br/>BEVOR Datenverbindung steht!
    C->>CS: 4. STOR file.txt (SCHNELL!)

    CS->>CS: 5. session_m.shared_lock()
    Note over CS: OK - session_m gehalten

    CS->>CS: 6. established_m.lock()
    Note over CS: WARTET...

    C->>DS: 7. Verbinde zu Data-Server

    DS->>DS: 8. Lambda: session_m.shared_lock()
    Note over DS: WARTET auf session_m...

    rect rgb(255, 200, 200)
        Note over CS,DS: DEADLOCK!<br/>Thread A wartet auf established_m (hält session_m)<br/>Thread B wartet auf session_m (will established_m entsperren)
    end
```

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
        shared_lock<shared_mutex> lck_session{*p_session_m}; // ← Braucht session_m!
        *p_dataClientId = dataClientId;
        *pp_incomingStreamFwd = new DynamicOstream<...>();
        p_established_m->unlock();  // ← Soll established_m entsperren
        return *pp_incomingStreamFwd;
    });
```

### 3. STOR-Befehl wartet auf `established_m`

**Datei:** `FtpServer.cpp:698-703`

```cpp
void FtpServer::on_msg_fileUpload(...) {
    shared_lock<shared_mutex> lck_session{session_m}; // ← Hält session_m!
    cout << "DEBUG 1-" << clientId << endl;
    unique_ptr<Session> &session{activeSessions.at(clientId)};
    shared_lock<shared_mutex> lck_session_modify{session->modify_m};
    cout << "DEBUG 2-" << clientId << endl;
    session->established_m.lock();  // ← WARTET auf established_m!
    cout << "DEBUG 3-" << clientId << endl;  // ← Wird nie erreicht
    ...
}
```

---

## Deadlock-Visualisierung

```mermaid
flowchart TB
    subgraph Ressourcen
        SM[session_m<br/>shared_mutex]
        EM[established_m<br/>mutex]
    end

    subgraph ThreadA[Thread A: STOR-Handler]
        A1[Hält: session_m]
        A2[Wartet auf: established_m]
    end

    subgraph ThreadB[Thread B: Data-Connection]
        B1[Wartet auf: session_m]
        B2[Will entsperren: established_m]
    end

    A1 -->|hält| SM
    A2 -.->|wartet| EM
    B1 -.->|wartet| SM
    B2 -->|will entsperren| EM

    style ThreadA fill:#ffcccc
    style ThreadB fill:#ccccff
    style SM fill:#ffffcc
    style EM fill:#ffffcc
```

### Zirkuläre Abhängigkeit

```mermaid
flowchart LR
    A[Thread A<br/>STOR-Handler] -->|wartet auf| EM[established_m]
    EM -->|wird entsperrt von| B[Thread B<br/>Data-Connection]
    B -->|wartet auf| SM[session_m]
    SM -->|gehalten von| A

    style A fill:#ff9999
    style B fill:#9999ff
    style EM fill:#ffff99
    style SM fill:#ffff99
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
        P3 --> P4[DEADLOCK]
        style P3 fill:#FFB6C1
        style P4 fill:#FF6347
    end
```

---

## Zusammenfassung

| Aspekt | Beschreibung |
|--------|--------------|
| **Problem** | Deadlock bei parallelen Dateiübertragungen |
| **Ursache** | Zirkuläre Mutex-Abhängigkeit zwischen `session_m` und `established_m` |
| **Auslöser** | STOR-Befehl wird empfangen, bevor die Datenverbindung etabliert ist |
| **Betroffene Methoden** | `on_msg_fileUpload` (Zeile 703), `setCreateForwardStream` Lambda (Zeile 439) |
| **Reproduzierbar** | Bei 5+ parallelen Uploads mit schnellem Client |

---

## Mögliche Lösungsansätze

```mermaid
flowchart LR
    subgraph Lösungen
        L1[1. Mutex-Reihenfolge ändern]
        L2[2. Condition Variable]
        L3[3. Lock-free Design]
        L4[4. Timeout mit Retry]
    end

    L1 --> D1[established_m VOR session_m sperren]
    L2 --> D2[Mit CV auf established warten]
    L3 --> D3[Atomare Operationen nutzen]
    L4 --> D4[try_lock mit Timeout]
```

1. **Mutex-Reihenfolge ändern**: `established_m` vor `session_m` sperren
2. **Condition Variable nutzen**: Statt direktem Lock auf `established_m` mit Condition Variable warten
3. **Lock-free Design**: Atomare Operationen für den `established`-Status verwenden
4. **Timeout mit Retry**: Lock-Versuch mit Timeout und Retry-Mechanismus
