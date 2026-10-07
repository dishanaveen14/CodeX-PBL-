# Week 4 — Multi-Process Simulator & IPC

## Final Project Structure

```text
week4-simulator/
│
├── common/
│   └── ipc.h
│
├── ui/
│   ├── ui_main.c
│   ├── ui_parser.c
│   ├── ui_parser.h
│   ├── ui_display.c
│   └── ui_display.h
│
├── core/
│   ├── core_main.c
│   ├── core_ipc.c
│   ├── core_ipc.h
│   ├── cpu.c
│   ├── cpu.h
│   ├── memory.c
│   ├── memory.h
│   ├── stack.c
│   ├── stack.h
│   ├── queue.c
│   └── queue.h
│
├── logger/
│   ├── logger_main.c
│   ├── logger_receiver.c
│   ├── logger_receiver.h
│   ├── logger_writer.c
│   ├── logger_writer.h
│   └── logger_rotation.c
│
├── tests/
│   ├── test_ipc.c
│   ├── test_ui.c
│   ├── test_core.c
│   ├── test_logger.c
│   ├── mock_core.c
│   ├── test_integration.c
│   └── test_stress.c
│
├── bench/
│   ├── benchmark.c
│   ├── benchmark_ipc.c
│   └── results/
│       ├── standalone.csv
│       ├── multiprocess.csv
│       └── benchmark_results.png
│
├── docs/
│   ├── architecture.md
│   ├── ipc-justification.md
│   ├── command-reference.md
│   ├── log-format.md
│   ├── testing.md
│   ├── benchmarking.md
│   └── ai-usage.md
│
├── build/
│   └── # Generated object files and binaries
│
├── sim.log
├── Makefile
├── run.sh
├── clean.sh
├── README.md
└── .gitignore
```

---

## 1. `common/`

Contains interfaces shared by all processes.

### `common/ipc.h`

The central IPC contract shared by UI, Core, and Logger.

Contains:

- POSIX message-queue names
- Message-size constants
- Command enumeration
- Response/status enumeration
- Log-level enumeration
- `cmd_msg_t`
- `resp_msg_t`
- `log_msg_t`
- Shared protocol definitions

**Rule:** Changes to `ipc.h` must be reviewed by the Team Leader because all three processes depend on this contract.

---

## 2. `ui/`

Owned by **Student 1**.

Responsible for user interaction and displaying simulator state.

### `ui_main.c`

- Starts the UI process
- Opens `/sim_cmd` and `/sim_resp`
- Creates input and display threads
- Handles shutdown and cleanup

### `ui_parser.c / ui_parser.h`

Parses commands entered by the user.

Example:

```text
LOAD program.txt
RUN
STEP
PUSH 10
READ_MEM 100
SHOW_REGS
SHOW_STACK
SHUTDOWN
```

### `ui_display.c / ui_display.h`

Displays:

- CPU registers
- Memory
- Stack
- Queue
- Command results
- Errors
- Core status

---

## 3. `core/`

Owned by **Student 2**.

Contains the simulator's main execution logic.

### `core_main.c`

- Starts the Core process
- Initializes CPU, memory, stack and queue
- Opens `/sim_cmd`, `/sim_resp`, and `/sim_log`
- Creates Core threads
- Handles shutdown

### `core_ipc.c / core_ipc.h`

Handles communication with the UI and Logger.

Responsibilities:

- Receive commands
- Validate commands
- Send responses
- Send log events
- Handle IPC errors and timeouts

### `cpu.c / cpu.h`

CPU simulation:

- Registers
- Program counter
- Instruction execution
- Instruction stepping
- Run loop

### `memory.c / memory.h`

Memory subsystem:

- Read
- Write
- Address validation
- Invalid-address handling

### `stack.c / stack.h`

Stack subsystem:

- Push
- Pop
- Peek
- Stack overflow detection
- Stack underflow detection

### `queue.c / queue.h`

Internal Core command queue:

```text
IPC Receiver Thread
        │
        ▼
   Internal Queue
        │
        ▼
 Execution Thread
```

Access must be protected using a mutex and condition variable.

---

## 4. `logger/`

Owned by **Student 3**.

Responsible for asynchronous logging.

### `logger_main.c`

