# Inter-Process Communication (IPC)
## Sockets and Signals

---

## 1. Introduction

Operating systems provide several mechanisms for communication and control between processes. Two fundamental mechanisms in this domain are **Sockets** and **Signals**:

- **Sockets** are used for data exchange between processes, both locally on the same host and across networks.
- **Signals** are asynchronous notifications used to alert or control processes when specific events occur.

Understanding these mechanisms is essential for systems programming, client-server architectures, and process management.

---

## 2. Sockets

A **socket** is an endpoint for bidirectional communication between two processes. It is an abstraction provided by the operating system kernel that enables processes to send and receive streams or packets of data. Sockets can operate locally on the same machine (Unix domain sockets) or across distinct machines via a computer network (Internet sockets).

> 💡 **Analogy:** Think of a socket as a *virtual plug*. Just like plugging two devices into a cable or network jack to exchange information, two independent programs connect through sockets to share data. One program writes data into the socket, and the other reads it.

### How It Works

1. **Creation:** Both the client and server create socket endpoints using the `socket()` system call.
2. **Binding & Listening (Server):** The server binds the socket to an address/port using `bind()` and marks it passive with `listen()`.
3. **Connection:** The client connects to the server's listening socket with `connect()`.
4. **Acceptance:** The server accepts the incoming connection with `accept()`, establishing a dedicated communication channel.
5. **Data Transfer:** Processes exchange data using `send()` / `write()` and `recv()` / `read()`.
6. **Termination:** Both ends close their connection descriptors with `close()`.

### Diagram: Socket Client-Server Flow

```text
       Server Process                               Client Process
      ┌───────────────┐                            ┌───────────────┐
      │   socket()    │                            │   socket()    │
      └───────┬───────┘                            └───────┬───────┘
              ▼                                            │
      ┌───────────────┐                                    │
      │    bind()     │                                    │
      └───────┬───────┘                                    │
              ▼                                            │
      ┌───────────────┐                                    │
      │   listen()    │                                    │
      └───────┬───────┘                                    │
              ▼                                            ▼
      ┌───────────────┐      Connection Request    ┌───────────────┐
      │   accept()    │ ◄───────────────────────── │   connect()   │
      └───────┬───────┘                            └───────┬───────┘
              │                                            │
              ▼            Data Transfer                   ▼
      ┌───────────────┐ ◄────────────────────────► ┌───────────────┐
      │ recv()/send() │                            │ send()/recv() │
      └───────┬───────┘                            └───────┬───────┘
              ▼                                            ▼
      ┌───────────────┐                            ┌───────────────┐
      │    close()    │                            │    close()    │
      └───────────────┘                            └───────────────┘
```

### Types of Sockets

1. **Stream Sockets (`SOCK_STREAM`):**
   - Provide connection-oriented, sequenced, and reliable byte-stream transmission.
   - Built on top of the **TCP** (Transmission Control Protocol).
   - Guarantees error-free and in-order delivery.
   - *Example:* Web browsing (HTTP/HTTPS), SSH, database connections.

2. **Datagram Sockets (`SOCK_DGRAM`):**
   - Provide connectionless, packet-oriented transmission.
   - Built on top of the **UDP** (User Datagram Protocol).
   - Fast with low latency, but messages can arrive out-of-order, be duplicated, or be dropped.
   - *Example:* Real-time gaming, video streaming, DNS lookups, VoIP.

3. **Unix Domain Sockets (`AF_UNIX` / `AF_LOCAL`):**
   - Used exclusively for communication between processes on the *same machine*.
   - Avoid network protocol stack overhead and checksums, resulting in significantly higher throughput.
   - *Example:* Local web server reverse proxies communicating with application backends (e.g., NGINX to Gunicorn/NodeJS) or local databases.

### Examples of Socket Applications

- **Web Browsers & Servers:** Browsers use sockets to connect to HTTP/HTTPS servers (ports 80/443) and fetch web pages.
- **Chat Applications:** Messaging platforms (e.g., WhatsApp, Discord) maintain persistent socket connections for real-time bi-directional messaging.
- **File Transfer Protocols:** FTP and SFTP rely on socket channels to stream files reliably between clients and servers.

### Advantages

- Supports both local inter-process communication and remote cross-network communication.
- Full-duplex (simultaneous two-way communication).
- Standardized API (POSIX / Berkeley sockets) available across almost all operating systems and programming languages.
- Highly scalable; a single server can serve thousands of concurrent clients.

### Disadvantages

- Substantial connection setup overhead compared to shared memory.
- Network sockets require handling dropped connections, latency, timeouts, and network partitions.
- Requires network security considerations (encryption such as TLS/SSL, authentication, firewall configuration).

---

## 3. Signals

A **signal** is an asynchronous notification delivered by the kernel to a process to inform it that an event has occurred. Signals represent a lightweight form of IPC; they carry minimal payload (typically just the signal number itself) and are primarily used for exception handling, process control, and lifecycle management.

> 💡 **Analogy:** Imagine signals as *alerts* or *software interrupts*. For instance, pressing `Ctrl+C` in your terminal sends an interrupt signal to the running foreground process telling it to stop. Signals do not transmit large data payloads; they indicate an urgent event.

### How It Works

1. **Generation:** An event generates a signal (e.g., hardware fault, timer expiration, keyboard shortcut, or another process calling `kill()`).
2. **Delivery:** The kernel holds the signal until it can safely deliver it to the target process.
3. **Handling:** When the process resumes execution, it responds in one of three ways:
   - **Default Action:** The operating system's default behavior takes place (e.g., terminate process, core dump, ignore, stop).
   - **Ignore:** The process ignores the signal (discarding it).
   - **Catch (Signal Handler):** The process executes a custom user-defined function, then resumes normal execution.

