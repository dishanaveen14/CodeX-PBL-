/* main.c - CLI menu, input, random generation, integration.
 * Owner: Team Lead
 *
 * Flow for every run:
 *   copy processes -> reset results -> run algorithm -> compute_metrics
 *   -> print_gantt -> print_process_table -> print_summary
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "common.h"

#define LINE_LEN 128

/* Algorithm ids (also the order used by "run all") */
enum { A_FCFS, A_SJF_NP, A_SJF_P, A_PRIO_NP, A_PRIO_P, A_RR, A_COUNT };

static const char *ALGO_NAME[A_COUNT] = {
    "FCFS", "SJF (Non-preemptive)", "SJF (Preemptive/SRTF)",
    "Priority (Non-preemptive)", "Priority (Preemptive)", "Round Robin"
};

/* ---------- safe input helpers ---------- */

/* Read an integer in [min,max]. Re-prompts on bad input.
 * Returns 1 on success, 0 on end-of-input (EOF). */
static int read_int(const char *prompt, int min, int max, int *out)
{
    char buf[LINE_LEN];

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (fgets(buf, sizeof buf, stdin) == NULL)
            return 0;
        if (strchr(buf, '\n') == NULL && !feof(stdin)) {   /* discard rest of long line */
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
        }

        char *end;
        errno = 0;
        long v = strtol(buf, &end, 10);
        while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')
            end++;

        if (end == buf || *end != '\0' || errno == ERANGE || v < min || v > max) {
            printf("  Invalid input. Please enter a whole number from %d to %d.\n", min, max);
            continue;
        }
        *out = (int)v;
        return 1;
    }
}

/* ---------- process input ---------- */

static int pid_taken(const Process p[], int upto, int pid)
{
    for (int i = 0; i < upto; i++)
        if (p[i].pid == pid)
            return 1;
    return 0;
}

/* Manual entry. Returns new n (0 if cancelled by EOF). */
static int enter_processes(Process p[])
{
    int n;
    if (!read_int("Number of processes (1-100): ", 1, MAX_PROC, &n))
        return 0;

    for (int i = 0; i < n; i++) {
        printf("--- Process %d of %d ---\n", i + 1, n);
        int pid, at, bt, pr;

        for (;;) {
            if (!read_int("  Process ID (1-9999): ", 1, 9999, &pid)) return 0;
            if (pid_taken(p, i, pid)) { printf("  That ID is already used. Choose another.\n"); continue; }
            break;
        }
        if (!read_int("  Arrival Time (0-1000): ", 0, 1000, &at)) return 0;
        if (!read_int("  Burst Time (1-100): ", 1, 100, &bt)) return 0;
        if (!read_int("  Priority (0-100, lower = higher priority): ", 0, 100, &pr)) return 0;

        p[i].pid = pid; p[i].at = at; p[i].bt = bt; p[i].pr = pr;
        p[i].ct = p[i].first_start = p[i].wt = p[i].tat = p[i].rt = 0;
    }
    return n;
}

/* Random generation. Returns new n (0 if cancelled). */
static int generate_random(Process p[])
{
    int n, max_at, max_bt, max_pr;
    if (!read_int("Number of processes (1-100): ", 1, MAX_PROC, &n)) return 0;
    if (!read_int("Maximum arrival time (0-50): ", 0, 50, &max_at)) return 0;
    if (!read_int("Maximum burst time (1-30): ", 1, 30, &max_bt)) return 0;
    if (!read_int("Maximum priority value (1-10): ", 1, 10, &max_pr)) return 0;

    for (int i = 0; i < n; i++) {
        p[i].pid = i + 1;
        p[i].at  = rand() % (max_at + 1);
        p[i].bt  = 1 + rand() % max_bt;
        p[i].pr  = 1 + rand() % max_pr;
        p[i].ct = p[i].first_start = p[i].wt = p[i].tat = p[i].rt = 0;
    }
    printf("Generated %d random processes.\n", n);
    return n;
}

static void display_processes(const Process p[], int n)
{
    if (n == 0) { printf("No processes yet. Use option 1 or 2 first.\n"); return; }
    printf("\n%-6s %-8s %-8s %-8s\n", "PID", "Arrival", "Burst", "Priority");
    printf("------ -------- -------- --------\n");
    for (int i = 0; i < n; i++)
        printf("P%-5d %-8d %-8d %-8d\n", p[i].pid, p[i].at, p[i].bt, p[i].pr);
    printf("\n");
}

