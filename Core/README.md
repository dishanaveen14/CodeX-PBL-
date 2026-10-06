# Student 2 Core Process

A small, terminal-based Operating Systems mini-project. The Core Process manages CPU operations, simulated memory, a stack, and a queue. It reads commands from the terminal and calls the corresponding module functions.

## Requirements

- GCC (the project was compiled with MinGW GCC on Windows)
- No build system or external libraries are required

## Compile

Open Command Prompt or PowerShell in this folder and run:

```cmd
gcc main.c cpu.c memory.c stack.c queue.c -o core.exe
```

If Windows reports that it cannot write `core.exe`, close any running copy of the program and compile again. You can also choose another output name, such as `terminal_core.exe`.

## Run

In Command Prompt:

```cmd
core.exe
```

In PowerShell:

```powershell
.\core.exe
```

The menu accepts these choices:

1. **Execute CPU Operation** — enter `ADD`, `SUB`, `MUL`, or `DIV`, then two integer operands. Division by zero is reported as an error.
2. **Write to Memory** — enter an address from 0 to 99 and an integer value.
3. **Read from Memory** — read a previously written address.
4. **Push to Stack** — add an integer to the stack.
5. **Pop from Stack** — remove the most recently pushed integer.
6. **Enqueue** — add an integer to the back of the queue.
7. **Dequeue** — remove an integer from the front of the queue.
8. **Display Core State** — show the last successful CPU operation and the current memory, stack, and queue contents.
9. **Exit** — close the program.

For example, to add 12 and 12, choose option `1`, enter `ADD` at the operation prompt, then enter `12` for each value.

## Modules

| Files | Purpose |
|---|---|
| `main.c` | Initializes the Core state, displays the menu, validates numeric input, and dispatches each choice. |
| `cpu.c`, `cpu.h` | Executes integer addition, subtraction, multiplication, and division. |
| `memory.c`, `memory.h` | Stores and retrieves values in 100 simulated memory addresses. |
| `stack.c`, `stack.h` | Implements a 10-item integer stack, with push, pop, peek, and display functions. |
| `queue.c`, `queue.h` | Implements a 10-item circular integer queue, with enqueue, dequeue, peek, and display functions. |

Stack and queue `peek` functions are available in their modules but are not separate menu options. Stack, queue, and memory contents exist only while the program is running; they are reset when it exits.

## Error handling

The program reports invalid menu choices, malformed integer input, invalid or unwritten memory locations, division by zero, stack overflow or underflow, and queue overflow or underflow. It checks these conditions instead of accessing invalid memory or removing items from an empty container.

## Architecture

```text
Terminal input
      |
      v
    main.c
      |
      +---- cpu.c
      +---- memory.c
      +---- stack.c
      +---- queue.c
      |
      v
Terminal output
```

This version is a standalone terminal demonstration. It does not use IPC; the module functions can later be called by a command handler connected to an IPC mechanism.