```text
[ Hardware Fault / User Input / Process kill() ]
                       │
                       ▼
             ┌───────────────────┐
             │   Kernel Space    │
             │  Generates Signal │
             └─────────┬─────────┘
                       │ Delivers signal
                       ▼
             ┌───────────────────┐
             │   Target Process  │
             └─────────┬─────────┘
                       │
        ┌──────────────┼──────────────┐
        ▼              ▼              ▼
  Default Action    Ignore     Custom Handler
 (e.g., Terminate)  (Drop)     (User Function)
```

> ⚠️ **Note:** `SIGKILL` (signal 9) and `SIGSTOP` cannot be caught, blocked, or ignored by any process.

### Common Standard Signals

| Signal | Number | Default Action | Description & Trigger |
|---|:---:|:---:|---|
| **`SIGINT`** | 2 | Terminate | Terminal interrupt signal (sent when the user presses `Ctrl+C`). |
| **`SIGQUIT`** | 3 | Core Dump | Terminal quit signal (sent via `Ctrl+\`); terminates and dumps core. |
| **`SIGKILL`** | 9 | Terminate (Forced) | Unconditionally kills the process immediately. Cannot be caught or ignored. |
| **`SIGSEGV`** | 11 | Core Dump | Segmentation violation (invalid memory access / null pointer dereference). |
| **`SIGPIPE`** | 13 | Terminate | Broken pipe (attempting to write to a pipe or socket with no active reader). |
| **`SIGTERM`** | 15 | Terminate | Graceful termination request (sent by default when using `kill <pid>`). |
| **`SIGUSR1` / `SIGUSR2`** | 10, 12 | Terminate | User-defined signals available for custom application notification logic. |
| **`SIGCHLD`** | 17 | Ignore | Sent to a parent process when a child process terminates or stops. |
| **`SIGTSTP`** | 20 | Stop | Terminal stop signal (sent when the user presses `Ctrl+Z`). |
| **`SIGHUP`** | 1 | Terminate | Hangup detected on controlling terminal; commonly used to reload server config. |

### Practical Examples of Signals

- **Terminating stuck tasks:** Running `kill -9 <pid>` sends `SIGKILL` to forcefully shut down an unresponsive process.
- **Handling crashes:** A segmentation fault causes the kernel to deliver `SIGSEGV`, allowing debuggers like GDB to inspect the failure point.
- **Hot configuration reload:** Daemons like NGINX and Apache listen for `SIGHUP` to reload config files without restarting worker processes.
- **Pausing tasks:** Pressing `Ctrl+Z` sends `SIGTSTP`, suspending execution until resumed with `fg` or `bg`.

### Advantages

- Minimal overhead; extremely fast and lightweight.
- Asynchronous delivery; the receiving process does not have to actively poll for events.
- Deeply integrated into the operating system and process lifecycle management.

### Disadvantages

- Carries virtually no data (only the signal ID).
- Asynchronous execution introduces concurrency bugs and race conditions in non-reentrant code.
- Standard POSIX signals do not queue; duplicate signals arriving in quick succession may be coalesced and lost.
- Signal handlers are restricted to executing only async-signal-safe functions.

---

## 4. Sockets vs Signals Comparison

| Feature | Sockets | Signals |
|---|---|---|
| **Primary Purpose** | Structured data communication | Event notification and process control |
| **Data Payload** | Arbitrary payloads (streams or datagrams) | Minimal (only signal number) |
| **Network Capability** | Yes (both local and network-wide) | No (local host only) |
| **Communication Flow** | Bi-directional (Full-duplex) | One-way notification (Kernel/Sender → Target) |
| **Synchronization** | Synchronous or asynchronous I/O | Asynchronous interruption |
| **Target Audience** | Client-server applications, distributed systems | Operating system, job control, error handling |

---

## 5. Common Problems & Pitfalls

- **Broken Pipes (`SIGPIPE`):** Writing to a socket that has already been closed by the peer triggers a `SIGPIPE` signal, which will terminate the application if unhandled.
- **Non-Reentrant Handlers:** Calling non-thread-safe functions (like `malloc()` or `printf()`) within a signal handler can lead to memory corruption and deadlocks.
- **Zombie Processes:** Failing to catch `SIGCHLD` and call `wait()`/`waitpid()` leaves terminated child processes lingering in the process table as zombies.
- **Socket Port Exhaustion / TIME_WAIT:** Opening and rapidly closing thousands of TCP connections can leave sockets in `TIME_WAIT`, exhausting ephemeral ports.

---

## 6. Applications

- **Networked & Distributed Systems:** Web servers, microservices, RPC frameworks, and distributed databases all use sockets for message transport.
- **Process Supervision & Daemons:** Process orchestrators (e.g., systemd, Docker, Kubernetes) use signals (`SIGTERM`, `SIGKILL`, `SIGCHLD`) to monitor and gracefully terminate containers.
- **Interactive Shells:** Command-line interfaces manage foreground and background jobs via `SIGINT`, `SIGTSTP`, and `SIGQUIT`.

---

## 7. Conclusion

Sockets and signals represent two complementary pillars of inter-process communication:

- **Sockets** provide flexible, rich, and high-bandwidth communication between processes, whether residing on the same machine or separated across the globe.
- **Signals** provide lightweight, kernel-driven asynchronous triggers that allow the operating system and processes to handle critical runtime events, manage lifecycle states, and maintain overall system stability.

> **Socket = Data Communication (Local & Network)**  
> **Signal = Event & Control Notification**

