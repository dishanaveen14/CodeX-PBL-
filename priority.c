/* priority.c - Priority scheduling (Student 2)
 * Lower pr number = higher priority.
 * Tie-break everywhere: earlier arrival, then smaller pid.
 * Only schedules: sets ct and first_start, records Gantt blocks.
 * No printing, no WT/TAT/RT, no globals. */
#include "common.h"

/* Returns true if process a beats process b by (pr, at, pid). */
static int better(const Process *a, const Process *b)
{
    if (a->pr != b->pr) return a->pr < b->pr;
    if (a->at != b->at) return a->at < b->at;
    return a->pid < b->pid;
}

/* Index of the best process that has arrived (at <= t) and still has
 * time left (rem > 0), or -1 if none is ready. */
static int pick_best(const Process p[], int n, int t, const int rem[])
{
    int best = -1;
    for (int i = 0; i < n; i++) {
        if (rem[i] <= 0 || p[i].at > t) continue;
        if (best == -1 || better(&p[i], &p[best])) best = i;
    }
    return best;
}

/* Smallest arrival time among unfinished processes (used to jump over
 * idle gaps). Caller guarantees at least one unfinished process. */
static int next_arrival(const Process p[], int n, const int rem[])
{
    int na = -1;
    for (int i = 0; i < n; i++)
        if (rem[i] > 0 && (na == -1 || p[i].at < na)) na = p[i].at;
    return na;
}

/* Reset outputs and fill the remaining-time array. */
static void init_run(Process p[], int n, int rem[])
{
    for (int i = 0; i < n; i++) {
        rem[i] = p[i].bt;
        p[i].ct = 0;
        p[i].first_start = -1;
    }
}

/* ---------- Non-preemptive: once started, a process runs to the end ---------- */
void priority_np(Process p[], int n, Gantt *g)
{
    int rem[MAX_PROC];
    int t = 0, done = 0;
    init_run(p, n, rem);

    while (done < n) {
        int i = pick_best(p, n, t, rem);
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

/* ---------- Preemptive: re-decide every time unit ---------- */
void priority_p(Process p[], int n, Gantt *g)
{
    int rem[MAX_PROC];
    int t = 0, done = 0;
    int cur = -1;            /* index running in the open block, -1 = none */
    int block_start = 0;     /* start time of the open block */
    init_run(p, n, rem);

    while (done < n) {
        int best = pick_best(p, n, t, rem);

        if (best == -1) {                    /* idle: close block, add idle gap */
            if (cur != -1) { gantt_add(g, p[cur].pid, block_start, t); cur = -1; }
            int na = next_arrival(p, n, rem);
            gantt_add(g, -1, t, na);
            t = na;
            continue;
        }

        /* Strict preemption: keep the running process unless the best
         * candidate has a strictly smaller pr number. */
        if (cur != -1 && rem[cur] > 0 && p[best].pr >= p[cur].pr)
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