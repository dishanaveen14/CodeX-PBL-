/* gantt.c - Gantt chart and formatted output (Student 3)
 * Displays visual ASCII Gantt chart, process execution table, summary, and comparison.
 */
#include <stdio.h>
#include "common.h"

void print_gantt(const Gantt *g)
{
    if (g == 0 || g->count == 0) {
        printf("  [Empty Gantt Chart]\n");
        return;
    }

    /* Top bar */
    printf(" ");
    for (int i = 0; i < g->count; i++) {
        printf("+-------");
    }
    printf("+\n");

    /* Process labels */
    printf(" ");
    for (int i = 0; i < g->count; i++) {
        if (g->e[i].pid == -1)
            printf("|  IDLE ");
        else
            printf("| P%-4d ", g->e[i].pid);
    }
    printf("|\n");

    /* Bottom bar */
    printf(" ");
    for (int i = 0; i < g->count; i++) {
        printf("+-------");
    }
    printf("+\n");

    /* Time markers */
    printf("%-2d", g->e[0].start);
    for (int i = 0; i < g->count; i++) {
        printf("%8d", g->e[i].end);
    }
    printf("\n");
}

void print_process_table(const Process p[], int n)
{
    if (n <= 0) return;
    printf("%-6s %-8s %-8s %-8s %-6s %-6s %-6s %-6s\n",
           "PID", "Arrival", "Burst", "Priority", "CT", "WT", "TAT", "RT");
    printf("------ -------- -------- -------- ------ ------ ------ ------\n");
    for (int i = 0; i < n; i++) {
        printf("P%-5d %-8d %-8d %-8d %-6d %-6d %-6d %-6d\n",
               p[i].pid, p[i].at, p[i].bt, p[i].pr,
               p[i].ct, p[i].wt, p[i].tat, p[i].rt);
    }
}

void print_summary(const Process p[], int n, const Gantt *g)
{
    Stats s = compute_stats(p, n, g);
    printf("Summary Metrics:\n");
    printf("  Average Waiting Time (WT)    : %.2f\n", s.avg_wt);
    printf("  Average Turnaround Time (TAT): %.2f\n", s.avg_tat);
    printf("  Average Response Time (RT)   : %.2f\n", s.avg_rt);
    printf("  CPU Utilization              : %.2f%%\n", s.cpu_util);
}

void print_comparison(const char *names[], const Stats s[], int count)
{
    printf("\n%-26s %-12s %-12s %-12s %-14s\n",
           "Algorithm", "Avg WT", "Avg TAT", "Avg RT", "CPU Util (%)");
    printf("-------------------------- ------------ ------------ ------------ --------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-26s %-12.2f %-12.2f %-12.2f %-13.2f%%\n",
               names[i], s[i].avg_wt, s[i].avg_tat, s[i].avg_rt, s[i].cpu_util);
    }
}
