# Inter-Process Communication (IPC)
## Shared Memory and Semaphores

## 1. Shared Memory

**Shared memory** is an IPC mechanism that allows two or more processes to access the same region of memory. It is one of the fastest IPC methods because data can be accessed directly without repeated copying.

### How It Works

1. Create a shared memory segment.
2. Attach it to the processes.
3. Processes read/write data.
4. Use synchronization when required.
5. Detach and remove the segment when finished.

```text
Process A
    |
    v
+------------------+
|  Shared Memory   |
|      Data        |
+------------------+
    ^
    |
Process B
```

### Advantages
- Very fast communication
- Efficient for large amounts of data
- Reduces data copying

### Disadvantages
- Requires synchronization
- Can cause race conditions
- Needs careful access control

---

## 2. Semaphores

A **semaphore** is a synchronization mechanism used to control access to shared resources. It prevents processes from interfering with each other when accessing shared data.

The two basic operations are:

- **Wait (P/down):** Acquire a resource.
- **Signal (V/up):** Release a resource.

### Types of Semaphores

**Binary Semaphore:** Has values `0` or `1` and is commonly used for mutual exclusion.

**Counting Semaphore:** Can have values greater than `1` and is used when multiple identical resources are available.

### Critical Section

A **critical section** is the part of a program where shared data is accessed.

```text
wait(semaphore)
      |
      v
+------------------+
| Critical Section |
+------------------+
      |
      v
signal(semaphore)
```

---

## 3. Shared Memory + Semaphores

Shared memory and semaphores are often used together:

- **Shared Memory → Communication**
- **Semaphore → Synchronization**

For example, in a producer-consumer system, shared memory stores the data while semaphores control when the producer can write and the consumer can read.

```text
Producer
   |
   | Write
   v
+------------------+
|  Shared Buffer   |
+------------------+
   |
   | Read
   v
Consumer

Semaphores → Control access
```

---

## 4. Common Problems

- **Race Condition:** Multiple processes access shared data at the same time, producing incorrect results.
- **Deadlock:** Processes wait indefinitely for resources held by each other.
- **Starvation:** A process waits for a long time because other processes repeatedly get access first.

---

## 5. Comparison

| Feature | Shared Memory | Semaphore |
|---|---|---|
| Purpose | Exchange data | Synchronize processes |
| Stores data | Yes | No |
| Controls access | No | Yes |
| Main benefit | High-speed communication | Safe resource access |

---

## 6. Applications

- Operating systems
- Producer-consumer systems
- Database systems
- Web servers
- Concurrent applications
- High-performance computing

---

## 7. Conclusion

**Shared memory** provides fast communication by allowing processes to access common data. **Semaphores** provide synchronization and control access to shared resources.

> **Shared Memory = Communication**  
> **Semaphore = Synchronization**
