# CPU Process Scheduling Simulator (Week 3 PBL)

A command-line simulator written in **C (C11)** that schedules a set of processes with several
CPU scheduling algorithms and reports the Gantt chart, Waiting Time (WT), Turnaround Time (TAT),
Response Time (RT), their averages, and CPU utilization.

## Algorithms
| # | Algorithm | Owner |
|---|---|---|
| 4 | FCFS | Student 1 |
| 5 | SJF - Non-preemptive | Student 1 |
| 6 | SJF - Preemptive (SRTF) | Student 1 |
| 7 | Priority - Non-preemptive | Student 2 |
| 8 | Priority - Preemptive | Student 2 |
| 9 | Round Robin (configurable time quantum) | Student 2 |
| 10 | Run all and show a comparison table | Team Lead / Student 3 |

## Build and run
```bash
make            # builds ./scheduler  (gcc -Wall -Wextra -std=c11)
./scheduler
make clean
```

## Input format
For each process the program asks for:
- **Process ID** (1-9999, must be unique)
- **Arrival Time** (0-1000)
- **Burst Time** (1-100)
- **Priority** (0-100) - **lower number = higher priority**

Round Robin additionally asks for the **time quantum** (1-100) when it is run.
Option 2 generates random processes (you choose how many and the maximum values).
You can enter the processes once and switch between algorithms without re-entering data.

## Conventions used by all algorithms
- Time starts at 0; CPU idle time appears in the Gantt chart as an IDLE block.
- Ties are broken by earlier arrival time, then smaller PID.
- WT = TAT - BT, TAT = CT - AT, RT = first CPU time - AT.
- CPU utilization = busy time / total time x 100.

## Menu
```
1. Enter processes manually      6. Run SJF (preemptive / SRTF)
2. Generate random processes     7. Run Priority (non-preemptive)
3. Display current processes     8. Run Priority (preemptive)
4. Run FCFS                      9. Run Round Robin
5. Run SJF (non-preemptive)     10. Run ALL algorithms and compare
0. Exit
```

## Project structure and ownership
| File | Purpose | Owner |
|---|---|---|
| `common.h` / `common.c` | Shared structs, function contract, `gantt_add()` | Team Lead |
| `main.c` | CLI menu, input, random generation, integration | Team Lead |
| `fcfs.c`, `sjf.c` | FCFS, SJF (NP + preemptive) | Student 1 |
| `priority.c`, `rr.c` | Priority (NP + preemptive), Round Robin | Student 2 |
| `metrics.c`, `gantt.c` | Metrics, Gantt chart, tables, comparison | Student 3 |
| `tests/` | Test cases and sample outputs | Student 3 |
| `Makefile`, `README.md` | Build and documentation | Team Lead |

## Known limitations
- Context-switch time is assumed to be zero.
- The Gantt buffer holds 2000 blocks; extremely long Round Robin runs (very small quantum with
  many long processes) may truncate the *chart* only - the metrics stay correct.

## Sample session
_To be filled in after all modules are merged (see `tests/`)._

## Team
Team Lead, Student 1, Student 2, Student 3 - see ownership table above.
