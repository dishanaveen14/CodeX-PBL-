# IPC Using POSIX Mechanisms

## 1. Project Overview

This project demonstrates six essential Inter-Process Communication (IPC) mechanisms using POSIX system calls in Linux (and Windows MinGW cross-platform support).

Inter-Process Communication (IPC) allows independent processes to exchange data, synchronize execution, and coordinate tasks. This project implements modular C source components for each technique, integrated into a unified menu-driven application, a native C Graphical User Interface (GUI), and a Web-based interactive dashboard.

### Implemented IPC Mechanisms
1. **Pipes** (Anonymous unidirectional & bidirectional byte streams)
2. **Message Queues** (Structured message exchange with priority scheduling)
3. **Sockets** (Client-server stream communication over loopback TCP)
4. **Shared Memory** (Zero-copy direct memory access mapped into RAM)
5. **Semaphores** (Process and thread synchronization preventing race conditions)
6. **Mutex Locks** (Mutual exclusion guarding shared critical sections)

- **Programming Language:** C (C99/C11 Standard)
- **Target Operating System:** Linux (Ubuntu recommended) / Windows MinGW / WSL
- **Interface:** POSIX APIs (`unistd.h`, `mqueue.h`, `sys/socket.h`, `sys/mman.h`, `semaphore.h`, `pthread.h`)
- **Compiler:** GCC (GNU Compiler Collection)

---

## 2. Team Members and Responsibilities

The project responsibilities are distributed among four team members:

```text
+---------------------------------------------------------------------------------+
|                                TEAM ALLOCATION                                  |
+---------------------------------------------------------------------------------+
| Student 1 | Pipes & POSIX Message Queues     | pipes.c, message_queue.c         |
| Student 2 | Stream Sockets & Shared Memory   | sockets.c, shared_memory.c       |
| Student 3 | Semaphores & Mutex Locks         | semaphores.c, mutex.c            |
| Student 4 | Integration, GUI & Main App      | main.c, gui.c, Makefile, README  |
+---------------------------------------------------------------------------------+
```

### Student 1 – Pipes and Message Queues
**Tasks:**
- Implement parent-child communication using anonymous POSIX pipes (`pipe()`, `fork()`, `read()`, `write()`).
- Implement POSIX message queues with priority handling (`mq_open()`, `mq_send()`, `mq_receive()`, `mq_unlink()`).
- Demonstrate data boundaries and priority inversion prevention.
- Handle file descriptors, EOF conditions, and error cleanup.

**Files:**
- `src/pipes.c`
- `include/pipes.h`
- `src/message_queue.c`
- `include/message_queue.h`

### Student 2 – Sockets and Shared Memory
**Tasks:**
- Implement stream socket communication (`socket()`, `bind()`, `listen()`, `accept()`, `connect()`).
- Demonstrate client-server duplex request/response handshakes.
- Implement POSIX shared memory (`shm_open()`, `ftruncate()`, `mmap()`, `munmap()`, `shm_unlink()`).
- Demonstrate high-performance zero-copy data exchange.
- Handle connection teardown and shared memory deallocation.

**Files:**
- `src/sockets.c`
- `include/sockets.h`
- `src/shared_memory.c`
- `include/shared_memory.h`

### Student 3 – Semaphores and Mutex Locks
**Tasks:**
- Implement POSIX semaphores (`sem_init()`, `sem_wait()`, `sem_post()`, `sem_destroy()`).
- Demonstrate process/thread synchronization and show counter corruption when unsynchronized.
- Implement POSIX mutexes (`pthread_mutex_init()`, `pthread_mutex_lock()`, `pthread_mutex_unlock()`).
- Demonstrate mutual exclusion on a shared bank account simulation with `pthread_mutex_trylock()`.
- Prevent race conditions and protect critical sections.

**Files:**
- `src/semaphores.c`
- `include/semaphores.h`
- `src/mutex.c`
- `include/mutex.h`

