# Inter-Process Communication (IPC)
## Pipes and Message Queues

---

## 1. Pipes

**A pipe** is an IPC mechanism that allows one process to send data to another process. It provides a communication channel between processes.

### How It Works

1. A pipe is created.
2. One process writes data into the pipe.
3. Another process reads the data from the pipe.
4. Data flows through the pipe in one direction for a normal pipe.

### Diagram

```text
        Write
Process A ──────────► ┌───────────────┐
                      │     PIPE      │
                      │     Data      │
                      └───────────────┘
                              │
                              ▼
                         Process B
                            Read
```

### Types of Pipes

**1. Anonymous Pipe**

- Used mainly between related processes.
- Usually created between a parent and child process.
- Has no permanent name.

```text
Parent Process
      |
      | Write
      ▼
 ┌──────────┐
 │   PIPE   │
 └──────────┘
      |
      | Read
      ▼
 Child Process
```

**2. Named Pipe (FIFO)**

- Has a name in the file system.
- Can be used by unrelated processes.
- Allows processes to communicate through the named pipe.

```text
Process A
    |
    | Write
    ▼
┌──────────────┐
│ Named Pipe   │
│    (FIFO)    │
└──────────────┘
    ▲
    |
    | Read
Process B
```

### Advantages

- Simple IPC mechanism
- Easy to implement
- Useful for communication between processes
- Provides sequential data transfer

### Disadvantages

- Normal pipes are generally one-way
- Limited communication compared with shared memory
- Data must pass through the pipe

---

## 2. Message Queues

A **message queue** is an IPC mechanism that allows processes to communicate by sending and receiving messages through a queue.

### How It Works

1. A message queue is created.
2. The sender places a message into the queue.
3. The message waits in the queue.
4. The receiver retrieves the message from the queue.

### Diagram

```text
Sender Process
      |
      | Send Message
      ▼
┌─────────────────────┐
│    Message Queue    │
├─────────────────────┤
│ Message 1           │
│ Message 2           │
│ Message 3           │
└─────────────────────┘
      |
      | Receive Message
      ▼
Receiver Process
```

### Advantages

- Processes do not need to communicate directly.
- Multiple messages can be stored.
- Processes can work independently.
- Messages can be organized according to their type or priority.

### Disadvantages

- Slower than shared memory for large amounts of data.
- Queue size is limited.
- Requires system resources to maintain the queue.

---

## 3. Pipes vs Message Queues

| Feature | Pipes | Message Queues |
|---|---|---|
| Communication | Data stream | Individual messages |
| Data storage | Temporary stream | Messages stored in queue |
| Direction | Usually one-way | Can support two-way communication |
| Message boundaries | Not preserved | Preserved |
| Communication | Simple | More structured |
| Suitable for | Simple data transfer | Structured message exchange |

---

## 4. Common Problems

- **Blocking:** A process may have to wait until data is available.
- **Buffer limitation:** Pipes and queues have limited storage.
- **Synchronization:** Processes must coordinate their communication.
- **Deadlock:** Processes may wait indefinitely for each other.

---

## 5. Applications

- Parent-child process communication
- Producer-consumer systems
- Operating systems
- Client-server communication
- Concurrent applications

---

## 6. Conclusion

**Pipes** provide a simple channel for transferring data between processes, while **Message Queues** allow processes to exchange structured messages through a queue.

> **Pipe = Data Stream Communication**  
> **Message Queue = Message-Based Communication**
