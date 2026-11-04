# Integration Notes and Architecture Contract

**Project Title:** Simulation and Comparative Analysis of Page Replacement Algorithms  
**Integration Owner:** Person 4  
**Date:** March 2026 / Week 5  

---

## 1. Overview and Architecture

This document establishes the binding interface contract for the 4 team members working on the C-based Page Replacement Algorithm Simulator. All members must implement strictly against these contracts so that parallel development can proceed without interface conflicts or integration regressions.

### Module Ownership

| Member | Primary Modules & Files | Assigned Responsibility |
|---|---|---|
| **Person 1** | `src/algorithms/fifo.c`, `fifo.h`<br>`src/algorithms/lru.c`, `lru.h`<br>`tests/test_fifo.c`, `test_lru.c` | FIFO and LRU algorithm implementations, hit/fault tracking, local unit tests |
| **Person 2** | `src/algorithms/optimal.c`, `optimal.h`<br>`src/core/simulation_types.h`<br>`src/core/simulation.c`, `simulation.h`<br>`src/core/statistics.c`, `statistics.h`<br>`tests/test_optimal.c`, `test_simulation.c` | Optimal (Belady's Min) algorithm, shared data structures, core simulation dispatcher, performance statistics calculation |
| **Person 3** | `src/gui/main.c`<br>`src/gui/main_window.c`, `main_window.h`<br>`src/gui/results_view.c`, `results_view.h`<br>`src/gui/input_validation.c`, `input_validation.h` | GTK 3 graphical user interface, user input validation, step-by-step frame table visualization, comparison views |
| **Person 4** | `Makefile`<br>`README.md`<br>`docs/integration_notes.md`<br>`tests/test_integration.c` | Build system, documentation, end-to-end integration testing, memory leak audits, release packaging |

---

## 2. Shared Data Structures (`src/core/simulation_types.h`)

All modules adhere to the agreed definitions below. No member may alter or rename these definitions without team consensus.

```c
#define EMPTY_FRAME (-1)

typedef enum {
    PAGE_HIT,
    PAGE_FAULT
} PageStatus;

typedef struct {
    int page;            /* The requested page number (>= 0) */
    PageStatus status;   /* PAGE_HIT or PAGE_FAULT */
    int *frames;         /* Snapshot array of size frame_count containing resident pages */
} SimulationStep;

typedef struct {
    char algorithm_name[16]; /* "FIFO", "LRU", or "Optimal" */

    int total_references;    /* Length of reference string */
    int total_hits;          /* Number of hits */
    int total_faults;        /* Number of page faults */

    double hit_ratio;        /* (total_hits / total_references) * 100.0 */
    double fault_ratio;      /* (total_faults / total_references) * 100.0 */

    int frame_count;         /* Number of physical frames */
    SimulationStep *steps;   /* Array of size total_references */
} SimulationResult;
```

---

## 3. Module Interfaces and Signatures

### A. Algorithm Module Interfaces (`src/algorithms/`)

All algorithm functions share an identical function signature and memory contract:

```c
SimulationResult *run_fifo(
    const int references[],
    int reference_count,
    int frame_count
);

SimulationResult *run_lru(
    const int references[],
    int reference_count,
    int frame_count
);

SimulationResult *run_optimal(
    const int references[],
    int reference_count,
    int frame_count
);
```

**Contract Rules:**
1. Each algorithm allocates its own `SimulationResult` using `create_simulation_result()` from `utils/memory.h`.
2. Input references must be non-negative integers (`page >= 0`). Empty frames in the `frames` array must be set to `EMPTY_FRAME` (`-1`).
3. Each reference must record exactly one `SimulationStep` containing the requested page, hit/fault status, and a full snapshot copy of all frames after processing.
4. If invalid arguments are supplied (e.g. `references == NULL`, `reference_count <= 0`, or `frame_count <= 0`), the function must return `NULL`.

---

### B. Common Simulation Engine (`src/core/simulation.h`)

The GUI invokes the simulation engine exclusively through this single unified API:

```c
typedef enum {
    ALGORITHM_FIFO,
    ALGORITHM_LRU,
    ALGORITHM_OPTIMAL,
    ALGORITHM_ALL
} AlgorithmChoice;

SimulationResult **run_simulation(
    const int references[],
    int reference_count,
    int frame_count,
    AlgorithmChoice choice,
    int *result_count
);
```

**Dispatch Behavior:**
- `ALGORITHM_FIFO`: Runs `run_fifo`, sets `*result_count = 1`.
- `ALGORITHM_LRU`: Runs `run_lru`, sets `*result_count = 1`.
- `ALGORITHM_OPTIMAL`: Runs `run_optimal`, sets `*result_count = 1`.
- `ALGORITHM_ALL`: Runs all three algorithms sequentially on the exact same input, sets `*result_count = 3`.

---

### C. Statistics Calculations (`src/core/statistics.h`)

Formula requirements:
$$\text{Hit Ratio} = \left(\frac{\text{Total Hits}}{\text{Total References}}\right) \times 100$$
$$\text{Fault Ratio} = \left(\frac{\text{Total Faults}}{\text{Total References}}\right) \times 100$$

Functions:
```c
int calculate_total_hits(const SimulationStep steps[], int reference_count);
int calculate_total_faults(const SimulationStep steps[], int reference_count);
double calculate_hit_ratio(int total_hits, int total_references);
double calculate_fault_ratio(int total_faults, int total_references);
void update_simulation_statistics(SimulationResult *result);
```

---

### D. Input Validation (`src/gui/input_validation.h`)

```c
ValidationStatus parse_and_validate_reference_string(
    const char *input_str,
    int **out_references,
    int *out_count
);

ValidationStatus validate_frame_count(int frame_count);
void free_parsed_references(int *references);
```

- Disallows negative numbers, non-digits, and empty input.
- Validates frame count in range `[1, 64]`.

---

## 4. Memory Ownership Rules

To prevent segmentation faults and memory leaks:

1. **Input Reference Array:**
   - Allocated by `parse_and_validate_reference_string()` in Person 3's module.
   - Freed by the caller (GUI event handler or test) using `free_parsed_references()`.
2. **SimulationResult Structure:**
   - Allocated by `create_simulation_result()` inside the algorithm or simulation engine.
   - Includes inner `SimulationStep` array and per-step `frames` arrays.
   - Freed exclusively by calling `free_simulation_result()` (for single result) or `free_simulation_results()` (for array of results).
3. **No Dangling Pointers:**
   - Cleanups reset pointers to `NULL`.
   - Never free individual frame snapshot pointers from outside `utils/memory.c`.

---

## 5. Integration Workflow for Team Members

1. **Person 1 (FIFO / LRU):**
   - Implement the actual replacement queues and timestamps in `src/algorithms/fifo.c` and `src/algorithms/lru.c`.
   - Run `tests/test_fifo.c` and `tests/test_lru.c`.
2. **Person 2 (Optimal & Engine):**
   - Implement forward-lookahead page replacement in `src/algorithms/optimal.c`.
   - Verify statistics with `tests/test_optimal.c` and `tests/test_simulation.c`.
3. **Person 3 (GUI & Presentation):**
   - Populate `src/gui/results_view.c` with GTK tables (`GtkGrid` or `GtkTreeView`).
   - Connect hit/fault visual styling and comparison charts/tables.
4. **Person 4 (Integration & Release):**
   - Run end-to-end tests via `tests/test_integration.c`.
   - Validate clean compilation with `make clean && make`.
   - Verify memory with Valgrind / AddressSanitizer.

