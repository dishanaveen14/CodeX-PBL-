/* metrics.c - Metrics computation (Student 3)
 * WT = TAT - BT, TAT = CT - AT, RT = first CPU time - AT
 * CPU utilization = (total busy time / total schedule time) * 100
 */
#include "common.h"

void compute_metrics(Process p[], int n)
{
    for (int i = 0; i < n; i++) {
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt  = p[i].tat - p[i].bt;
        p[i].rt  = p[i].first_start - p[i].at;
    }
}

Stats compute_stats(const Process p[], int n, const Gantt *g)
{
    Stats s = {0.0, 0.0, 0.0, 0.0};
    if (n <= 0) return s;

    double total_wt = 0.0, total_tat = 0.0, total_rt = 0.0;
    for (int i = 0; i < n; i++) {
        total_wt  += p[i].wt;
        total_tat += p[i].tat;
        total_rt  += p[i].rt;
    }
    s.avg_wt  = total_wt / n;
    s.avg_tat = total_tat / n;
    s.avg_rt  = total_rt / n;

    if (g != 0 && g->count > 0) {
        int total_busy = 0;
        int total_time = g->e[g->count - 1].end;
        for (int i = 0; i < g->count; i++) {
            if (g->e[i].pid != -1) {
                total_busy += (g->e[i].end - g->e[i].start);
            }
        }
        if (total_time > 0) {
            s.cpu_util = ((double)total_busy / (double)total_time) * 100.0;
        } else {
            s.cpu_util = 0.0;
        }
    } else {
        s.cpu_util = 0.0;
    }

    return s;
}