/* ---------- running algorithms ---------- */

/* Copy originals into work[], reset results, run algorithm 'id', compute metrics. */
static void run_algo(int id, const Process orig[], int n, int tq, Process work[], Gantt *g)
{
    memcpy(work, orig, (size_t)n * sizeof(Process));
    for (int i = 0; i < n; i++)
        work[i].ct = work[i].first_start = work[i].wt = work[i].tat = work[i].rt = 0;
    memset(g, 0, sizeof *g);

    switch (id) {
    case A_FCFS:    fcfs(work, n, g);               break;
    case A_SJF_NP:  sjf_np(work, n, g);             break;
    case A_SJF_P:   sjf_p(work, n, g);              break;
    case A_PRIO_NP: priority_np(work, n, g);        break;
    case A_PRIO_P:  priority_p(work, n, g);         break;
    case A_RR:      round_robin(work, n, tq, g);    break;
    default: break;
    }
    compute_metrics(work, n);
}

static void run_and_show(int id, const Process orig[], int n)
{
    Process work[MAX_PROC];
    Gantt g;
    int tq = 0;

    if (n == 0) { printf("No processes yet. Use option 1 or 2 first.\n"); return; }
    if (id == A_RR && !read_int("Time quantum (1-100): ", 1, 100, &tq))
        return;

    run_algo(id, orig, n, tq, work, &g);

    printf("\n==================== %s", ALGO_NAME[id]);
    if (id == A_RR) printf(" (quantum = %d)", tq);
    printf(" ====================\n\nGantt Chart:\n");
    print_gantt(&g);
    printf("\n");
    print_process_table(work, n);
    printf("\n");
    print_summary(work, n, &g);
    printf("\n");
}

static void run_all(const Process orig[], int n)
{
    Process work[MAX_PROC];
    Gantt g;
    Stats stats[A_COUNT];
    const char *names[A_COUNT];
    int tq;

    if (n == 0) { printf("No processes yet. Use option 1 or 2 first.\n"); return; }
    if (!read_int("Time quantum for Round Robin (1-100): ", 1, 100, &tq))
        return;

    for (int a = 0; a < A_COUNT; a++) {
        run_algo(a, orig, n, tq, work, &g);
        stats[a] = compute_stats(work, n, &g);
        names[a] = ALGO_NAME[a];
    }
    printf("\n=========== Algorithm Comparison (RR quantum = %d) ===========\n", tq);
    print_comparison(names, stats, A_COUNT);
    printf("\n");
}

/* ---------- menu ---------- */

static void print_menu(int n)
{
    printf("\n========== CPU SCHEDULING SIMULATOR ==========\n");
    printf("Processes loaded: %d\n", n);
    printf(" 1. Enter processes manually\n");
    printf(" 2. Generate random processes\n");
    printf(" 3. Display current processes\n");
    printf(" 4. Run FCFS\n");
    printf(" 5. Run SJF (non-preemptive)\n");
    printf(" 6. Run SJF (preemptive / SRTF)\n");
    printf(" 7. Run Priority (non-preemptive)\n");
    printf(" 8. Run Priority (preemptive)\n");
    printf(" 9. Run Round Robin\n");
    printf("10. Run ALL algorithms and compare\n");
    printf(" 0. Exit\n");
}

int main(void)
{
    Process procs[MAX_PROC];
    int n = 0;
    int choice;

    srand((unsigned)time(NULL));

    for (;;) {
        print_menu(n);
        if (!read_int("Choose an option: ", 0, 10, &choice))
            break;                                   /* EOF: leave quietly */

        switch (choice) {
        case 0:  printf("Goodbye!\n"); return 0;
        case 1: { int m = enter_processes(procs); if (m > 0) n = m; break; }
        case 2: { int m = generate_random(procs); if (m > 0) n = m; display_processes(procs, n); break; }
        case 3:  display_processes(procs, n);                    break;
        case 4:  run_and_show(A_FCFS, procs, n);                 break;
        case 5:  run_and_show(A_SJF_NP, procs, n);               break;
        case 6:  run_and_show(A_SJF_P, procs, n);                break;
        case 7:  run_and_show(A_PRIO_NP, procs, n);              break;
        case 8:  run_and_show(A_PRIO_P, procs, n);               break;
        case 9:  run_and_show(A_RR, procs, n);                   break;
        case 10: run_all(procs, n);                              break;
        }
    }
    return 0;
}
