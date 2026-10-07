/*
 * tests/test_logger_flood.c
 *
 * Flood / throughput test for the logger.
 *
 * Usage:  ./test_logger_flood [N]   (default N=100000)
 *
 * What it does:
 *  1. mkdtemp() a temp dir; fork/execve ../logger/logger inside it
 *     with LOG_ROTATE_BYTES=1000000000 so rotation never fires.
 *  2. Retry mq_open(/sim_log) for up to 3 s until the queue exists.
 *  3. Send N blocking mq_send() messages each containing "flood seq=NNNNNN".
 *  4. Print messages/sec measured with CLOCK_MONOTONIC.
 *  5. Send LVL_SHUTDOWN, waitpid() the logger, measure total time.
 *  6. Check: exit-code 0, sim.log has exactly N lines, seq numbers
 *     are 1..N in order with no gaps/duplicates, stderr has
 *     "received=N written=N".
 *  7. Print PASS/FAIL per check + final summary.
 *  8. Print one CSV line to stdout: n,send_s,msgs_per_s,total_s
 *
 * Build (from project root):
 *   gcc -Wall -Wextra -O2 -pthread tests/test_logger_flood.c \
 *       -o build/tests/test_logger_flood -lrt
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

/* Pull in the shared message contract. */
#if defined(__has_include) && __has_include("../common/ipc.h")
#include "../common/ipc.h"
#else
typedef struct {
    uint64_t ts_ns;
    uint8_t  level;
    char     src[8];
    char     text[200];
} log_msg_t;
#define LOG_QUEUE_NAME "/sim_log"
#define LVL_INFO     0
#define LVL_SHUTDOWN 255
#endif

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

/* Ignore the return value of system() in a way GCC 15 accepts. */
static void rm_rf(const char *dir)
{
    char cmd[512];
    snprintf(cmd, sizeof cmd, "rm -rf %s", dir);
    if (system(cmd) != 0) { /* best-effort cleanup, ignore errors */ }
}

static double mono_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* Count newline characters = number of complete log lines in file. */
static long count_lines(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    long n = 0;
    int  c;
    while ((c = fgetc(f)) != EOF)
        if (c == '\n') n++;
    fclose(f);
    return n;
}

/*
 * Scan sim.log for "flood seq=NNNNNN" markers.
 * They must appear in order 1..N with no gaps and no duplicates.
 * Returns 0 on success, -1 on failure.
 */
static int check_sequences(const char *path, long N)
{
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen sim.log seq check"); return -1; }

    char line[512];
    long expected = 1;
    int  ok = 1;
    while (fgets(line, sizeof line, f)) {
        const char *p = strstr(line, "flood seq=");
        if (!p) continue;
        long seq = atol(p + 10); /* strlen("flood seq=") == 10 */
        if (seq != expected) {
            fprintf(stderr, "  SEQ gap/dup: expected=%ld got=%ld\n",
                    expected, seq);
            ok = 0;
            break;
        }
        expected++;
    }
    fclose(f);

    if (ok && expected - 1 != N) {
        fprintf(stderr, "  SEQ count: expected %ld saw %ld\n",
                N, expected - 1);
        ok = 0;
    }
    return ok ? 0 : -1;
}

/*
 * Read the stderr capture file and look for:
 *   [logger] shutdown: received=N written=N
 * Returns 0 if found with the right numbers, -1 otherwise.
 */
static int check_stderr_counts(const char *path, long N)
{
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen stderr log"); return -1; }

    char line[512];
    int  found = 0;
    while (fgets(line, sizeof line, f)) {
        unsigned long rcv = 0, wrt = 0;
        if (sscanf(line, "[logger] shutdown: received=%lu written=%lu",
                   &rcv, &wrt) == 2) {
            if ((long)rcv == N && (long)wrt == N) {
                found = 1;
            } else {
                fprintf(stderr,
                    "  stderr counts wrong: received=%lu written=%lu "
                    "(want %ld each)\n", rcv, wrt, N);
            }
            break;
        }
    }
    fclose(f);
    return found ? 0 : -1;
}

