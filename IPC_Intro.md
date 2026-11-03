# Inter-Process Communication (IPC)
## Introductory Overview & Fundamentals

---

## 1. Definition

**Inter-Process Communication (IPC)** is a set of mechanisms and protocols provided by an operating system that allows separate processes to share data, synchronize actions, and coordinate their execution. 

Because modern operating systems isolate each process within its own private virtual memory space for security and stability, one process cannot directly read from or write to the memory of another process. IPC provides safe, controlled pathways through which processes can communicate and collaborate.

---

## 2. Why IPC Is Needed

Operating systems require IPC for several vital architectural reasons:

- **Information Sharing:** Multiple concurrent processes or users may need access to the same shared data source (e.g., databases, shared configuration files, shared state).
- **Computation Speed-up:** Computationally intensive tasks can be partitioned into independent subtasks that run concurrently across multiple CPU cores or processes.
- **Modularity:** System architects can divide large, complex software applications into smaller, specialized, and loosely coupled modular processes (such as microservices or daemon processes) that interact cleanly.
- **Convenience & Composability:** Users and programs can combine distinct utilities to perform sophisticated tasks (e.g., shell pipelines connecting `ps`, `grep`, and `awk`).

---

## 3. High-Level IPC Categorization

All IPC techniques fundamentally fall into two architectural paradigms:

```text
                 ┌────────────────────────────────────────┐
                 │ Inter-Process Communication Paradigms  │
                 └──────────────────┬─────────────────────┘
                                    │
           ┌────────────────────────┴────────────────────────┐
           ▼                                                 ▼
┌───────────────────────┐                         ┌───────────────────────┐
│     Shared Memory     │                         │    Message Passing    │
├───────────────────────┤                         ├───────────────────────┤
│ • Direct RAM access   │                         │ • Kernel mediated     │
│ • Maximum speed       │                         │ • Built-in isolation  │
│ • Requires locking    │                         │ • Works across nets   │
└───────────────────────┘                         └───────────────────────┘
```

1. **Shared Memory:**
   - Processes map a common region of physical memory into their virtual address spaces.
   - Once established, communication occurs at native memory bus speeds without recurring kernel intervention.
   - Requires explicit synchronization mechanisms (e.g., semaphores or mutexes) to avoid race conditions.

2. **Message Passing:**
   - Communication is mediated by the operating system kernel or network protocol stacks.
   - Processes exchange data via discrete messages or continuous byte streams.
   - Includes primitives such as pipes, message queues, sockets, signals, and RPC.

