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

```
┌─────────────────────────────────────────────────────────────┐
│                        Session                              │
├─────────────────────────────────────────────────────────────┤
│  session_m (shared_mutex)     - Zugriff auf Session-Map    │
│  modify_m  (shared_mutex)     - Änderungen an Session-Daten│
│  established_m (mutex)        - Datenverbindung etabliert  │
│  processed_m (mutex)          - Datenverarbeitung fertig   │
│  closed_m (mutex)             - Verbindung geschlossen     │
├─────────────────────────────────────────────────────────────┤
│  tcpData (TcpServer)          - Daten-Server für PASV      │
│  dataClientId (int)           - Client-ID der Datenverb.   │
│  incomingStreamFwd (Stream)   - Eingehende Daten           │
└─────────────────────────────────────────────────────────────┘
```

---

## Normaler Ablauf (funktioniert)

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│  FTP Client  │     │ Control-     │     │ Data-Server  │
│              │     │ Server       │     │ (PASV)       │
└──────┬───────┘     └──────┬───────┘     └──────┬───────┘
       │                    │                    │
       │  1. PASV           │                    │
       │───────────────────>│                    │
       │                    │                    │
       │                    │ 2. established_m   │
       │                    │    .try_lock()     │
       │                    │ ─ ─ ─ ─ ─ ─ ─ ─ ─>│
       │                    │                    │
       │  3. 227 Passive    │                    │
       │<───────────────────│                    │
       │                    │                    │
       │  4. Verbinde zu    │                    │
       │     Data-Server    │                    │
       │─────────────────────────────────────────>
       │                    │                    │
       │                    │ 5. established_m   │
       │                    │    .unlock()       │
       │                    │<─ ─ ─ ─ ─ ─ ─ ─ ─ │
       │                    │    (Lambda)        │
       │                    │                    │
       │  6. STOR file.txt  │                    │
       │───────────────────>│                    │
       │                    │                    │
       │                    │ 7. established_m   │
       │                    │    .lock()         │
       │                    │    (sofort OK!)    │
       │                    │                    │
       │  8. Dateiübertrag. │                    │
       │─────────────────────────────────────────>
       │                    │                    │
       ▼                    ▼                    ▼
```

---

## Fehlerhafter Ablauf (Deadlock)

Bei schnellen parallelen Anfragen kann folgendes passieren:

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│  FTP Client  │     │ Control-     │     │ Data-Server  │
│              │     │ Server       │     │ (PASV)       │
└──────┬───────┘     └──────┬───────┘     └──────┬───────┘
       │                    │                    │
       │  1. PASV           │                    │
       │───────────────────>│                    │
       │                    │                    │
       │                    │ 2. established_m   │
       │                    │    .try_lock()     │
       │                    │ ─ ─ ─ ─ ─ ─ ─ ─ ─>│
       │                    │    [GESPERRT]      │
       │                    │                    │
       │  3. 227 Passive    │                    │
       │<───────────────────│                    │
       │                    │                    │
       │  4. STOR file.txt  │                    │
       │───────────────────>│  ← Client sendet  │
       │    (SCHNELL!)      │    STOR BEVOR die │
       │                    │    Datenverbindung│
       │                    │    steht!         │
       │                    │                    │
       │                    │ 5. session_m      │
       │                    │    .shared_lock() │
       │                    │    [OK]           │
       │                    │                    │
       │                    │ 6. established_m  │
       │                    │    .lock()        │
       │                    │    [WARTET...]    │
       │                    │         │         │
       │                    │         │         │
       │  7. Verbinde zu    │         │         │
       │     Data-Server    │         │         │
       │───────────────────────────────────────>│
       │                    │         │         │
       │                    │         │  8. Lambda will
       │                    │         │     session_m
       │                    │         │     .shared_lock()
       │                    │         │         │
       │                    │         │    [WARTET...]
       │                    │         │         │
       │                    │         │         │
       │                    ▼         ▼         │
       │               ╔═══════════════════╗    │
       │               ║    DEADLOCK!      ║    │
       │               ║                   ║    │
       │               ║ Thread A wartet   ║    │
       │               ║ auf established_m ║    │
       │               ║ (hält session_m)  ║    │
       │               ║                   ║    │
       │               ║ Thread B wartet   ║    │
       │               ║ auf session_m     ║    │
       │               ║ (will established ║    │
       │               ║  _m entsperren)   ║    │
       │               ╚═══════════════════╝    │
       ▼                                        ▼
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

```
                    ┌─────────────────────┐
                    │                     │
          ┌─────────▼─────────┐           │
          │                   │           │
          │    session_m      │           │
          │   (shared_mutex)  │           │
          │                   │           │
          └─────────┬─────────┘           │
                    │                     │
        ┌───────────┴───────────┐         │
        │                       │         │
        ▼                       ▼         │
┌───────────────┐       ┌───────────────┐ │
│   Thread A    │       │   Thread B    │ │
│ (STOR-Handler)│       │ (Data-Conn)   │ │
├───────────────┤       ├───────────────┤ │
│ HÄLT:         │       │ WARTET AUF:   │ │
│ session_m     │       │ session_m     │─┘
│               │       │               │
│ WARTET AUF:   │       │ WILL:         │
│ established_m │◄──────│ established_m │
│               │       │ entsperren    │
└───────────────┘       └───────────────┘

        ══════════════════════════════
        ║  Zirkuläre Abhängigkeit!   ║
        ║                            ║
        ║  A wartet auf B            ║
        ║  B wartet auf A            ║
        ══════════════════════════════
```

---

## Warum tritt das Problem nur bei parallelen Uploads auf?

```
Zeit ──────────────────────────────────────────────────────────►

Einzelner Upload (funktioniert):
┌─────┐ ┌─────────────────┐ ┌─────┐ ┌────────────────────────┐
│PASV │ │ Datenverbindung │ │STOR │ │ Dateiübertragung       │
└─────┘ └─────────────────┘ └─────┘ └────────────────────────┘
         ▲                   ▲
         │                   │
         └───────────────────┴── Genug Zeit dazwischen


Parallele Uploads (Deadlock-Risiko):
Client 1: ┌─────┐┌─────┐
          │PASV ││STOR │    ← Kein Warten!
          └─────┘└─────┘

Client 2: ┌─────┐┌─────┐
          │PASV ││STOR │
          └─────┘└─────┘

Client 3: ┌─────┐┌─────┐
          │PASV ││STOR │
          └─────┘└─────┘
              ▲
              │
              └── STOR kommt an, BEVOR Datenverbindung steht!
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

1. **Mutex-Reihenfolge ändern**: `established_m` vor `session_m` sperren
2. **Condition Variable nutzen**: Statt direktem Lock auf `established_m` mit Condition Variable warten
3. **Lock-free Design**: Atomare Operationen für den `established`-Status verwenden
4. **Timeout mit Retry**: Lock-Versuch mit Timeout und Retry-Mechanismus
