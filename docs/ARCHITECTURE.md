# Deadlock Simulation System — Architecture Specification

## 1. Architectural Philosophy and Overview

The **Deadlock Simulation System** is an interactive desktop application engineered entirely in the **C programming language (C11 standard)**. The application simulates how an operating system allocates shared, multi-instance resource types across competing concurrent processes, determines safety using **Banker's Algorithm**, detects deadlocks using an **Outstanding Request Matrix**, renders the state via a **Resource Allocation Graph (RAG)**, and resolves deadlocks via **victim recovery**.

To allow four team members to develop their respective components in parallel without interface mismatches or merge chaos, this architecture establishes:
1. **Single Source of Truth:** A central `SystemState` struct owned and updated through designated interfaces.
2. **Defensive Copies & Isolation:** Safety and avoidance algorithms execute against temporary clones/work vectors, never inadvertently mutating live allocations.
3. **Strict Separation of Concerns:**
   - **Need Matrix** (Maximum demand minus current allocation) is strictly separated from **Outstanding Request Matrix** (current unsatisfied blocking request).
   - **UNSAFE** is strictly distinguished from **DEADLOCKED**.
4. **Uniform C Calling Conventions:** Standard error codes (`ErrorCode`), documented struct contracts, and predictable ownership semantics.

---

## 2. Team Structure & Module Ownership

| Member | Focus Domain | Key Responsibilities | Primary Interface Directory |
| :--- | :--- | :--- | :--- |
| **Person 1** | Process & Resource Management | Process creation/removal, Resource types & instances, Allocation & Maximum matrices, Need & Available calculation, request submission | `include/person1_process_resource/` |
| **Person 2** | Banker's Algorithm & Safety Analysis | Safety algorithm (Work/Finish vectors), Safe sequence discovery, Request evaluation on trial states, step-by-step trace | `include/person2_bankers/` |
| **Person 3** | Deadlock Detection, Recovery & RAG | Deadlock detection algorithm, Outstanding request tracking, RAG graph data builder, victim selection & recovery | `include/person3_detection_recovery/` |
| **Person 4** | Architecture, GUI Coordination & Integration | Shared data contracts, central coordinator, main GUI window, navigation tabs, status dispatch, final test harness | `include/person4_integration/`, `include/gui/` |

---

## 3. Shared Data Contract

