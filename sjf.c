/* sjf.c - Shortest Job First scheduling (Student 1)
 * Non-preemptive (SJF) and Preemptive (SRTF - Shortest Remaining Time First).
 * Tie-break everywhere: earlier arrival time, then smaller pid.
 * Sets ct and first_start, records Gantt blocks.
 */
#include "common.h"

/* Returns true if process a beats process b by (burst/remaining, arrival, pid). */
static int better_np(const Process *a, const Process *b)
{
    if (a->bt != b->bt) return a->bt < b->bt;
    if (a->at != b->at) return a->at < b->at;
    return a->pid < b->pid;
}

static int better_p(const Process *a, int rem_a, const Process *b, int rem_b)
{
    if (rem_a != rem_b) return rem_a < rem_b;
    if (a->at != b->at) return a->at < b->at;
    return a->pid < b->pid;
}

static int pick_best_np(const Process p[], int n, int t, const int rem[])
{
    int best = -1;
    for (int i = 0; i < n; i++) {
        if (rem[i] <= 0 || p[i].at > t) continue;
        if (best == -1 || better_np(&p[i], &p[best])) best = i;
    }
    return best;
}

static int pick_best_p(const Process p[], int n, int t, const int rem[])
{
    int best = -1;
    for (int i = 0; i < n; i++) {
        if (rem[i] <= 0 || p[i].at > t) continue;
        if (best == -1 || better_p(&p[i], rem[i], &p[best], rem[best])) best = i;
    }
    return best;
}

static int next_arrival(const Process p[], int n, const int rem[])
{
    int na = -1;
    for (int i = 0; i < n; i++)
        if (rem[i] > 0 && (na == -1 || p[i].at < na)) na = p[i].at;
    return na;
}

static void init_run(Process p[], int n, int rem[])
{
    for (int i = 0; i < n; i++) {
        rem[i] = p[i].bt;
        p[i].ct = 0;
        p[i].first_start = -1;
    }
}

/* ---------- Non-preemptive SJF ---------- */
void sjf_np(Process p[], int n, Gantt *g)
{
    if (n <= 0) return;
    int rem[MAX_PROC];
    int t = 0, done = 0;
    init_run(p, n, rem);

    while (done < n) {
        int i = pick_best_np(p, n, t, rem);
        if (i == -1) {                       /* CPU idle: jump to next arrival */
            int na = next_arrival(p, n, rem);
            gantt_add(g, -1, t, na);
            t = na;
            continue;
        }
        p[i].first_start = t;
        gantt_add(g, p[i].pid, t, t + p[i].bt);
        t += p[i].bt;
        p[i].ct = t;
        rem[i] = 0;
        done++;
    }
}

/* ---------- Preemptive SJF (SRTF) ---------- */
void sjf_p(Process p[], int n, Gantt *g)
{
    if (n <= 0) return;
    int rem[MAX_PROC];
    int t = 0, done = 0;
    int cur = -1;            /* index running in open block, -1 = none */
    int block_start = 0;     /* start time of the open block */
    init_run(p, n, rem);

    while (done < n) {
        int best = pick_best_p(p, n, t, rem);

        if (best == -1) {                    /* idle: close block, add idle gap */
            if (cur != -1) { gantt_add(g, p[cur].pid, block_start, t); cur = -1; }
            int na = next_arrival(p, n, rem);
            gantt_add(g, -1, t, na);
            t = na;
            continue;
        }

        /* Strict preemption: keep running process if candidate doesn't strictly have smaller remaining time */
        if (cur != -1 && rem[cur] > 0 && rem[best] >= rem[cur])
            best = cur;

        if (best != cur) {                   /* context switch: close old block */
            if (cur != -1) gantt_add(g, p[cur].pid, block_start, t);
            cur = best;
            block_start = t;
        }

        if (p[cur].first_start == -1) p[cur].first_start = t;

        rem[cur]--;                          /* run one time unit */
        t++;
        if (rem[cur] == 0) {
            p[cur].ct = t;
            done++;
            gantt_add(g, p[cur].pid, block_start, t);
            cur = -1;
        }
    }
}
