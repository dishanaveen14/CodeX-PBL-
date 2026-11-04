# Simulation and Comparative Analysis of Page Replacement Algorithms

> **Current Status: Fully Integrated & Tested Application (Week 5)**  
> *All components assigned across the four team members (Person 1: FIFO & LRU, Person 2: Optimal & Engine/Statistics, Person 3: GTK GUI & Input Validation, Person 4: Integration, Build System & Documentation) are fully implemented and verified.*

---

## 1. Project Overview

This application simulates operating system physical memory management using virtual memory paging. It models how an OS page replacement mechanism manages a fixed pool of page frames when memory references occur.

The application allows users to:
1. Provide a page reference string (e.g. `7 0 1 2 0 3 0 4 2 3`) and frame count.
2. Select an individual algorithm (**FIFO**, **LRU**, or **Optimal**) or choose **Compare All**.
3. View the memory frame state at each reference step.
4. Distinguish between page hits and page faults.
5. Compute key performance metrics: total hits, total faults, hit ratio (%), and fault ratio (%).
6. Compare performance across all three algorithms side-by-side with step-by-step detail tabs.

---

## 2. Technology Stack

- **Primary Language:** C (C11 standard)
- **Graphical User Interface:** GTK+ 3.0 (C API)
- **Build System:** GNU Make / Makefile
- **Data Structures:** Standard C arrays, structs, and dynamic memory allocation
- **Version Control:** Git

*No external or secondary languages (such as Python or JavaScript) are used. The simulation engine, algorithms, data structures, statistics, validation, and GUI integration are implemented purely in C.*

---

## 3. Project Directory Structure

```text
CodeX-PBL-/
├── include/
│   └── common.h                # Global constants, limits, and status codes
│
├── src/
│   ├── algorithms/             # Algorithm implementations
│   │   ├── fifo.c              # FIFO algorithm logic (Person 1)
│   │   ├── fifo.h              # FIFO interface
│   │   ├── lru.c               # LRU algorithm logic (Person 1)
│   │   ├── lru.h               # LRU interface
│   │   ├── optimal.c           # Optimal algorithm logic (Person 2)
│   │   └── optimal.h           # Optimal interface
│   │
│   ├── core/                   # Core simulation engine and statistics
│   │   ├── simulation.c        # Common simulation dispatcher (Person 2)
│   │   ├── simulation.h        # Common simulation interface
│   │   ├── simulation_types.h  # Shared data structures (SimulationResult, Step, etc.)
│   │   ├── statistics.c        # Hit/fault ratio calculation (Person 2)
│   │   └── statistics.h        # Statistics interface
│   │
│   ├── gui/                    # GTK Graphical User Interface (Person 3)
│   │   ├── main.c              # Application main entry point
│   │   ├── main_window.c       # Main application window and controls
│   │   ├── main_window.h       # Main window header
│   │   ├── results_view.c      # Step table and comparison visualizer
│   │   ├── results_view.h      # Results view header
│   │   ├── input_validation.c  # Input parser and validator
│   │   └── input_validation.h  # Validation interface
│   │
│   └── utils/                  # Memory management & utility functions
│       ├── memory.c            # Safe allocation and cleanup routines
│       └── memory.h            # Memory utilities header
│
├── tests/                      # Unit and integration test suites
│   ├── test_fifo.c             # FIFO unit tests (Person 1)
│   ├── test_lru.c              # LRU unit tests (Person 1)
│   ├── test_optimal.c          # Optimal unit tests (Person 2)
│   ├── test_simulation.c       # Simulation engine & statistics tests (Person 2)
│   └── test_integration.c      # End-to-end integration tests (Person 4)
│
├── docs/                       # Project documentation
│   └── integration_notes.md    # Shared contracts, signatures, and guidelines
│
├── Makefile                    # Modular build automation script
├── README.md                   # Project documentation and instructions
└── .gitignore                  # Git untracked patterns
```

---

## 4. Team Responsibilities

The workload is partitioned across four members:

