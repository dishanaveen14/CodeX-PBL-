/*
 * tests/test_logger_bad.c
 *
 * Edge-case / robustness tests for the logger process.
 *
 * Each test case starts a fresh logger in its own mkdtemp() directory
 * and exercises one failure/edge scenario.
 *
 * Cases:
 *  a) Sender starts before logger: mq_open() returns -1 / ENOENT.
 *  b) Oversized message: mq_send returns -1 / EMSGSIZE; logger stays alive.
 *  c) Too-small message (10 bytes): logger ignores it, next valid msg logs.
 *  d) Unterminated strings (no NUL in src/text): logger doesn't crash.
 *  e) SHUTDOWN while messages still queued: all 500 lines appear in sim.log.
 *  f) SIGTERM: logger exits cleanly (code 0) and removes the queue.
 *  g) SIGKILL while sender is using O_NONBLOCK handle: sender doesn't hang.
 *
 * Build (from project root):
 *   gcc -Wall -Wextra -O2 -pthread tests/test_logger_bad.c \
 *       -o build/tests/test_logger_bad -lrt
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
#define LVL_WARN     1
#define LVL_SHUTDOWN 255
#endif

/* ------------------------------------------------------------------ */
/* Shared utilities                                                    */
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

static void ms_sleep(int ms)
{
    struct timespec ts = { ms / 1000, (long)(ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

/* Count newlines in file = number of log lines written. */
static long count_lines(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    long n = 0; int c;
    while ((c = fgetc(f)) != EOF) if (c == '\n') n++;
    fclose(f);
    return n;
}

/* Returns 1 if the file contains the substring needle, 0 otherwise. */
static int file_contains(const char *path, const char *needle)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    char line[512]; int found = 0;
    while (!found && fgets(line, sizeof line, f))
        if (strstr(line, needle)) found = 1;
    fclose(f);
    return found;
}

/*
 * Returns 1 if the process pid is alive (kill(pid,0) == 0),
 * 0 if it has exited or errno != ESRCH.
 */
static int proc_alive(pid_t pid)
{
    return kill(pid, 0) == 0;
}

/* ------------------------------------------------------------------ */
/* Logger lifecycle helpers                                            */
/* ------------------------------------------------------------------ */

/*
 * Find the logger binary: /proc/self/exe -> strip two components ->
 * append "logger".  Fall back to "./build/logger".
 */
static void find_logger(char *buf, size_t bufsz)
{
    ssize_t len = readlink("/proc/self/exe", buf, bufsz - 1);
    if (len > 0) {
        buf[len] = '\0';
        char *sl = strrchr(buf, '/'); if (sl) *sl = '\0';
        sl = strrchr(buf, '/');
        if (sl) {
            sl[1] = '\0';
            strncat(buf, "logger", bufsz - strlen(buf) - 1);
            return;
        }
    }
    snprintf(buf, bufsz, "./build/logger");
}

/*
 * Spawn the logger into tmpdir with the given extra environment variable
 * (may be NULL).  Stderr is captured in tmpdir/logger_stderr.txt.
 * Returns the child PID, or -1 on error.
 */
static pid_t start_logger(const char *tmpdir, const char *extra_env)
{
    char logger_path[512];
    find_logger(logger_path, sizeof logger_path);
    if (access(logger_path, X_OK) != 0) {
        /* try one more fallback */
        snprintf(logger_path, sizeof logger_path, "./build/logger");
        if (access(logger_path, X_OK) != 0) {
            fprintf(stderr, "Cannot find logger binary: %s\n", logger_path);
            return -1;
        }
    }

    /* Build environment */
    extern char **environ;
    int envc = 0; while (environ[envc]) envc++;
    char **new_env = malloc((size_t)(envc + 3) * sizeof(char *));
    if (!new_env) return -1;
    for (int i = 0; i < envc; i++) new_env[i] = environ[i];
    int ei = envc;
    /* Always disable rotation so tests see a single sim.log file. */
    new_env[ei++] = "LOG_ROTATE_BYTES=1000000000";
    if (extra_env) new_env[ei++] = (char *)extra_env;
    new_env[ei] = NULL;

    char stderrfile[512];
    snprintf(stderrfile, sizeof stderrfile, "%s/logger_stderr.txt", tmpdir);

    /* Remove any stale queue so logger always creates a fresh one. */
    mq_unlink(LOG_QUEUE_NAME);
    pid_t pid = fork();
    if (pid < 0) { free(new_env); perror("fork"); return -1; }
    if (pid == 0) {
        int fd = open(stderrfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd >= 0) { dup2(fd, STDERR_FILENO); close(fd); }
        if (chdir(tmpdir) != 0) { perror("chdir"); _exit(1); }
        char *args[] = { logger_path, NULL };
        execve(logger_path, args, new_env);
        perror("execve"); _exit(1);
    }
    free(new_env);
    return pid;
}

/*
 * Wait for LOG_QUEUE_NAME to appear in the kernel (up to max_ms ms).
 * Returns an O_WRONLY mqd_t on success, (mqd_t)-1 on timeout.
 */
static mqd_t wait_for_queue(int max_ms)
{
    mqd_t q = (mqd_t)-1;
    double deadline = mono_now() + max_ms * 0.001;
    while (mono_now() < deadline) {
        q = mq_open(LOG_QUEUE_NAME, O_WRONLY);
        if (q != (mqd_t)-1) return q;
        if (errno != ENOENT) return (mqd_t)-1;
        ms_sleep(10);
    }
    return (mqd_t)-1;
}

/* Build a valid log message (blocking mq_send). */
static int send_msg(mqd_t q, uint8_t lvl, const char *src, const char *text)
{
    log_msg_t m;
    memset(&m, 0, sizeof m);
    m.level = lvl;
    strncpy(m.src,  src,  sizeof m.src  - 1);
    strncpy(m.text, text, sizeof m.text - 1);
    return (int)mq_send(q, (const char *)&m, sizeof m, 0);
}

/* Send a LVL_SHUTDOWN message and close the queue handle. */
static void send_shutdown(mqd_t q)
{
    log_msg_t sd;
    memset(&sd, 0, sizeof sd);
    sd.level = LVL_SHUTDOWN;
    strncpy(sd.src, "TEST", sizeof sd.src - 1);
    mq_send(q, (const char *)&sd, sizeof sd, 0);
    mq_close(q);
}

/* Wait for pid to exit (up to max_ms ms). Returns wait status. */
static int wait_timeout(pid_t pid, int max_ms, int *ws_out)
{
    double deadline = mono_now() + max_ms * 0.001;
    while (mono_now() < deadline) {
        int ws = 0;
        pid_t r = waitpid(pid, &ws, WNOHANG);
        if (r == pid) { if (ws_out) *ws_out = ws; return 0; }
        ms_sleep(50);
    }
    return -1; /* timed out */
}

/* ------------------------------------------------------------------ */
/* PASS / FAIL bookkeeping                                             */
/* ------------------------------------------------------------------ */

static int g_failures = 0;
static int g_total    = 0;

static void report(const char *name, int pass)
{
    g_total++;
    printf("  %s  %s\n", pass ? "PASS" : "FAIL", name);
    fflush(stdout);
    if (!pass) g_failures++;
}

/* ------------------------------------------------------------------ */
/* Individual test cases                                               */
/* ------------------------------------------------------------------ */

/*
 * a) Sender starts before logger.
 *    mq_open(O_WRONLY) on /sim_log must return -1 with ENOENT.
 */
static void test_a_no_logger(void)
{
    printf("\n[a] Sender before logger\n");
    /* Make sure no stale queue exists. */
    mq_unlink(LOG_QUEUE_NAME);

    errno = 0;
    mqd_t q = mq_open(LOG_QUEUE_NAME, O_WRONLY);
    int got_err = errno;
    if (q != (mqd_t)-1) {
        mq_close(q);
        report("a: mq_open returns -1 with ENOENT (no logger)", 0);
        return;
    }
    report("a: mq_open returns -1 with ENOENT (no logger)", got_err == ENOENT);
}

/*
 * b) Oversized message.
 *    mq_send with size > msgsize must return -1 / EMSGSIZE.
 *    After that, a normal message is still accepted by the logger.
 */
static void test_b_oversized(void)
{
    printf("\n[b] Oversized message\n");

    char tmpdir[] = "/tmp/bad_b_XXXXXX";
    if (!mkdtemp(tmpdir)) { perror("mkdtemp b"); return; }

    pid_t pid = start_logger(tmpdir, NULL);
    if (pid < 0) goto cleanup_b;

    mqd_t q = wait_for_queue(3000);
    if (q == (mqd_t)-1) {
        fprintf(stderr, "  b: queue never appeared\n");
        kill(pid, SIGKILL); waitpid(pid, NULL, 0);
        goto cleanup_b;
    }

    /* Send a buffer that is 1 byte larger than a valid log_msg_t. */
    char big[sizeof(log_msg_t) + 1];
    memset(big, 0, sizeof big);
    errno = 0;
    ssize_t r = mq_send(q, big, sizeof big, 0);
    int got_err = errno;
    report("b: mq_send oversized -> -1 / EMSGSIZE",
           r < 0 && got_err == EMSGSIZE);

    /* Logger must still be alive and log a subsequent valid message. */
    ms_sleep(200);
    report("b: logger still alive after oversized send", proc_alive(pid));

    send_msg(q, LVL_INFO, "TEST", "after oversized");
    send_shutdown(q);

    int ws = 0;
    wait_timeout(pid, 5000, &ws);

    char logfile[256];
    snprintf(logfile, sizeof logfile, "%s/sim.log", tmpdir);
    report("b: valid message after oversized is logged",
           file_contains(logfile, "after oversized"));

cleanup_b:;
    rm_rf(tmpdir);
}

/*
 * c) Too-small message (10 bytes).
 *    Logger must silently ignore it (wrong size check in logger_main.c line
 *    "if ((size_t)r != sizeof m) continue;").
 *    A valid message sent afterwards must be logged.
 */
static void test_c_small_message(void)
{
    printf("\n[c] Too-small message (10 bytes)\n");

    char tmpdir[] = "/tmp/bad_c_XXXXXX";
    if (!mkdtemp(tmpdir)) { perror("mkdtemp c"); return; }

    pid_t pid = start_logger(tmpdir, NULL);
    if (pid < 0) goto cleanup_c;

    mqd_t q = wait_for_queue(3000);
    if (q == (mqd_t)-1) {
        fprintf(stderr, "  c: queue never appeared\n");
        kill(pid, SIGKILL); waitpid(pid, NULL, 0);
        goto cleanup_c;
    }

    /*
     * We need to open a separate write handle whose msgsize the kernel
     * will accept for a 10-byte payload.  The queue was created by the
     * logger with msgsize = sizeof(log_msg_t).  A send of 10 bytes is
     * legal as long as 10 <= msgsize; mq_send checks size <= mq_msgsize.
     * So we can just send 10 bytes on the existing handle -- the kernel
     * accepts it, but the logger's size check will reject it.
     */
    char small[10];
    memset(small, 0xAB, sizeof small);
    ssize_t r = mq_send(q, small, sizeof small, 0);
    if (r < 0) {
        /* mq_send itself failed -- unexpected, but possible on some kernels.
         * That is still a "logger not crashed" situation; just note it. */
        fprintf(stderr, "  c: mq_send 10-byte returned %d errno %d (%s)\n",
                (int)r, errno, strerror(errno));
    }

    ms_sleep(200);
    report("c: logger still alive after undersized send", proc_alive(pid));

    send_msg(q, LVL_INFO, "TEST", "after small");
    send_shutdown(q);

    int ws = 0;
    wait_timeout(pid, 5000, &ws);

    char logfile[256];
    snprintf(logfile, sizeof logfile, "%s/sim.log", tmpdir);
    report("c: valid message after undersized is logged",
           file_contains(logfile, "after small"));

cleanup_c:;
    rm_rf(tmpdir);
}

/*
 * d) Unterminated strings.
 *    src[] and text[] are completely filled with 'A' (no NUL byte).
 *    The logger's defensive NUL-termination in logger_main.c
 *    ("m.src[sizeof m.src - 1] = '\0'; m.text[sizeof m.text - 1] = '\0';")
 *    must prevent a crash and must write a line.
 */
static void test_d_unterminated(void)
{
    printf("\n[d] Unterminated strings in src and text\n");

    char tmpdir[] = "/tmp/bad_d_XXXXXX";
    if (!mkdtemp(tmpdir)) { perror("mkdtemp d"); return; }

    pid_t pid = start_logger(tmpdir, NULL);
    if (pid < 0) goto cleanup_d;

    mqd_t q = wait_for_queue(3000);
    if (q == (mqd_t)-1) {
        fprintf(stderr, "  d: queue never appeared\n");
        kill(pid, SIGKILL); waitpid(pid, NULL, 0);
        goto cleanup_d;
    }

    log_msg_t m;
    memset(&m, 0, sizeof m);
    m.level = LVL_INFO;
    memset(m.src,  'A', sizeof m.src);   /* no NUL */
    memset(m.text, 'A', sizeof m.text);  /* no NUL */
    if (mq_send(q, (const char *)&m, sizeof m, 0) < 0)
        perror("  d: mq_send");

    ms_sleep(300);
    report("d: logger still alive after unterminated-string message",
           proc_alive(pid));

    send_shutdown(q);

    int ws = 0;
    wait_timeout(pid, 5000, &ws);

    char logfile[256];
    snprintf(logfile, sizeof logfile, "%s/sim.log", tmpdir);
    long lines = count_lines(logfile);
    report("d: at least one line written (logger didn't crash silently)",
           lines >= 1);

cleanup_d:;
    rm_rf(tmpdir);
}

/*
 * e) SHUTDOWN while messages still queued.
 *    Send 500 messages, then immediately SHUTDOWN.
 *    All 500 must appear in sim.log (logger drains the ring before exit).
 */
static void test_e_shutdown_with_queue(void)
{
    printf("\n[e] SHUTDOWN while messages queued\n");

    char tmpdir[] = "/tmp/bad_e_XXXXXX";
    if (!mkdtemp(tmpdir)) { perror("mkdtemp e"); return; }

    pid_t pid = start_logger(tmpdir, NULL);
    if (pid < 0) goto cleanup_e;

    mqd_t q = wait_for_queue(3000);
    if (q == (mqd_t)-1) {
        fprintf(stderr, "  e: queue never appeared\n");
        kill(pid, SIGKILL); waitpid(pid, NULL, 0);
        goto cleanup_e;
    }

    const int NMSG = 500;
    for (int i = 1; i <= NMSG; i++) {
        char txt[64];
        snprintf(txt, sizeof txt, "etest seq=%04d", i);
        if (send_msg(q, LVL_INFO, "TEST", txt) < 0) {
            perror("  e: mq_send");
            break;
        }
    }
    /* Immediately send SHUTDOWN while those messages may still be buffered. */
    send_shutdown(q);

    int ws = 0;
    if (wait_timeout(pid, 10000, &ws) < 0) {
        fprintf(stderr, "  e: logger did not exit within 10 s\n");
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        report("e: all 500 messages in sim.log after SHUTDOWN", 0);
        goto cleanup_e;
    }

    char logfile[256];
    snprintf(logfile, sizeof logfile, "%s/sim.log", tmpdir);
    long lines = count_lines(logfile);
    if (lines != NMSG)
        fprintf(stderr, "  e: expected %d lines, got %ld\n", NMSG, lines);
    report("e: all 500 messages in sim.log after SHUTDOWN", lines == NMSG);

cleanup_e:;
    rm_rf(tmpdir);
}

/*
 * f) SIGTERM.
 *    Send 100 messages, kill with SIGTERM.
 *    Logger must exit with code 0 within 3 s and /sim_log must be removed.
 */
static void test_f_sigterm(void)
{
    printf("\n[f] SIGTERM\n");

    char tmpdir[] = "/tmp/bad_f_XXXXXX";
    if (!mkdtemp(tmpdir)) { perror("mkdtemp f"); return; }

    pid_t pid = start_logger(tmpdir, NULL);
    if (pid < 0) goto cleanup_f;

    mqd_t q = wait_for_queue(3000);
    if (q == (mqd_t)-1) {
        fprintf(stderr, "  f: queue never appeared\n");
        kill(pid, SIGKILL); waitpid(pid, NULL, 0);
        goto cleanup_f;
    }

    for (int i = 0; i < 100; i++)
        send_msg(q, LVL_INFO, "TEST", "sigterm test msg");

    mq_close(q);        /* close our handle before signalling */
    ms_sleep(100);      /* let some messages drain */
    kill(pid, SIGTERM);

    int ws = 0;
    int timed_out = wait_timeout(pid, 3000, &ws);
    if (timed_out < 0) {
        fprintf(stderr, "  f: logger did not exit within 3 s after SIGTERM\n");
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
    }

    report("f: logger exits with code 0 after SIGTERM",
           !timed_out && WIFEXITED(ws) && WEXITSTATUS(ws) == 0);

    /* Queue must have been unlinked by the logger. */
    errno = 0;
    mqd_t qcheck = mq_open(LOG_QUEUE_NAME, O_WRONLY);
    int chk_err  = errno;
    if (qcheck != (mqd_t)-1) mq_close(qcheck);
    report("f: /sim_log removed after SIGTERM shutdown",
           qcheck == (mqd_t)-1 && chk_err == ENOENT);

cleanup_f:;
    rm_rf(tmpdir);
}

/*
 * g) SIGKILL while sender uses O_NONBLOCK.
 *    After the logger is killed, mq_send(O_NONBLOCK) must return -1
 *    (EAGAIN or EBADF or similar) immediately -- the sender must not hang.
 */
static void test_g_sigkill_nonblock(void)
{
    printf("\n[g] SIGKILL while sender uses O_NONBLOCK\n");

    char tmpdir[] = "/tmp/bad_g_XXXXXX";
    if (!mkdtemp(tmpdir)) { perror("mkdtemp g"); return; }

    pid_t pid = start_logger(tmpdir, NULL);
    if (pid < 0) goto cleanup_g;

    mqd_t q = wait_for_queue(3000);
    if (q == (mqd_t)-1) {
        fprintf(stderr, "  g: queue never appeared\n");
        kill(pid, SIGKILL); waitpid(pid, NULL, 0);
        goto cleanup_g;
    }

    /* Re-open the queue with O_NONBLOCK for the sender side. */
    mq_close(q);
    q = mq_open(LOG_QUEUE_NAME, O_WRONLY | O_NONBLOCK);
    if (q == (mqd_t)-1) {
        perror("  g: mq_open O_NONBLOCK");
        kill(pid, SIGKILL); waitpid(pid, NULL, 0);
        goto cleanup_g;
    }

    /* Fill the queue to capacity so the next send would block if blocking. */
    {
        log_msg_t m; memset(&m, 0, sizeof m);
        m.level = LVL_INFO;
        strncpy(m.src,  "TEST", sizeof m.src  - 1);
        strncpy(m.text, "fill", sizeof m.text - 1);
        /* Keep sending until EAGAIN (queue full) or error. */
        int sent = 0;
        while (mq_send(q, (const char *)&m, sizeof m, 0) == 0) sent++;
        (void)sent;
        /* errno is now EAGAIN -- queue is full */
    }

    /* Kill the logger with SIGKILL. */
    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);

    /*
     * Now try a non-blocking send.  The kernel queue still exists until
     * the logger calls mq_unlink() -- but the logger is dead, so the queue
     * remains.  The send will return -1/EAGAIN (queue full) or another
     * non-blocking error.  The critical property: it must NOT block.
     *
     * We verify this by checking the return value is -1 (not 0) and
     * it returns quickly (not a hang).
     */
    double t0 = mono_now();
    log_msg_t probe; memset(&probe, 0, sizeof probe);
    probe.level = LVL_INFO;
    strncpy(probe.src,  "TEST",  sizeof probe.src  - 1);
    strncpy(probe.text, "probe", sizeof probe.text - 1);
    ssize_t r = mq_send(q, (const char *)&probe, sizeof probe, 0);
    double elapsed = mono_now() - t0;

    /*
     * mq_send must return -1 (queue full / process dead) in well under 1 s.
     * (On Linux, the queue still exists until explicitly unlinked, so we
     * expect EAGAIN from O_NONBLOCK on a full queue.)
     */
    report("g: O_NONBLOCK mq_send returns immediately (no hang)",
           elapsed < 1.0 && r < 0);

    mq_close(q);
    /* Clean up the orphaned queue (logger is dead so it can't). */
    mq_unlink(LOG_QUEUE_NAME);

cleanup_g:;
    rm_rf(tmpdir);
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("=== test_logger_bad ===\n");

    test_a_no_logger();
    test_b_oversized();
    test_c_small_message();
    test_d_unterminated();
    test_e_shutdown_with_queue();
    test_f_sigterm();
    test_g_sigkill_nonblock();

    printf("\n=== Summary: %d/%d passed ===\n",
           g_total - g_failures, g_total);

    return (g_failures > 0) ? 1 : 0;
}
