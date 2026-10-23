/* test_all.c - Integration test for Week 3 CPU Scheduler */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "common.h"

#define P(id,a,b,r) {id,a,b,r,0,0,0,0,0}

int main(void)
{
    printf("Starting comprehensive scheduler tests...\n");

    /* Test Case 1: Standard arrival sequence */
    Process set1[] = {
        P(1, 0, 4, 2),
        P(2, 1, 3, 1),
        P(3, 2, 1, 3)
    };
    int n1 = 3;

    /* FCFS */
    {
        Process p[3]; memcpy(p, set1, sizeof(set1));
        Gantt g = {.count = 0};
        fcfs(p, n1, &g);
        compute_metrics(p, n1);
        Stats s = compute_stats(p, n1, &g);
        assert(p[0].ct == 4 && p[0].wt == 0 && p[0].first_start == 0);
        assert(p[1].ct == 7 && p[1].wt == 3 && p[1].first_start == 4);
        assert(p[2].ct == 8 && p[2].wt == 5 && p[2].first_start == 7);
        assert(s.cpu_util > 99.9);
        printf("PASS: FCFS\n");
    }

    /* SJF Non-Preemptive */
    {
        Process p[3]; memcpy(p, set1, sizeof(set1));
        Gantt g = {.count = 0};
        sjf_np(p, n1, &g);
        compute_metrics(p, n1);
        /* At t=0, P1 starts and finishes at 4. At t=4, P2 (bt=3) and P3 (bt=1) ready. P3 picked next (finishes 5), then P2 (finishes 8) */
        assert(p[0].ct == 4 && p[0].first_start == 0);
        assert(p[2].ct == 5 && p[2].first_start == 4);
        assert(p[1].ct == 8 && p[1].first_start == 5);
        printf("PASS: SJF (NP)\n");
    }

    /* SJF Preemptive (SRTF) */
    {
        Process p[3]; memcpy(p, set1, sizeof(set1));
        Gantt g = {.count = 0};
        sjf_p(p, n1, &g);
        compute_metrics(p, n1);
        /* t=0..1: P1 (rem 3)
           t=1: P2 arrives (rem 3). Tie: P1 beats P2 because arrival is earlier.
           t=1..2: P1 (rem 2)
           t=2: P3 arrives (rem 1). P3 strictly shorter than P1 (rem 2). P3 preempts!
           t=2..3: P3 finishes at 3.
           t=3: P1 (rem 2) vs P2 (rem 3). P1 runs to completion at 5.
           t=5..8: P2 runs to completion at 8. */
        assert(p[2].ct == 3 && p[2].first_start == 2);
        assert(p[0].ct == 5 && p[0].first_start == 0);
        assert(p[1].ct == 8 && p[1].first_start == 5);
        printf("PASS: SJF (Preemptive / SRTF)\n");
    }

    /* Priority Non-Preemptive */
    {
        Process p[3]; memcpy(p, set1, sizeof(set1));
        Gantt g = {.count = 0};
        priority_np(p, n1, &g);
        compute_metrics(p, n1);
        /* t=0: P1 runs to 4. At t=4, P2 (pr=1) vs P3 (pr=3). P2 runs to 7. P3 runs to 8. */
        assert(p[0].ct == 4);
        assert(p[1].ct == 7);
        assert(p[2].ct == 8);
        printf("PASS: Priority (NP)\n");
    }

    /* Priority Preemptive */
    {
        Process p[3]; memcpy(p, set1, sizeof(set1));
        Gantt g = {.count = 0};
        priority_p(p, n1, &g);
        compute_metrics(p, n1);
        /* t=0: P1 starts (pr=2).
           t=1: P2 arrives with pr=1 (higher priority). Preempts P1!
           t=1..4: P2 runs to completion at 4.
           t=4: P1 (pr=2) vs P3 (pr=3). P1 runs 3 remaining units to 7.
           t=7..8: P3 runs to 8. */
        assert(p[1].ct == 4 && p[1].first_start == 1);
        assert(p[0].ct == 7 && p[0].first_start == 0);
        assert(p[2].ct == 8 && p[2].first_start == 7);
        printf("PASS: Priority (Preemptive)\n");
    }

    /* Round Robin */
    {
        Process p[3]; memcpy(p, set1, sizeof(set1));
        Gantt g = {.count = 0};
        round_robin(p, n1, 2, &g);
        compute_metrics(p, n1);
        /* tq=2
           t=0..2: P1 (slice 2, rem 2). At t=1 P2 arrives, at t=2 P3 arrives.
           Arrivals admitted: queue = [P2, P3, P1]
           t=2..4: P2 (slice 2, rem 1). Queue = [P3, P1, P2]
           t=4..5: P3 (slice 1, rem 0). Done! CT=5. Queue = [P1, P2]
           t=5..7: P1 (slice 2, rem 0). Done! CT=7. Queue = [P2]
           t=7..8: P2 (slice 1, rem 0). Done! CT=8. */
        assert(p[2].ct == 5);
        assert(p[0].ct == 7);
        assert(p[1].ct == 8);
        printf("PASS: Round Robin\n");
    }

    /* Test Case 2: Idle gaps */
    Process set2[] = {
        P(1, 2, 2, 1),
        P(2, 6, 2, 1)
    };
    {
        Process p[2]; memcpy(p, set2, sizeof(set2));
        Gantt g = {.count = 0};
        fcfs(p, 2, &g);
        compute_metrics(p, 2);
        Stats s = compute_stats(p, 2, &g);
        assert(g.count == 4); /* [idle 0-2], [P1 2-4], [idle 4-6], [P2 6-8] */
        assert(g.e[0].pid == -1 && g.e[0].start == 0 && g.e[0].end == 2);
        assert(g.e[1].pid == 1 && g.e[1].start == 2 && g.e[1].end == 4);
        assert(g.e[2].pid == -1 && g.e[2].start == 4 && g.e[2].end == 6);
        assert(g.e[3].pid == 2 && g.e[3].start == 6 && g.e[3].end == 8);
        /* Total time 8, busy time 4 -> cpu_util = 50% */
        assert(s.cpu_util >= 49.9 && s.cpu_util <= 50.1);
        printf("PASS: Idle Gap Handling & CPU Utilization (50%%)\n");
    }

    printf("ALL TESTS PASSED SUCCESSFULLY!\n");
    return 0;
}