/* ------------------------------------------------------------------ */
/* PASS / FAIL bookkeeping                                             */
/* ------------------------------------------------------------------ */

static int g_failures = 0;

static void report(const char *name, int pass)
{
    printf("  %s  %s\n", pass ? "PASS" : "FAIL", name);
    if (!pass) g_failures++;
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    long N = 100000;
    if (argc > 1) {
        N = atol(argv[1]);
        if (N <= 0) { fprintf(stderr, "bad N\n"); return 1; }
    }

    /* 1. Create temp directory */
    char tmpdir[] = "/tmp/flood_XXXXXX";
    if (!mkdtemp(tmpdir)) { perror("mkdtemp"); return 1; }
    printf("[flood] tmpdir = %s\n", tmpdir);

    char logfile[256], stderrfile[256];
    snprintf(logfile,    sizeof logfile,    "%s/sim.log",           tmpdir);
    snprintf(stderrfile, sizeof stderrfile, "%s/logger_stderr.txt", tmpdir);

    /*
     * 2. Locate the logger binary.
     *    Read /proc/self/exe, strip two trailing path components
     *    (build/tests/<name> -> build/), then append "logger".
     *    Fall back to "./build/logger" if that file is missing.
     */
    char logger_path[512];
    {
        ssize_t len = readlink("/proc/self/exe",
                               logger_path, sizeof logger_path - 1);
        if (len > 0) {
            logger_path[len] = '\0';
            char *sl = strrchr(logger_path, '/');
            if (sl) *sl = '\0';            /* strip binary name */
            sl = strrchr(logger_path, '/');
            if (sl) {
                sl[1] = '\0';             /* keep trailing '/' after parent */
                strncat(logger_path, "logger",
                        sizeof logger_path - strlen(logger_path) - 1);
            }
        } else {
            snprintf(logger_path, sizeof logger_path, "../logger");
        }
    }
    if (access(logger_path, X_OK) != 0) {
        /* Try from project root */
        snprintf(logger_path, sizeof logger_path, "./build/logger");
        if (access(logger_path, X_OK) != 0) {
            fprintf(stderr,
                "Cannot find logger binary. Build with 'make' first.\n"
                "Last tried: %s\n", logger_path);
            return 1;
        }
    }

    /* 3. Build child environment: inherit everything + rotation override. */
    extern char **environ;
    int envc = 0;
    while (environ[envc]) envc++;
    char **new_env = malloc((size_t)(envc + 2) * sizeof(char *));
    if (!new_env) { perror("malloc"); return 1; }
    for (int i = 0; i < envc; i++) new_env[i] = environ[i];
    /* Never rotate during the flood so all messages end up in one file. */
    new_env[envc]     = "LOG_ROTATE_BYTES=1000000000";
    new_env[envc + 1] = NULL;

    /* 4. Fork / exec the logger. */
    /* Remove any stale queue so the logger always creates a fresh one. */
    mq_unlink(LOG_QUEUE_NAME);
    pid_t logger_pid = fork();
    if (logger_pid < 0) { perror("fork"); free(new_env); return 1; }

    if (logger_pid == 0) {
        /* Child: redirect stderr to capture file, chdir to temp dir, exec. */
        int fd = open(stderrfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) { perror("open stderr capture"); _exit(1); }
        dup2(fd, STDERR_FILENO);
        close(fd);

        if (chdir(tmpdir) != 0) { perror("chdir tmpdir"); _exit(1); }

        char *args[] = { logger_path, NULL };
        execve(logger_path, args, new_env);
        perror("execve logger");
        _exit(1);
    }

    free(new_env);

    /* 5. Wait for /sim_log queue to appear (up to 3 s). */
    mqd_t q = (mqd_t)-1;
    {
        double deadline = mono_now() + 3.0;
        while (mono_now() < deadline) {
            q = mq_open(LOG_QUEUE_NAME, O_WRONLY);
            if (q != (mqd_t)-1) break;
            if (errno != ENOENT) { perror("mq_open (unexpected)"); break; }
            struct timespec ns = { 0, 10000000L }; /* 10 ms */
            nanosleep(&ns, NULL);
        }
    }
    if (q == (mqd_t)-1) {
        fprintf(stderr, "Queue %s never appeared within 3 s\n", LOG_QUEUE_NAME);
        kill(logger_pid, SIGKILL);
        waitpid(logger_pid, NULL, 0);
        return 1;
    }

    printf("[flood] queue open, flooding %ld messages...\n", N);

    /* 6. Flood: N blocking mq_send() calls. */
    double t_send_start = mono_now();

    for (long i = 1; i <= N; i++) {
        log_msg_t m;
        memset(&m, 0, sizeof m);
        m.level = LVL_INFO;
        /* ts_ns == 0  ->  logger stamps on arrival */
        strncpy(m.src, "FLOOD", sizeof m.src - 1);
        /* text contains the sequence number for later verification */
        snprintf(m.text, sizeof m.text, "flood seq=%06ld", i);
        if (mq_send(q, (const char *)&m, sizeof m, 0) < 0) {
            perror("mq_send");
            fprintf(stderr, "  mq_send failed at i=%ld\n", i);
            break;
        }
    }

    double t_send_end = mono_now();
    double send_secs  = t_send_end - t_send_start;
    double msgs_per_s = (send_secs > 0.0) ? (double)N / send_secs : 0.0;

    printf("[flood] send done: %ld msgs, %.3f s, %.0f msg/s\n",
           N, send_secs, msgs_per_s);

    /* 7. Send SHUTDOWN and wait for logger to exit cleanly. */
    double t_total_start = mono_now();
    {
        log_msg_t sd;
        memset(&sd, 0, sizeof sd);
        sd.level = LVL_SHUTDOWN;
        strncpy(sd.src,  "TEST",           sizeof sd.src  - 1);
        strncpy(sd.text, "flood shutdown", sizeof sd.text - 1);
        if (mq_send(q, (const char *)&sd, sizeof sd, 0) < 0)
            perror("mq_send SHUTDOWN");
    }
    mq_close(q);

    int wstatus = 0;
    waitpid(logger_pid, &wstatus, 0);
    double total_secs = mono_now() - t_total_start;

    printf("[flood] logger exited %.3f s after SHUTDOWN was sent\n", total_secs);

    /* 8. Correctness checks. */
    printf("\n--- Checks ---\n");

    /* a) Exit code */
    int exit_code = WIFEXITED(wstatus) ? WEXITSTATUS(wstatus) : -1;
    report("Logger exit code == 0",
           WIFEXITED(wstatus) && exit_code == 0);

    /* b) Line count */
    long lines = count_lines(logfile);
    if (lines < 0) {
        fprintf(stderr, "  Cannot read %s\n", logfile);
        report("sim.log has exactly N lines", 0);
    } else {
        if (lines != N)
            fprintf(stderr, "  line count: expected %ld got %ld\n", N, lines);
        report("sim.log has exactly N lines", lines == N);
    }

    /* c) Sequence integrity */
    report("Sequence numbers 1..N in order, no gaps/duplicates",
           check_sequences(logfile, N) == 0);

    /* d) stderr counters match */
    report("Logger stderr says received=N written=N",
           check_stderr_counts(stderrfile, N) == 0);

    /* 9. Summary */
    printf("\n--- Summary ---\n");
    if (g_failures == 0)
        printf("ALL PASS\n");
    else
        printf("%d FAIL(s)\n", g_failures);

    /* 10. CSV benchmark line (stdout, for benchmarking scripts) */
    printf("\n# CSV: n,send_seconds,msgs_per_sec,total_seconds\n");
    printf("%ld,%.6f,%.2f,%.6f\n", N, send_secs, msgs_per_s, total_secs);

    /* Cleanup temp dir (best-effort). */
    rm_rf(tmpdir);

    return (g_failures > 0) ? 1 : 0;
}