### Student 4 – Integration, GUI and Main Application
**Tasks:**
- Architect the shared integration contract and function prototypes.
- Collect and verify source modules from all team members.
- Implement the interactive console menu interface (`main.c`).
- Implement the simple Graphical User Interface in C (`gui.c`, `gui.h`).
- Maintain the Web UI dashboard bridge (`ui/server.py`, `ui/public/`).
- Create and maintain the automated build system (`Makefile`, `run.sh`).
- Perform system-wide regression testing and documentation.

**Files:**
- `src/main.c`
- `src/gui.c`
- `include/gui.h`
- `Makefile`
- `run.sh`
- `README.md`

---

## 3. Project Folder Structure

```text
CodeX-PBL-week2/
├── README.md                      # Comprehensive project documentation
├── Makefile                       # Automated build script (Linux & MinGW)
├── run.sh                         # Multi-mode launcher (CLI, GUI, Web, Test)
│
├── src/                           # C source implementations
│   ├── main.c                     # Menu-driven integrated application
│   ├── gui.c                      # Simple GUI implementation in pure C
│   ├── pipes.c                    # Student 1: POSIX Pipes implementation
│   ├── message_queue.c            # Student 1: POSIX Message Queue implementation
│   ├── sockets.c                  # Student 2: Stream Sockets implementation
│   ├── shared_memory.c            # Student 2: POSIX Shared Memory implementation
│   ├── semaphores.c               # Student 3: POSIX Semaphores implementation
│   └── mutex.c                    # Student 3: POSIX Mutex Locks implementation
│
├── include/                       # Modular header interfaces
│   ├── pipes.h
│   ├── message_queue.h
│   ├── sockets.h
│   ├── shared_memory.h
│   ├── semaphores.h
│   ├── mutex.h
│   └── gui.h
│
├── ui/                            # Interactive Web Dashboard
│   ├── server.py                  # Python HTTP & REST API bridge
│   └── public/
│       ├── index.html             # Dashboard frontend
│       ├── app.js                 # Dynamic event dispatcher
│       └── style.css              # Dark theme modern stylesheet
│
└── screenshots/                   # Execution screenshots & verification
```

---

## 4. System Architecture

```text
===================================================================================
                       POSIX IPC DEMONSTRATOR ARCHITECTURE
===================================================================================

                               +--------------------+
                               |     USER INPUT     |
                               +--------------------+
                                 /        |        \
                                /         |         \
                               v          v          v
                       +-------------+ +---------+ +-----------+
                       | C CLI Menu  | |  C GUI  | |  Web GUI  |
                       |  (main.c)   | | (gui.c) | |(server.py)|
                       +-------------+ +---------+ +-----------+
                                \         |         /
                                 \        |        /
                                  v       v       v
                       +-------------------------------+
                       |    STUDENT 4 INTEGRATION      |
                       +-------------------------------+
                         /            |              \
                        /             |               \
                       v              v                v
      +-------------------+  +-------------------+  +-------------------+
      |     STUDENT 1     |  |     STUDENT 2     |  |     STUDENT 3     |
      |-------------------|  |-------------------|  |-------------------|
      | • Pipes (pipe)    |  | • Sockets (TCP)   |  | • Semaphores (sem)|
      | • Message Queues  |  | • Shared Memory   |  | • Mutexes (mutex) |
      +-------------------+  +-------------------+  +-------------------+
```

---

## 5. Function Naming Conventions

Each module exports clean, standardized functions called from `main.c` and `gui.c`:

```c
/* Standard Menu Execution Signatures */
void run_pipes(void);
void run_message_queue(void);
void run_sockets(void);
void run_shared_memory(void);
void run_semaphores(void);
void run_mutex(void);

/* Programmatic Execution Signatures (Used by GUI and Web Bridge) */
int execute_pipes_demo(const char *msg, char *out_buf, size_t out_len);
int execute_message_queue_demo(const char *msg, int prio, char *out_buf, size_t out_len);
int execute_sockets_demo(const char *msg, char *out_buf, size_t out_len);
int execute_shared_memory_demo(const char *msg, char *out_buf, size_t out_len);
int execute_semaphores_demo(int workers, int iters, char *out_buf, size_t out_len);
int execute_mutex_demo(int threads, int ops, char *out_buf, size_t out_len);
```