- Creates/opens `/sim_log`
- Initializes the logger
- Creates receiver and writer threads
- Handles shutdown
- Cleans up resources

### `logger_receiver.c / logger_receiver.h`

Receives messages from `/sim_log` and places them into an in-memory ring buffer.

The receiver should avoid expensive file I/O.

### `logger_writer.c / logger_writer.h`

Consumes messages from the ring buffer and writes them to:

```text
sim.log
```

Messages should be flushed in batches.

### `logger_rotation.c`

Handles log-file rotation when the log reaches the configured size.

---

## 5. `tests/`

Contains unit, integration, failure and stress tests.

### `test_ipc.c`

Tests:

- Queue creation
- Sending
- Receiving
- Message sizes
- Queue cleanup

### `test_ui.c`

Tests:

- Command parsing
- Invalid commands
- Argument validation
- UI response handling

### `test_core.c`

Tests:

- CPU execution
- Memory operations
- Stack operations
- Queue operations
- Invalid addresses
- Overflow/underflow

### `test_logger.c`

Tests:

- Log reception
- Ring-buffer behavior
- File writing
- Log rotation

### `mock_core.c`

Acts as a fake Core so the UI can be tested independently.

### `test_integration.c`

Tests:

```text
UI → Core
Core → UI
Core → Logger
```

### `test_stress.c`

Tests high-volume IPC and logging.

Target:

```text
100,000+ log messages
```

---

## 6. `bench/`

Contains performance benchmarks.

### `benchmark.c`

Compares:

```text
Standalone Simulator
        vs
Multi-Process Simulator
```

Metrics:

- Execution time
- CPU usage
- Maximum RSS
- Throughput

### `benchmark_ipc.c`

Measures:

- Message round-trip latency
- Messages per second
- IPC overhead

### `bench/results/`

Stores raw benchmark data and generated charts.

---

## 7. `docs/`

Project documentation.

### `architecture.md`

Contains:

- System architecture
- Process diagram
- Thread diagram
- Data flow
- Startup/shutdown sequence

### `ipc-justification.md`

Explains why POSIX Message Queues were selected instead of:

- Pipes/FIFOs
- Shared memory
- Sockets

### `command-reference.md`

Documents every UI command.

Example:

| Command | Arguments | Description |
|---|---|---|
| `LOAD` | filename | Load a program |
| `RUN` | — | Run program |
| `STEP` | — | Execute one instruction |
| `PUSH` | value | Push value onto stack |
| `POP` | — | Pop stack value |
| `READ_MEM` | address | Read memory |
| `WRITE_MEM` | address value | Write memory |
| `SHOW_REGS` | — | Display registers |
| `SHOW_STACK` | — | Display stack |
| `SHOW_MEM` | — | Display memory |
| `SHUTDOWN` | — | Shut down simulator |

### `log-format.md`

Defines the format of `sim.log`.

Example:

```text
2026-10-07T09:03:21.123456 INFO  CORE   Instruction executed
2026-10-07T09:03:21.123789 WARN  CORE   Stack near capacity
2026-10-07T09:03:21.124001 ERROR CORE   Invalid memory address
```

### `testing.md`

Documents:

- Unit tests
- Integration tests
- Failure tests
- Stress tests
- Sanitizer usage

### `benchmarking.md`

Documents:

- Benchmark methodology
- Number of runs
- Hardware/software environment
- Metrics
- Results
- Analysis

### `ai-usage.md`

Records how AI tools were used during development.

Each student should document:

- What was asked
- What code/idea was generated
- What was modified
- What was tested
- What the student learned

---

## 8. Root Files

### `Makefile`

Provides standard build targets.

Recommended targets:

```text
make
make debug
make test
make asan
make tsan
make benchmark
make clean
```

`asan` and `tsan` should be separate targets.

### `run.sh`

Starts the processes in the correct order:

```text
Logger
   ↓
Core
   ↓
UI
```

It should also handle cleanup of stale message queues.

### `clean.sh`

Removes:

- Build artifacts
- Binaries
- Temporary files
- Stale POSIX message queues

### `README.md`

The main project documentation.

Should include:

1. Project overview
2. Architecture
3. Team responsibilities
4. Build instructions
5. Run instructions
6. Command reference
7. Testing
8. Benchmarking
9. IPC justification
10. AI usage
11. Demo instructions

