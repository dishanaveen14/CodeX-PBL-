# Deadlock Simulation System

An interactive graphical simulation system demonstrating operating system process scheduling, resource allocation, deadlock avoidance using **Banker's Algorithm**, multi-instance **Deadlock Detection**, **Resource Allocation Graph (RAG)** visualization, and **Victim Recovery**.

Developed in pure **C** for Windows desktop environments.

---

## 1. Technology Stack & Requirements

- **Language:** C (Standard C11)
- **Compiler:** GCC (MinGW-w64 / MSYS2 UCRT64)
- **GUI Framework:** Native Windows C API (`user32.dll`, `gdi32.dll`, `comctl32.dll`)
  - Zero external GUI library dependencies or third-party package managers required.
  - Native Windows desktop controls with high performance and minimal memory footprint.
- **Build Utilities:** Make / Batch (`build.bat`) / PowerShell (`build.ps1`)

---

## 2. Project Folder Structure

```text
CodeX-PBL-/
├── bin/                                # Compiled executables (deadlock_sim.exe, test_skeleton.exe)
├── build/                              # Intermediate build files
├── docs/                               # Architectural and developer documentation
│   ├── ARCHITECTURE.md                 # Data contracts, state invariants, module boundaries
│   └── TEAM_GUIDELINES.md              # Team coding standards and contribution guidelines
├── include/                            # Public C headers
│   ├── common/                         # Shared core contracts
│   │   ├── types.h                     # Limits, Enums, Error codes
│   │   ├── system_state.h              # Master SystemState struct & vectors/matrices
│   │   └── error.h                     # Error and status string translation
│   ├── person1_process_resource/       # Person 1 interfaces
│   │   ├── process_mgmt.h              # Process creation, removal, queries
│   │   ├── resource_mgmt.h             # Resource types, instances, queries
│   │   └── matrix_mgmt.h               # Allocation, Maximum, Need, Available calculations
│   ├── person2_bankers/                # Person 2 interfaces
│   │   └── bankers.h                   # Safety algorithm, Safe sequence, Request checks
│   ├── person3_detection_recovery/     # Person 3 interfaces
│   │   ├── detection.h                 # Deadlock detection & Outstanding requests
│   │   ├── rag.h                       # Resource Allocation Graph nodes & edges
│   │   └── recovery.h                  # Victim selection & termination workflow
│   ├── person4_integration/           # Person 4 interfaces
│   │   └── coordinator.h               # Application context & workflow orchestration
│   ├── gui/                            # GUI Shell and tab panels
│   │   ├── gui_app.h                   # Main window, event loop, navigation tabs
│   │   ├── panel_dashboard.h           # System status & metrics dashboard
│   │   ├── panel_setup.h               # Setup panel placeholder (Person 1)
│   │   ├── panel_bankers.h             # Banker's algorithm panel placeholder (Person 2)
│   │   ├── panel_detection.h           # Deadlock detection panel placeholder (Person 3)
│   │   └── panel_rag.h                 # RAG graph visualization panel placeholder
│   └── utils/
│       └── logger.h                    # Component-based logging utility
├── src/                                # C source implementations
│   ├── main.c                          # Application entry point (WinMain / main)
│   ├── common/
│   │   ├── system_state.c              # State lifecycle & memory operations
│   │   └── error.c                     # Error string conversion implementations
│   ├── person1_process_resource/       # Person 1 implementation stubs
│   │   ├── process_mgmt.c              # Process management stubs (TODO)
│   │   ├── resource_mgmt.c             # Resource management stubs (TODO)
│   │   └── matrix_mgmt.c               # Matrix calculation stubs (TODO)
│   ├── person2_bankers/
│   │   └── bankers.c                   # Banker's Algorithm stubs (TODO)
│   ├── person3_detection_recovery/
│   │   ├── detection.c                 # Deadlock detection stubs (TODO)
│   │   ├── rag.c                       # RAG builder stubs (TODO)
│   │   └── recovery.c                  # Recovery operation stubs (TODO)
│   ├── person4_integration/
│   │   └── coordinator.c               # Master workflow orchestration stubs
│   ├── gui/
│   │   ├── gui_app.c                   # Window procedure, tab layout, resize handler
│   │   ├── panel_dashboard.c           # Dashboard UI panel shell
│   │   ├── panel_setup.c               # Person 1 UI panel shell
│   │   ├── panel_bankers.c             # Person 2 UI panel shell
│   │   ├── panel_detection.c           # Person 3 UI panel shell
│   │   └── panel_rag.c                 # RAG visualization canvas shell
│   └── utils/
│       └── logger.c                    # Console/diagnostic logger
├── tests/
│   └── test_skeleton.c                 # Test harness verifying all contracts and stubs
├── build.bat                           # Windows Command Prompt build runner
├── build.ps1                           # Windows PowerShell build runner
├── Makefile                            # Standard GNU Makefile
└── README.md                           # Project documentation
```

---

## 3. Team Responsibilities

