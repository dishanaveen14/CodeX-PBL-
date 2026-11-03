# CodeX — Operating Systems: Inter-Process Communication (IPC)

Welcome to the **CodeX** Project-Based Learning (PBL) repository for Operating Systems. This project provides a comprehensive, structured study of **Inter-Process Communication (IPC)** mechanisms, their architectural trade-offs, synchronization methods, and practical operating system implementations.

---

## 👥 Team CodeX & Contributors

| Member | Focus Topic & Module | Primary Document |
|---|---|---|
| **Disha** | IPC Architecture, Foundations & Master Report | [IPC.md](IPC.md), [Inter1.md](Inter1.md) |
| **Dhanush** | Pipes & Message Queues | [Pipes_and_Message_Queues.md](Pipes_and_Message_Queues.md) |
| **Sohan** | Shared Memory & Semaphores | [Shared_Memory_and_Semaphores.md](Shared_Memory_and_Semaphores.md) |
| **Bathin** | Sockets & Signals | [Sockets_and_Signals.md](Sockets_and_Signals.md) |

---

## 📚 Documentation Index

Each topic is cleanly modularized into dedicated Markdown guides, with [IPC.md](IPC.md) serving as the unified master reference:

| Document | Topic | Description |
|---|---|---|
| 📑 **[README.md](README.md)** | Repository Index | Project overview, team members, architecture summary, and file index. |
| 📘 **[IPC.md](IPC.md)** | Master Comprehensive Guide | Complete end-to-end report covering all 8 IPC techniques with architectural diagrams and comparison tables. |
| 🚰 **[Pipes_and_Message_Queues.md](Pipes_and_Message_Queues.md)** | Pipes & Message Queues | Anonymous pipes, named FIFOs, POSIX/System V message queues, diagrams, and trade-offs. |
| 🧠 **[Shared_Memory_and_Semaphores.md](Shared_Memory_and_Semaphores.md)** | Shared Memory & Semaphores | High-speed memory sharing, critical sections, binary and counting semaphores, and race conditions. |
| 🌐 **[Sockets_and_Signals.md](Sockets_and_Signals.md)** | Sockets & Signals | Stream (TCP), Datagram (UDP), and Unix domain sockets, kernel signals, handlers, and process lifecycle. |
| 📝 **[Inter1.md](Inter1.md)** | IPC Introduction & Overview | Foundational concepts: definition of IPC, process isolation, and classification into shared memory vs message passing. |

---

## 🗺️ Project Structure

```text
CodeX-PBL-/
├── README.md                          # Main repository landing page & index
├── IPC.md                             # Comprehensive project master report (with figures)
├── Inter1.md                          # Foundational IPC concepts & introduction
├── Pipes_and_Message_Queues.md        # Deep dive: Pipes and Message Queues
├── Shared_Memory_and_Semaphores.md    # Deep dive: Shared Memory & Semaphores
├── Sockets_and_Signals.md             # Deep dive: Sockets & Signals
└── images/                            # Architectural diagrams & figures for IPC.md
    ├── image1.png                     # Classification of IPC techniques
    ├── image2.png                     # Data flow through a pipe
    ├── image3.png                     # Kernel message queue model
    ├── image4.png                     # Shared memory architecture
    ├── image5.png                     # Semaphore-guarded critical section
    ├── image6.png                     # Client-server socket architecture
    ├── image7.png                     # Kernel signal delivery model
    ├── image8.png                     # Memory-mapped file sharing
    └── image9.png                     # Remote Procedure Call (RPC) lifecycle
```

---

## ⚡ IPC Techniques at a Glance

| Technique | Category | Scope | Data Payload | Primary Use Case |
|---|---|---|---|---|
| **Pipes** | Message Passing | Single host (parent-child or named) | Unstructured byte stream | Shell pipelines (`cmd1 \| cmd2`), producer-consumer streams |
| **Message Queues** | Message Passing | Single host | Structured, typed messages | Asynchronous decoupled tasks with priority ordering |
| **Shared Memory** | Shared Memory | Single host | Direct memory access (RAM) | High-throughput, low-latency inter-process data exchange |
| **Semaphores** | Synchronization | Single host | Counter / lock state (no payload) | Protecting critical sections, avoiding race conditions |
| **Sockets** | Network / Local | Single host or across networks | Streams (TCP) or datagrams (UDP) | Distributed systems, client-server apps, web protocols |
| **Signals** | Asynchronous Event | Single host | Integer signal number | Interrupts (`Ctrl+C`), process termination, exception handling |
| **Memory-Mapped Files** | Shared Memory / File | Single host | File pages mapped into memory | High-performance file I/O, database storage engines |
| **RPC** | High-level Abstraction | Across network / hosts | Marshalled function calls & returns | Microservices (gRPC), cloud services, remote invocation |

---

## 🎯 Key Takeaways

1. **Shared Memory = Speed & Direct Access** — The fastest way to share data on the same host, but requires mutual exclusion (semaphores) to guard against concurrency bugs.
2. **Pipes & Queues = Kernel-Managed Safety** — Built-in synchronization and buffering managed by the OS kernel, reducing developer error at the cost of memory copies.
3. **Sockets & RPC = Cross-Network Reach** — The bedrock of modern internet applications, web APIs, and distributed cloud computing.
4. **Signals = Asynchronous Process Control** — Lightweight notifications that interrupt a process to handle urgent conditions, terminations, or status changes.

