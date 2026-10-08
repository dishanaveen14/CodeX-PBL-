# CodeX-PBL — OS Simulation Project

A multi-process system simulation demonstrating CPU operations, memory management,
stack/queue data structures, and POSIX IPC, connected to a structured logging
process via a POSIX message queue.

---

## Project Structure

```
files/
├── common/
│   └── ipc.h               # Shared Logger contract (log_msg_t, LOG_QUEUE_NAME, LVL_*)
├── core/
│   ├── core_log.h          # ★ Thin Logger façade (core_log_init / core_log / core_log_close)
│   ├── core_main.c         # ★ Core entry point — all Logger call sites live here
│   ├── cpu.h / cpu.c       # ★ CPU simulation (ADD/SUB/MUL/DIV on 4 registers)
│   ├── memory.h / memory.c # ★ Flat 4 KiB memory bank with bounds checking
│   ├── stack.h / stack.c   # ★ Fixed-capacity LIFO stack
│   ├── queue.h / queue.c   # ★ Fixed-capacity circular FIFO queue
│   └── test_driver.c       # ★ Unit tests for all subsystems (also logs results)
├── logger/
│   └── logger_main.c       # Logger process (POSIX mqueue → ring buffer → sim.log)
├── tests/
│   ├── test_sender.c       # Sample sender / shutdown trigger
│   ├── test_logger_flood.c # Logger throughput test
│   └── test_logger_bad.c   # Logger robustness / edge-case tests
├── Makefile                # ★ Updated — adds Core and test_driver targets
└── run.sh                  # ★ Updated — convenience build + run script
```

★ = modified or created as part of the Logger–Core integration.

---

## Prerequisites

- Linux (POSIX mqueue requires Linux; WSL2 works on Windows)
- GCC ≥ 7, make, librt (usually included with glibc)

---

## Build

```bash
cd files
make            # builds Logger + Core + test_driver + test_sender
make tests      # additionally builds logger flood/bad tests
make clean      # removes all build artefacts
```

Individual targets:

| Target                       | Output binary              |
|------------------------------|----------------------------|
| `build/logger`               | Logger process             |
| `build/core/core`            | Core process (full demo)   |
| `build/core/test_driver`     | Core unit-test driver      |
| `build/tests/test_sender`    | Manual Logger sender       |
| `build/tests/test_logger_flood` | Logger throughput test  |
| `build/tests/test_logger_bad`   | Logger robustness test  |

---

## Running

### Quick start (all-in-one script)

```bash
chmod +x run.sh
./run.sh              # build + Logger + Core demo + tail sim.log
./run.sh test-core    # build + Logger + Core unit tests + tail sim.log
./run.sh test         # Logger robustness tests only
./run.sh clean        # clean build
```

### Manual startup

**Step 1 — Start the Logger** (must come first so `/sim_log` exists):

```bash
cd files
build/logger &           # Logger runs in background, writes to sim.log
```

**Step 2 — Run the Core**:

```bash
build/core/core          # runs all demos and logs events
```

**Step 3 — Stop the Logger**:

```bash
build/tests/test_sender shutdown   # sends LVL_SHUTDOWN to the Logger
```

**Step 4 — Inspect the log**:

```bash
cat sim.log
```

### Core unit tests

```bash
build/logger &
build/core/test_driver
build/tests/test_sender shutdown
cat sim.log
```

---

## Logger–Core Integration Design

### Communication mechanism

The Core uses the same POSIX message queue (`/sim_log`) as every other
component.  The queue contract is defined in `common/ipc.h`:

```c
typedef struct {
    uint64_t ts_ns;   /* CLOCK_REALTIME nanoseconds (0 = Logger stamps) */
    uint8_t  level;   /* LVL_INFO=0  LVL_WARN=1  LVL_ERROR=2  LVL_SHUTDOWN=255 */
    char     src[8];  /* "CORE" */
    char     text[200];
} log_msg_t;
```

### Façade: `core/core_log.h`

| Function | Description |
|---|---|
| `core_log_init()` | Opens `/sim_log` O_WRONLY\|O_NONBLOCK. Returns -1 (prints warning) if Logger absent — Core continues. |
| `core_log(level, fmt, ...)` | Stamps `ts_ns`, fills `log_msg_t`, calls `mq_send` non-blocking. Silently drops if queue full. |
| `core_log_close()` | Closes the queue handle. |

**Key design choices:**

- **Non-blocking sends** — the Core never stalls on a full queue.
- **Graceful degradation** — if the Logger is not running `core_log_init()`
  prints one warning and all subsequent `core_log()` calls are no-ops.
- **Single log site per event** — `cpu.c`, `memory.c`, `stack.c`, `queue.c`
  contain zero logging. All `core_log()` calls live in `core_main.c` and
  `test_driver.c`, so no event is logged twice.
- **`CORE_LOG_IMPL` guard** — static storage (`g_log_q`, `g_log_ok`) is
  defined once by `#define CORE_LOG_IMPL` before the `#include "core_log.h"`
  in each binary's translation unit (`core_main.c`, `test_driver.c`).

### Events logged

| Event | Level |
|---|---|
| Core startup / shutdown | INFO |
| Subsystem initialisation | INFO |
| Each CPU ALU operation + result | INFO |
| Division by zero / invalid register | WARN |
| Memory write/read + result | INFO |
| Memory OOB / misaligned access rejected | WARN |
| Stack push/pop + depth | INFO |
| Stack overflow / underflow rejected | WARN |
| Queue enqueue/dequeue + count | INFO |
| Queue overflow / underflow rejected | WARN |
| IPC fork created, child PID | INFO |
| Pipe send (child side) + value | INFO |
| Pipe receive (parent side) + value | INFO |
| Sum computed | INFO |
| Child exit status | INFO / WARN |
| Any syscall failure (pipe, fork, read, write, waitpid) | ERROR |
| Test PASS / FAIL (test_driver) | INFO / ERROR |

---

## Log Format

Lines written by the Logger follow this pattern (from `logger_main.c`):

```
2026-10-08 09:30:00.123456 [INFO ] [CORE   ] Core startup
2026-10-08 09:30:00.124000 [INFO ] [CORE   ] CPU ADD R2=R0+R1 -> R2=13
2026-10-08 09:30:00.124100 [WARN ] [CORE   ] CPU DIV by zero rejected (R1=0)
2026-10-08 09:30:00.125000 [INFO ] [CORE   ] IPC addition: sum = 7 + 5 = 12
```

Log files rotate at 1 MB (configurable via `LOG_ROTATE_BYTES` env var),
keeping up to 3 backups (`sim.log.1` … `sim.log.3`).