| Member | Primary Responsibility | Assigned Files | Status |
|---|---|---|---|
| **Person 1** | FIFO and LRU algorithms | `src/algorithms/fifo.c`, `fifo.h`<br>`src/algorithms/lru.c`, `lru.h`<br>`tests/test_fifo.c`, `test_lru.c` | **Completed** |
| **Person 2** | Optimal algorithm, simulation engine & statistics | `src/algorithms/optimal.c`, `optimal.h`<br>`src/core/simulation.c`, `simulation.h`<br>`src/core/simulation_types.h`<br>`src/core/statistics.c`, `statistics.h`<br>`tests/test_optimal.c`, `test_simulation.c` | **Completed** |
| **Person 3** | GTK GUI and input validation | `src/gui/main.c`<br>`src/gui/main_window.c`, `main_window.h`<br>`src/gui/results_view.c`, `results_view.h`<br>`src/gui/input_validation.c`, `input_validation.h` | **Completed** |
| **Person 4** | Final integration, Makefile, testing & documentation | `Makefile`<br>`README.md`<br>`docs/integration_notes.md`<br>`tests/test_integration.c`<br>`src/utils/memory.c`, `memory.h` | **Completed** |

---

## 5. Dependencies and Prerequisites

To build and run the complete graphical application, install:
- **C Compiler:** `gcc` or `clang` supporting C11
- **Build Tool:** GNU `make`
- **Helper Tool:** `pkg-config`
- **GUI Framework:** GTK+ 3.0 development headers and libraries

### Installing Dependencies

- **Debian / Ubuntu / Linux Mint:**
  ```bash
  sudo apt-get update
  sudo apt-get install build-essential libgtk-3-dev pkg-config
  ```

- **Fedora / RHEL:**
  ```bash
  sudo dnf install gcc make gtk3-devel pkgconfig
  ```

- **Windows (MSYS2 UCRT64):**
  1. Open the **MSYS2 UCRT64** terminal (e.g. `ucrt64.exe` or "MSYS2 UCRT64" from Start Menu).  
     *Ensure the prompt displays `UCRT64` (e.g. `DESKTOP-XXXXX UCRT64 ~`). Do not use `MSYS2 MSYS` or `CLANG64`.*
  2. Navigate to your project directory:
     ```bash
     cd /c/Users/<YourUsername>/Desktop/CodeX-PBL-
     ```
  3. Update package database and core packages:
     ```bash
     pacman -Syu
     ```
  4. Install the required MinGW-w64 UCRT64 toolchain, GTK 3, pkg-config, and make:
     ```bash
     pacman -S --needed \
       mingw-w64-ucrt-x86_64-gcc \
       mingw-w64-ucrt-x86_64-gtk3 \
       mingw-w64-ucrt-x86_64-pkg-config \
       mingw-w64-ucrt-x86_64-make
     ```

### Verifying Environment and Dependencies

Run the following commands in the **MSYS2 UCRT64** terminal to confirm the tools and libraries are properly detected:

1. **Verify GCC Compiler:**
   ```bash
   gcc --version
   which gcc
   ```
   *Expected output: GCC version 13+ or 14+ located in `/ucrt64/bin/gcc`.*

2. **Verify GTK 3 & pkg-config:**
   ```bash
   pkg-config --modversion gtk+-3.0
   pkg-config --cflags --libs gtk+-3.0
   ```
   *Expected output: GTK 3.24.x version string and corresponding include/library flags.*

3. **Verify Make Tool:**
   ```bash
   make --version
   # or mingw32-make --version
   ```

---

## 6. Compilation and Execution

Run all commands from the project root inside the **MSYS2 UCRT64** terminal (or standard Linux shell):

### Build the Full GUI Application
```bash
make
```
*(Alternatively, `mingw32-make` is also supported).*

### Run the Application
```bash
make run
```
*Builds the binary if not already built, then executes `bin/page_simulator.exe` and displays the GUI window.*

### Run All Test Suites
Compiles and executes all unit and integration test suites:
```bash
make test
```

### Run Individual Test Suites
Each test suite can also be compiled and executed independently:
```bash
make test-fifo          # Person 1: FIFO unit tests
make test-lru           # Person 1: LRU unit tests
make test-optimal       # Person 2: Optimal unit tests
make test-simulation    # Person 2: Simulation engine & statistics tests
make test-integration   # Person 4: Full pipeline integration tests
```

### Clean Build Artifacts
Removes all generated object files, executables, and temporary build folders:
```bash
make clean
```

---

## 7. Comparative Benchmark Summary

For the reference string `7 0 1 2 0 3 0 4 2 3` with `3` physical frames:

| Metric | FIFO | LRU | Optimal |
|---|---|---|---|
| **Total References** | 10 | 10 | 10 |
| **Page Hits** | 1 | 2 | 4 |
| **Page Faults** | 9 | 8 | 6 |
| **Hit Ratio (%)** | 10.00% | 20.00% | 40.00% |
| **Fault Ratio (%)** | 90.00% | 80.00% | 60.00% |
