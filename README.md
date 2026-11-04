# Simulation and Comparative Analysis of Page Replacement Algorithms

> **Current Status: Structural Skeleton Phase (Week 5)**  
> *Note: This codebase currently establishes the foundational project structure, common data structures, module interfaces, memory management rules, and build scripts. Placeholder function bodies and TODO markers are established for team members. The actual page replacement algorithms (FIFO, LRU, Optimal) and GUI table rendering are pending independent development by assigned team members.*

---

## 1. Project Overview

This application simulates operating system physical memory management using virtual memory paging. It models how an OS page replacement mechanism manages a fixed pool of page frames when memory references occur.

The application allows users to:
1. Provide a page reference string (e.g. `7 0 1 2 0 3 0 4 2 3`) and frame count.
2. Select an individual algorithm (**FIFO**, **LRU**, or **Optimal**) or choose **Compare All**.
3. View the memory frame state at each reference step.
4. Distinguish between page hits and page faults.
5. Compute key performance metrics: total hits, total faults, hit ratio (%), and fault ratio (%).
6. Compare performance across all three algorithms side-by-side.

---

## 2. Technology Stack

- **Primary Language:** C (C11 standard)
- **Graphical User Interface:** GTK+ 3.0 (C API)
- **Build System:** GNU Make / Makefile
- **Data Structures:** Standard C arrays, structs, and dynamic memory allocation
- **Version Control:** Git

*No external or secondary languages (such as Python or JavaScript) are used. The simulation engine, data structures, statistics, validation, and GUI integration are implemented purely in C.*

---

## 3. Project Directory Structure

```text
page-replacement-simulator/
│
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

| Member | Primary Responsibility | Assigned Files |
|---|---|---|
| **Person 1** | FIFO and LRU algorithms | `src/algorithms/fifo.c`, `fifo.h`<br>`src/algorithms/lru.c`, `lru.h`<br>`tests/test_fifo.c`, `test_lru.c` |
| **Person 2** | Optimal algorithm, simulation engine & statistics | `src/algorithms/optimal.c`, `optimal.h`<br>`src/core/simulation.c`, `simulation.h`<br>`src/core/simulation_types.h`<br>`src/core/statistics.c`, `statistics.h`<br>`tests/test_optimal.c`, `test_simulation.c` |
| **Person 3** | GTK GUI and input validation | `src/gui/main.c`<br>`src/gui/main_window.c`, `main_window.h`<br>`src/gui/results_view.c`, `results_view.h`<br>`src/gui/input_validation.c`, `input_validation.h` |
| **Person 4** | Final integration, Makefile, testing & documentation | `Makefile`<br>`README.md`<br>`docs/integration_notes.md`<br>`tests/test_integration.c`<br>`src/utils/memory.c`, `memory.h` |

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

- **Windows (MSYS2 UCRT64 / MinGW):**
  ```bash
  pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gtk3 mingw-w64-ucrt-x86_64-pkg-config make
  ```

---

## 6. Compilation and Execution

### Build the Full GUI Application
```bash
make
```

### Run the Application
```bash
make run
```

### Run the Test Suites
The unit and integration test suites can be compiled and executed directly:
```bash
make test
```

### Clean Build Artifacts
```bash
make clean
```

---

## 7. Memory Management & Ownership Rules

- Dynamic allocation is managed through `src/utils/memory.c`.
- **References:** Allocated during parsing in `parse_and_validate_reference_string()` and freed by caller with `free_parsed_references()`.
- **Simulation Results:** Allocated by `create_simulation_result()` and freed recursively via `free_simulation_result()` or `free_simulation_results()`.
- All frame snapshots are allocated and freed cleanly without memory leaks or dangling pointers.