### `.gitignore`

Should exclude:

```text
build/
*.o
*.log
*.out
core.*
.vscode/
.idea/
```

---

# Process Architecture

```text
                         POSIX MESSAGE QUEUES
┌──────────────┐          /sim_cmd          ┌──────────────────┐
│              │ ─────────────────────────► │                  │
│  UI Process  │                            │  Core Process    │
│              │ ◄───────────────────────── │                  │
└──────┬───────┘          /sim_resp         │  ┌────────────┐  │
       │                                    │  │    CPU     │  │
       │ /sim_log                           │  ├────────────┤  │
       │                                    │  │   Memory   │  │
       ▼                                    │  ├────────────┤  │
┌──────────────┐                            │  │   Stack    │  │
│              │                            │  ├────────────┤  │
│ Logger Proc. │                            │  │   Queue    │  │
│              │                            │  └────────────┘  │
└──────┬───────┘                            └──────────────────┘
       │
       ▼
   ┌─────────┐
   │ sim.log │
   └─────────┘
```

---

# Thread Architecture

## UI

```text
UI Process
├── Input Thread
│   └── User input → /sim_cmd
│
└── Display Thread
    └── /sim_resp → Terminal
```

## Core

```text
Core Process
├── IPC Receiver Thread
│   └── /sim_cmd → Internal Command Queue
│
└── Execution Thread
    └── Command Queue → CPU/Memory/Stack/Queue
```

## Logger

```text
Logger Process
├── Receiver Thread
│   └── /sim_log → Ring Buffer
│
└── Writer Thread
    └── Ring Buffer → sim.log
```

---

# Ownership

| Component | Owner | Main Responsibility |
|---|---|---|
| `ui/` | Student 1 | User interface |
| `core/` | Student 2 | Simulator execution |
| `logger/` | Student 3 | Asynchronous logging |
| `common/ipc.h` | Team Leader | Shared IPC contract |
| `Makefile` | Team Leader | Build system |
| `run.sh` | Team Leader | Process launcher |
| `tests/` | Team | Testing |
| `bench/` | Team Leader | Performance analysis |
| `docs/` | Team | Documentation |
| Integration | Team Leader | Full-system integration |

---

# Build Configuration

The project should support at least four build modes:

```text
Normal
  ↓
Debug
  ↓
AddressSanitizer
  ↓
ThreadSanitizer
```

Recommended compiler flags:

```text
-Wall -Wextra -g
```

For AddressSanitizer:

```text
-fsanitize=address
```

For ThreadSanitizer:

```text
-fsanitize=thread
```

Do not combine AddressSanitizer and ThreadSanitizer in the same build.

---

# Final Deliverables

At project completion, the repository should contain:

- [ ] Working UI process
- [ ] Working Core process
- [ ] Working Logger process
- [ ] POSIX message-queue IPC
- [ ] Thread-safe internal Core queue
- [ ] Thread-safe Logger ring buffer
- [ ] Graceful shutdown
- [ ] Error and timeout handling
- [ ] Log rotation
- [ ] Unit tests
- [ ] Integration tests
- [ ] Stress tests
- [ ] IPC benchmark
- [ ] Standalone vs multi-process benchmark
- [ ] Benchmark chart
- [ ] Architecture documentation
- [ ] IPC justification
- [ ] Command reference
- [ ] AI-usage documentation
- [ ] README
- [ ] Final demo script

---

# Final Demo Flow

```text
1. ./run.sh
       │
       ├── Start Logger
       ├── Start Core
       └── Start UI
              │
              ▼
2. Load program
              │
              ▼
3. Run / Step
              │
              ▼
4. Display registers, memory, stack and queue
              │
              ▼
5. Trigger an error
              │
              ▼
6. Verify error appears in sim.log
              │
              ▼
7. Show /dev/mqueue
              │
              ▼
8. Kill Logger
              │
              ▼
9. Demonstrate Core continues running
              │
              ▼
10. Present benchmark results
```

## Definition of Done

The project is considered complete when all three processes can run independently and together, communicate through the agreed POSIX message-queue protocol, handle expected failures safely, and pass the automated tests without races, memory errors, or resource leaks.