The foundational definitions reside in [`include/common/types.h`](file:///c:/Users/LENOVO/Desktop/CodeX-PBL-/include/common/types.h) and [`include/common/system_state.h`](file:///c:/Users/LENOVO/Desktop/CodeX-PBL-/include/common/system_state.h).

### 3.1 Dimensions and Indexing Invariants
```c
#define MAX_PROCESSES   16   /* Maximum concurrent processes supported */
#define MAX_RESOURCES   16   /* Maximum distinct resource types supported */
```
- **Process Indexing:** Every process has a unique integer ID $i \in [0, \text{process\_count} - 1]$. The process list defines the row index for all matrices.
- **Resource Indexing:** Every resource type has a unique integer ID $j \in [0, \text{resource\_count} - 1]$. The resource list defines the column index for all vectors and matrices.

### 3.2 Key Data Structures

#### `SystemState`
The master state representing the simulated OS:
```c
typedef struct {
    int process_count;
    int resource_count;
    Process processes[MAX_PROCESSES];
    Resource resources[MAX_RESOURCES];
    int total[MAX_RESOURCES];                           /* Total physical instances */
    int available[MAX_RESOURCES];                       /* Available instances vector */
    int allocation[MAX_PROCESSES][MAX_RESOURCES];       /* Held instances */
    int maximum[MAX_PROCESSES][MAX_RESOURCES];          /* Declared maximum demand */
    int need[MAX_PROCESSES][MAX_RESOURCES];             /* Need = Maximum - Allocation */
    int outstanding_requests[MAX_PROCESSES][MAX_RESOURCES]; /* Current pending requests */
    SystemSafetyStatus overall_status;                  /* SAFE, UNSAFE, DEADLOCKED */
    char status_message[MAX_DESC_LEN];
} SystemState;
```

#### Vector & Matrix Mathematical Rules
$$\text{Need}[i][j] = \text{Maximum}[i][j] - \text{Allocation}[i][j]$$
$$\text{Available}[j] = \text{Total}[j] - \sum_{i=0}^{\text{process\_count}-1} \text{Allocation}[i][j]$$

- **Validity Constraints:**
  - $\forall i, j: \text{Allocation}[i][j] \ge 0$, $\text{Maximum}[i][j] \ge 0$, $\text{Need}[i][j] \ge 0$.
  - $\forall i, j: \text{Allocation}[i][j] \le \text{Maximum}[i][j]$.
  - $\forall j: \sum_{i} \text{Allocation}[i][j] \le \text{Total}[j]$.

---

## 4. Module Interface Boundaries

### 4.1 Person 1: Process and Resource Management
Header files:
- `include/person1_process_resource/process_mgmt.h`:
  - `sys_process_create(state, name, out_id)`
  - `sys_process_remove(state, id)`
  - `sys_process_get(state, id, out_process)`
- `include/person1_process_resource/resource_mgmt.h`:
  - `sys_resource_create(state, name, total, out_id)`
  - `sys_resource_set_total(state, id, total)`
  - `sys_resource_get(state, id, out_resource)`
- `include/person1_process_resource/matrix_mgmt.h`:
  - `sys_matrix_set_allocation(state, pid, rid, instances)`
  - `sys_matrix_set_maximum(state, pid, rid, instances)`
  - `sys_matrix_calculate_need(state)`
  - `sys_matrix_calculate_available(state)`
  - `sys_matrix_validate_all(state)`
  - `sys_request_validate_preliminary(state, pid, request_vector)`
  - `sys_allocation_apply(state, pid, request_vector)`
  - `sys_resources_release(state, pid, release_vector)`

### 4.2 Person 2: Banker's Algorithm and Safety Analysis
Header file:
- `include/person2_bankers/bankers.h`:
  - `banker_run_safety_check(state, out_result)`
  - `banker_find_safe_sequence(state, out_sequence)`
  - `banker_evaluate_resource_request(state, pid, req_vector, out_result)`
  - `banker_get_safety_explanation(result)`

**Protocol for Request Evaluation:**
1. Check $\text{Request} \le \text{Need}[i]$.
2. Check $\text{Request} \le \text{Available}$.
3. Create a trial copy `SystemState trial = *state`.
4. Apply allocation tentatively on `trial`.
5. Run safety check on `trial`.
6. If safe, request is approved and Person 1's `sys_allocation_apply` can be called. If unsafe, request is denied and original `state` remains untouched.

### 4.3 Person 3: Deadlock Detection, Recovery, and RAG
Header files:
- `include/person3_detection_recovery/detection.h`:
  - `detection_run(state, out_result)`
  - `detection_record_outstanding_request(state, pid, req_vector)`
  - `detection_clear_outstanding_request(state, pid)`
- `include/person3_detection_recovery/rag.h`:
  - `rag_build_from_state(state, out_graph)`
  - `rag_reset(graph)`
- `include/person3_detection_recovery/recovery.h`:
  - `recovery_select_victim(state, deadlock_result, out_victim_id)`
  - `recovery_execute_victim_termination(state, victim_id, out_result)`

**Distinction Rule:**
- **Deadlock detection** uses `allocation` and `outstanding_requests`. It identifies processes actively blocked waiting on instances held by other blocked processes.
- **Banker's Algorithm** uses `allocation` and `need`. It determines if future potential requests could lead to deadlock.

### 4.4 Person 4: System Integration Coordinator
Header file:
- `include/person4_integration/coordinator.h`:
  - Holds `SystemContext` encapsulating master state, cached safety result, cached deadlock result, and cached RAG model.
  - Exposes unified workflows: `coordinator_init`, `coordinator_handle_resource_request`, `coordinator_run_full_analysis`, `coordinator_execute_recovery`, and `coordinator_sync_all_views`.

---

## 5. GUI Architecture (Native Win32 C Application)

The GUI utilizes the native Windows C controls (`user32`, `gdi32`, `comctl32`).
- **Main Window:** Hosted in `src/gui/gui_app.c`. Responsive resizing with `WM_SIZE` and minimum tracking dimensions ($800 \times 600$).
- **Tab Controller:** Five tabs for modular separation:
  1. **Dashboard:** System state overview, health status, and live metrics.
  2. **Setup & Matrices (P1):** Process/Resource creation, Matrix editors.
  3. **Banker's Algorithm (P2):** Safety check trigger, Safe sequence, Trace.
  4. **Deadlock Detection (P3):** Detection analysis, Waiting requests, Recovery.
  5. **RAG Visualizer (P3/P4):** Vector drawing canvas with nodes and directional arrows.
- **Status Bar:** Bottom bar showing active module status and user feedback.

---

## 6. Error Handling and Memory Conventions

- **Error Codes:** Functions return an `ErrorCode` enum (`ERR_SUCCESS` on success, negative values on error).
- **No Uncontrolled Heap Allocations:** Fixed-capacity data buffers (`MAX_PROCESSES`, `MAX_RESOURCES`) avoid heap fragmentation and memory leaks.
- **Pure C Calling:** No hidden class states or language runtime dependencies.
- **Defensive State Cloning:** Any speculative operation must clone the state using `system_state_clone()` before applying changes.