---

## 6. Compilation and Execution

### Prerequisites
- **Linux / Ubuntu / WSL:**
  ```bash
  sudo apt update
  sudo apt install build-essential python3
  ```
- **Windows:** MinGW GCC (`gcc` version ≥ 7.0)

### 1. Build the Application
```bash
make all
```

### 2. Run the Interactive Console Menu
```bash
# On Linux / WSL:
./ipc_project

# On Windows:
.\ipc_project.exe
```

The interactive menu will display:
```text
========================================
       IPC USING POSIX MECHANISMS       
========================================

1. Pipes
2. Message Queues
3. Sockets
4. Shared Memory
5. Semaphores
6. Mutex Locks
7. Graphical User Interface (Simple GUI in C)
8. Exit

Enter your choice:
```

### 3. Launch the Simple GUI in C
```bash
# Direct GUI invocation flag:
./ipc_project --gui
# Or:
make gui
```
- On **Windows**: Opens a native Win32 window with 8 control buttons, status indicators, and an embedded real-time output console.
- On **Linux**: Opens an interactive ANSI-rendered full-color visual terminal dashboard.

### 4. Launch the Web UI Dashboard
```bash
# Start server:
python3 ui/server.py
# Or using the launcher script:
./run.sh web
```
Open **`http://localhost:8080`** in any web browser to execute any of the 6 IPC tests with live parameter inputs and scrolling telemetry logs!

### 5. Run Automated Verification Tests
```bash
make test
# Or:
./ipc_project -a
```

---

## 7. IPC Mechanism Details & Testing Strategy

| IPC Mechanism | System Calls Used | Scope | Speed | Key Characteristic Demonstrated |
|---|---|---|---|---|
| **Pipes** | `pipe()`, `fork()`, `read()`, `write()`, `waitpid()` | Parent-Child | Fast | Kernel-buffered stream, automatic reader blocking, clean EOF. |
| **Message Queues** | `mq_open()`, `mq_send()`, `mq_receive()`, `mq_unlink()` | System-wide | Fast | Discrete message boundaries with strict priority-based dequeuing. |
| **Sockets** | `socket()`, `bind()`, `listen()`, `accept()`, `connect()` | Local / Network | Medium | Full-duplex client-server connection over loopback stream. |
| **Shared Memory** | `shm_open()`, `ftruncate()`, `mmap()`, `munmap()` | System-wide | Maximum | Direct RAM mapping with zero kernel data-copy overhead. |
| **Semaphores** | `sem_init()`, `sem_wait()`, `sem_post()`, `sem_destroy()` | Threads/Processes | Very Fast | Protects shared variables; verifies 0 lost updates vs corrupted unsynced counter. |
| **Mutex Locks** | `pthread_mutex_init()`, `lock()`, `trylock()`, `unlock()` | Threads/Processes | Very Fast | Strict mutual exclusion on financial balance with non-blocking polling. |

---

## 8. Expected Output

When running the automated test suite (`./ipc_project -a`), the system produces the following sample execution log:

