# CodeX-PBL — IPC Addition Demo

A multi-process system simulation demonstrating POSIX Inter-Process Communication (IPC) via `fork()` and `pipe()`, connected to a structured logging process via a POSIX message queue.

---

## Project Structure

```
files/
├── common/
│   └── ipc.h               # Shared Logger contract (log_msg_t, LOG_QUEUE_NAME, LVL_*)
├── core/
│   ├── core_log.h          # Thin Logger façade (core_log_init / core_log / core_log_close)
│   └── core_main.c         # Core entry point — runs the IPC fork/pipe logic
├── logger/
│   └── logger_main.c       # Logger process (POSIX mqueue → ring buffer → sim.log)
├── tests/
│   ├── test_sender.c       # Sample sender / shutdown trigger
│   ├── test_logger_flood.c # Logger throughput test
│   └── test_logger_bad.c   # Logger robustness / edge-case tests
├── ui/
│   ├── public/             # HTML/CSS/JS frontend
│   └── server.py           # HTTP Bridge parsing API calls to Core stdin
├── Makefile                # Build instructions
└── run.sh                  # Convenience build + run script
```

---

## Prerequisites

- Linux (POSIX mqueue requires Linux; WSL2 works on Windows)
- GCC ≥ 7, make, librt (usually included with glibc)
- Python 3 (for the UI server)

---

## Running the Demo

### Quick start (all-in-one script)

```bash
cd files
chmod +x run.sh
./run.sh           # build + Logger + Core + UI server
```

Navigate to `http://localhost:8080` to access the dashboard.

### Manual startup

If you want to run the components manually:

**Step 1 — Start the Logger** (must come first so `/sim_log` exists):
```bash
cd files
make all
./build/logger &           # Logger runs in background, writes to sim.log
```

**Step 2 — Start the UI Server**:
```bash
python3 ui/server.py
```
*(The server automatically launches the Core process and pipes API requests to it).*

**Step 3 — Stop the Logger** (after shutting down the UI server with Ctrl+C):
```bash
./build/tests/test_sender shutdown   # sends LVL_SHUTDOWN to the Logger
```

---

## Architecture Overview

### IPC Addition Demo
The UI accepts two numbers and passes them to the Core via the Python HTTP bridge. The Core:
1. Creates a POSIX `pipe()`.
2. Forks a child process (`fork()`).
3. **Child (Process 2):** Takes `Number 2` and writes it into the pipe.
4. **Parent (Process 1):** Takes `Number 1`, reads `Number 2` from the pipe, and calculates the sum.
5. The sum is returned to the UI.

### Logger Integration
Every step of the IPC process is logged. The Core sends non-blocking messages (using `mq_timedsend`) to the `/sim_log` POSIX message queue. The Logger process receives these messages and writes them to `sim.log`. The UI dynamically tails this file to display events as they happen.
