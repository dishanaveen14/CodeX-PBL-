# Interactive Deadlock Simulation System (Week 4 PBL)

A desktop app (Python + tkinter) that simulates resource allocation, safe/unsafe states,
deadlock detection, deadlock avoidance (Banker's Algorithm), the Resource Allocation Graph,
deadlock recovery and resource utilization.

## How to run
```bash
python main.py        # or:  py main.py   (Windows)
```
Needs Python 3.10+ only. tkinter comes with Python. No pip installs.

## How to use
Use the 12 buttons on the left (or File > Load sample for ready-made examples):

| Button | What it does |
|---|---|
| 1. Create Processes | Choose how many processes (1-10) |
| 2. Create Resources | Choose resource types (1-6) and total instances of each |
| 3. Enter Allocation Matrix | What each process currently holds |
| 4. Enter Maximum Matrix | The most each process may ever need |
| 5. Display Need Matrix | Need = Maximum - Allocation |
| 6. Display Resource Allocation Graph | Draws the graph, highlights cycles |
| 7. Request Resources | A process asks for resources (with or without Banker's avoidance) |
| 8. Run Banker's Algorithm | Safe/unsafe check and safe sequence |
| 9. Detect Deadlock | Finds the deadlocked processes |
| 10. Recover from Deadlock | Terminate processes or preempt resources |
| 11. Display System Status | All matrices and the Available vector |
| 12. Exit | Close the app |

The coloured banner at the top always shows the system state:
**SAFE** (green), **UNSAFE** (orange) or **DEADLOCKED** (red).

## Files and owners
| File | Purpose | Owner |
|---|---|---|
| `main.py`, `gui_main.py` | Start-up, main window, dialogs, System Status tab | Team Lead |
| `state.py`, `samples.py` | Shared `SystemState` class and sample scenarios | Team Lead |
| `banker.py`, `gui_banker.py` | Banker's Algorithm, requests, Banker tab | Student 1 |
| `detection.py`, `recovery.py`, `gui_deadlock.py` | Detection, classification, recovery, Deadlock tab | Student 2 |
| `rag.py`, `utilization.py`, `gui_rag.py`, `gui_util.py`, `tests/` | Graph, utilization, tests | Student 3 |

## Sample scenarios (File > Load sample)
- **safe** - 5 processes, 3 resources. Safe sequence P1, P3, P4, P0, P2.
- **unsafe** - same, but P4 holds extra resources. No safe sequence, not deadlocked.
- **deadlock** - 3 processes in a circular wait. All three are deadlocked.

## Team
Team Lead, Student 1, Student 2, Student 3.