```text
>>> EXECUTING ALL 6 POSIX IPC MECHANISMS <<<

[Student 1: Pipes Module]
[1] Creating anonymous unidirectional pipe using pipe()...
[+] Pipe created. Read FD: 3, Write FD: 4
[2] Simulating independent process context via POSIX thread...
[+] Writer process wrote 47 bytes into the kernel pipe.
[3] Reader process successfully retrieved message:
    --> "POSIX Pipe Data: System Telemetry Packet #1042"
[+] POSIX Pipe IPC Demonstration Completed Successfully.

[Student 1: Message Queue Module]
[1] Configuring POSIX Message Queue: /ipc_posix_demo_queue
[+] POSIX Message Queue initialized (Max: 10, Size: 256 bytes).
[2] Dispatching messages with varied priority levels...
    [Enqueued] (Prio:  1) "Msg #1: Routine telemetry heartbeat"
    [Enqueued] (Prio:  5) "Msg #2: Core process memory stats update"
    [Enqueued] (Prio: 10) "URGENT: Core Engine High-Priority Trigger"
[3] Receiving messages from queue (demonstrating priority sorting)...
    [Dequeued #1] (Prio: 10, 41 bytes) -> "URGENT: Core Engine High-Priority Trigger"
    [Dequeued #2] (Prio:  5, 40 bytes) -> "Msg #2: Core process memory stats update"
    [Dequeued #3] (Prio:  1, 35 bytes) -> "Msg #1: Routine telemetry heartbeat"
[4] Notice: High priority (10) dequeued first even though sent last!

[Student 2: Sockets Module]
[1] Starting Socket Server on localhost loopback (127.0.0.1)...
[+] Server listening on 127.0.0.1:50870.
[2] Creating Client socket and connecting to server...
[+] Connection established with server.
[3] Client transmitting payload: "Client: Telemetry Request Packet #2001"
[4] Client received reply from server: "ACK: Server processed packet (38 bytes)"
[5] Socket connection closed cleanly.

[Student 2: Shared Memory Module]
[1] Creating Shared Memory Object: /ipc_posix_demo_shm
[+] Shared memory segment (304 bytes) mapped into address space.
[2] Writer writing structured payload directly into memory block...
    [Written] TxID: 9001 | Payload: "High-Speed Matrix State: 1024x1024 Float Buffer"
[3] Reader accessing shared memory directly without kernel copy...
    [Read Back] TxID: 9001 | Data: "High-Speed Matrix State: 1024x1024 Float Buffer"
[4] Advantage: Zero-copy inter-process data exchange!

[Student 3: Semaphores Module]
[Configuration] 4 concurrent workers, 25000 increments each (Expected: 100000)
[Experiment 1: Unsynchronized Concurrent Access]
    --> Final Unsafe Counter: 100000 / 100000
[Experiment 2: POSIX Semaphore Synchronized Access]
    --> Final Synchronized Counter: 100000 / 100000
    --> Result: 100% DETERMINISTIC AND CORRECT!

[Student 3: Mutex Locks Module]
[Scenario] Shared Bank Account ($10000.00 initial balance, 4 threads, 10000 ops)
[Experiment 1: Without Mutex Lock Protection]
    --> Final Balance: $10150.00 (Discrepancy: +150.00)
    --> Status: RACE CONDITION CORRUPTED BALANCE
[Experiment 2: With POSIX Mutex Lock (pthread_mutex_lock)]
    --> Final Balance: $10000.00 (Discrepancy: $0.00)
    --> Status: 100% ACCURATE (Mutual Exclusion Preserved!)
[+] pthread_mutex_trylock() tested: Successfully acquired non-blocking lock.

[+] All 6 IPC demonstrations completed successfully.
```

---

## 9. Deliverables Checklist

- [x] Student 1: POSIX Pipes implementation (`src/pipes.c`, `include/pipes.h`)
- [x] Student 1: POSIX Message Queue implementation (`src/message_queue.c`, `include/message_queue.h`)
- [x] Student 2: Stream Sockets implementation (`src/sockets.c`, `include/sockets.h`)
- [x] Student 2: POSIX Shared Memory implementation (`src/shared_memory.c`, `include/shared_memory.h`)
- [x] Student 3: POSIX Semaphores implementation (`src/semaphores.c`, `include/semaphores.h`)
- [x] Student 3: POSIX Mutex Locks implementation (`src/mutex.c`, `include/mutex.h`)
- [x] Student 4: Integrated Menu Application (`src/main.c`)
- [x] Student 4: Simple C Graphical User Interface (`src/gui.c`, `include/gui.h`)
- [x] Student 4: Web UI Dashboard & Bridge (`ui/server.py`, `ui/public/`)
- [x] Build System (`Makefile`, `run.sh`)
- [x] Full System Verification & Regression Tests

---

## 10. Conclusion

This project integrates all six foundational POSIX IPC mechanisms into an educational, modular C software suite. By dividing work cleanly among 4 team members and integrating both a console interface and graphical interfaces (native C GUI + Web UI), the project provides hands-on practical mastery over concurrent process communication, resource synchronization, and operating systems engineering.
