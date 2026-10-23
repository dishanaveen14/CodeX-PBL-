/* fcfs.c - First-Come, First-Served scheduling (Student 1)
 * Non-preemptive.
 * Tie-break: earlier arrival time, then smaller pid.
 * Sets ct and first_start, records Gantt blocks.
 */
#include "common.h"

void fcfs(Process p[], int n, Gantt *g)
{
    if (n <= 0) return;

    int order[MAX_PROC];
    for (int i = 0; i < n; i++) {
        order[i] = i;
        p[i].ct = 0;
        p[i].first_start = -1;
    }

    /* Stable sort (insertion sort) by (at, pid) */
    for (int i = 1; i < n; i++) {
        int k = order[i];
        int j = i - 1;
        while (j >= 0 && (p[order[j]].at > p[k].at ||
              (p[order[j]].at == p[k].at && p[order[j]].pid > p[k].pid))) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = k;
    }

    int t = 0;
    for (int idx = 0; idx < n; idx++) {
        int i = order[idx];
        if (p[i].at > t) {
            /* CPU idle until process arrives */
            gantt_add(g, -1, t, p[i].at);
            t = p[i].at;
        }
        p[i].first_start = t;
        gantt_add(g, p[i].pid, t, t + p[i].bt);
        t += p[i].bt;
        p[i].ct = t;
    }
}