| Role | Member | Responsibilities | Assigned Files |
| :--- | :--- | :--- | :--- |
| **Person 1** | Process & Resource Management | **[IMPLEMENTED]** Process creation/removal, Resource instance setup, Allocation and Maximum Matrix entry, automatic Need Matrix and Available Vector calculations, preliminary request validation, resource allocation/release operations, textbook preset loading, and interactive GUI setup panel. | `include/person1_process_resource/*`, `src/person1_process_resource/*`, `include/gui/panel_setup.h`, `src/gui/panel_setup.c` |
| **Person 2** | Banker's Algorithm & Safe State Analysis | **[IMPLEMENTED]** Deadlock avoidance via Banker's Safety Algorithm (Work/Finish tracking), Safe Sequence discovery, Trial-state request evaluation and rollback on unsafe states, step-by-step educational trace generation, and interactive Banker's GUI panel. | `include/person2_bankers/*`, `src/person2_bankers/*`, `include/gui/panel_bankers.h`, `src/gui/panel_bankers.c` |
| **Person 3** | Deadlock Detection, Recovery, & RAG | Multi-instance Deadlock Detection algorithm using Outstanding Requests Matrix, Outstanding Request tracking, RAG graph data builder, victim selection heuristic, and recovery termination. | `include/person3_detection_recovery/*`, `src/person3_detection_recovery/*`, `include/gui/panel_detection.h`, `src/gui/panel_detection.c` |
| **Person 4** | Architecture, GUI Coordination, & Integration | Project skeleton, shared contracts, coordinator orchestration, master GUI window, tab navigation, RAG rendering integration, end-to-end testing, and final delivery. | `include/common/*`, `include/person4_integration/*`, `include/gui/gui_app.*`, `src/main.c`, build configurations, `tests/*` |

---

## 4. Build Instructions

### Prerequisites
Make sure `gcc` is installed and available in your `PATH` (e.g. MSYS2 UCRT64 or MinGW-w64).

### Option A: Using Windows Command Prompt (`build.bat`)
```cmd
# 1. Compile the GUI application
build.bat

# 2. Compile and run the verification test suite
build.bat test

# 3. Clean build artifacts
build.bat clean
```

### Option B: Using Windows PowerShell (`build.ps1`)
```powershell
# 1. Compile the GUI application
.\build.ps1

# 2. Compile and run the test harness
.\build.ps1 test

# 3. Clean build artifacts
.\build.ps1 clean
```

### Option C: Using GNU Make (`make`)
```bash
# 1. Compile application
make

# 2. Compile and run test suite
make test

# 3. Clean build artifacts
make clean
```

---

## 5. Run Instructions

### Launching the Graphical Application
```cmd
# Via build tool:
build.bat run

# Or directly execute the binary:
bin\deadlock_sim.exe
```

When launched, the desktop application displays:
- A responsive Windows window titled **"Deadlock Simulation System - PBL Architecture Skeleton"**.
- A native **Tab Navigation Bar** containing:
  1. **Dashboard:** Live state metrics and system classification (SAFE / UNSAFE / DEADLOCKED).
  2. **Setup & Matrices (P1):** Planned forms and matrix tables for Person 1.
  3. **Banker's Algorithm (P2):** Planned safety checks and safe sequence views for Person 2.
  4. **Deadlock Detection (P3):** Planned detection algorithms, waiting requests, and recovery workflows for Person 3.
  5. **RAG Visualizer (P3/P4):** Canvas layout for rendering nodes and directed edges.
- A **Status Bar** indicating the active module and skeleton readiness.

---

## 6. Verification and Test Suite

To verify that all shared headers, structs, memory rules, and function stubs compile and link correctly:
```cmd
build.bat test
```
The test harness runs 9 test suites with 83 assertions:
- `[TEST SUITE 1] System State Lifecycle & Memory Contracts`
- `[TEST SUITE 2] Person 1: Process Creation, Queries & Safe Removal`
- `[TEST SUITE 3] Person 1: Resource Creation, Instances & Total Updates`
- `[TEST SUITE 4] Person 1: Matrices, Math Invariants (Need & Available)`
- `[TEST SUITE 5] Person 1: Preliminary Request Checks & Resource Release`
- `[TEST SUITE 6] Person 2: Banker's Safety Algorithm (Safe State Analysis)`
- `[TEST SUITE 7] Person 2: Banker's Safety Algorithm (Unsafe State Analysis)`
- `[TEST SUITE 8] Person 2: Banker's Resource-Request Avoidance Algorithm`
- `[TEST SUITE 9] Person 3 & 4 Stubs Pending Implementation`

---

## 7. Instructions for Adding Future Implementations

When implementing feature logic:

1. **Do not modify interface signatures** without coordinating with Person 4.
2. **Locate your assigned file** in `src/<your_module>/`:
   - Person 1: `src/person1_process_resource/`
   - Person 2: `src/person2_bankers/`
   - Person 3: `src/person3_detection_recovery/`
3. Replace the `/* TODO: Person X */` comment and `return ERR_NOT_IMPLEMENTED;` with your algorithm logic.
4. **Follow Memory & State Invariants:**
   - Person 1 updates `allocation`, `maximum`, `need`, and `available` in `SystemState`.
   - Person 2 runs safety checks on copies of vectors (`Work = Available`) and must **never** modify the live state during an analysis or check.
   - Person 3 runs detection against `outstanding_requests` and `allocation`, distinct from `need`.
5. Connect your functional logic to the designated UI panel in `src/gui/panel_<your_module>.c`.
6. Run `build.bat test` to verify your changes against the shared test suite.

