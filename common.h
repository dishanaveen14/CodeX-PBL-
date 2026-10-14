/* common.h - shared contract for the whole team.
 * Owned by the Team Lead. Do NOT edit without telling the lead. */
#ifndef COMMON_H
#define COMMON_H

#define MAX_PROC  100
#define MAX_GANTT 2000

typedef struct {
    int pid, at, bt, pr;     /* input: ID, arrival, burst, priority (lower number = higher priority) */
    int ct, first_start;     /* set by the algorithm: completion time, first time on CPU */
    int wt, tat, rt;         /* set by compute_metrics() */
} Process;

typedef struct { int pid; int start; int end; } GanttEntry;   /* pid = -1 means CPU idle */
typedef struct { GanttEntry e[MAX_GANTT]; int count; } Gantt;
typedef struct { double avg_wt, avg_tat, avg_rt, cpu_util; } Stats;

void gantt_add(Gantt *g, int pid, int start, int end);   /* common.c (Lead) */

/* Student 1 */
void fcfs(Process p[], int n, Gantt *g);
void sjf_np(Process p[], int n, Gantt *g);
void sjf_p(Process p[], int n, Gantt *g);          /* SRTF */
/* Student 2 */
void priority_np(Process p[], int n, Gantt *g);
void priority_p(Process p[], int n, Gantt *g);
void round_robin(Process p[], int n, int tq, Gantt *g);
/* Student 3 */
void  compute_metrics(Process p[], int n);
Stats compute_stats(const Process p[], int n, const Gantt *g);
void  print_gantt(const Gantt *g);
void  print_process_table(const Process p[], int n);
void  print_summary(const Process p[], int n, const Gantt *g);
void  print_comparison(const char *names[], const Stats s[], int count);

#endif
