/* rr.c - Round Robin (Student 2)
 * Ready queue = circular FIFO of process indices.
 * Rule: processes arriving by the end of a slice (including exactly at
 * quantum expiry) are enqueued BEFORE the preempted process is re-queued.
 * No printing, no WT/TAT/RT, no globals. */
#include "common.h"

typedef struct {
    int item[MAX_PROC + 1];
    int head, tail, size;
} Queue;

static void q_init(Queue *q) { q->head = q->tail = q->size = 0; }

/* A process is never in the queue twice at once, so size <= n <= MAX_PROC
 * and this can never overflow. */
static void q_push(Queue *q, int v)
{
    q->item[q->tail] = v;
    q->tail = (q->tail + 1) % (MAX_PROC + 1);
    q->size++;
}

static int q_pop(Queue *q)
{
    int v = q->item[q->head];
    q->head = (q->head + 1) % (MAX_PROC + 1);
    q->size--;
    return v;
}

/* Fill order[] with process indices sorted by (at, pid). */
static void sort_by_arrival(const Process p[], int n, int order[])
{
    for (int i = 0; i < n; i++) order[i] = i;
    for (int i = 1; i < n; i++) {            /* insertion sort (stable) */
        int k = order[i], j = i - 1;
        while (j >= 0 && (p[order[j]].at > p[k].at ||
              (p[order[j]].at == p[k].at && p[order[j]].pid > p[k].pid))) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = k;
    }
}

/* Enqueue every not-yet-queued process with at <= t, in arrival order. */
static void admit(const Process p[], int n, const int order[],
                  int *next, int t, Queue *q)
{
    while (*next < n && p[order[*next]].at <= t)
        q_push(q, order[(*next)++]);
}

void round_robin(Process p[], int n, int tq, Gantt *g)
{
    int rem[MAX_PROC], order[MAX_PROC];
    Queue q;
    int t = 0, done = 0, next = 0;

    if (tq <= 0) tq = 1;                     /* guard: avoid infinite loop */
    for (int i = 0; i < n; i++) {
        rem[i] = p[i].bt;
        p[i].ct = 0;
        p[i].first_start = -1;
    }
    sort_by_arrival(p, n, order);
    q_init(&q);
    admit(p, n, order, &next, t, &q);

    while (done < n) {
        if (q.size == 0) {                   /* idle: jump to next arrival */
            int na = p[order[next]].at;
            gantt_add(g, -1, t, na);
            t = na;
            admit(p, n, order, &next, t, &q);
            continue;
        }

        int i = q_pop(&q);
        if (p[i].first_start == -1) p[i].first_start = t;

        int slice = rem[i] < tq ? rem[i] : tq;
        gantt_add(g, p[i].pid, t, t + slice);
        t += slice;
        rem[i] -= slice;

        admit(p, n, order, &next, t, &q);    /* new arrivals FIRST */
        if (rem[i] > 0) q_push(&q, i);       /* then preempted process */
        else { p[i].ct = t; done++; }
    }
}