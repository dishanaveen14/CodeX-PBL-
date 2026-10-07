/*
 * logger_main.c : Logging process (Student 3)
 *
 * Receiver thread : mq_timedreceive(/sim_log) -> ring buffer
 * Writer thread   : ring buffer -> sim.log (batched, with rotation)
 * Shutdown        : SHUTDOWN message (level 255) or SIGTERM/SIGINT
 *
 * Build: gcc -Wall -Wextra -O2 -pthread logger_main.c -o logger -lrt
 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/* Use the team's frozen contract if it exists, otherwise a local copy. */
#if defined(__has_include) && __has_include("../common/ipc.h")
#include "../common/ipc.h"
#else
typedef struct {
    uint64_t ts_ns;   /* CLOCK_REALTIME nanoseconds, 0 = stamp on arrival */
    uint8_t  level;   /* 0 INFO, 1 WARN, 2 ERROR, 255 SHUTDOWN */
    char     src[8];  /* "UI", "CORE", ... */
    char     text[200];
} log_msg_t;
#define LOG_QUEUE_NAME "/sim_log"
#endif

#define LVL_SHUTDOWN 255
#define RING_CAP     4096          /* entries in the ring buffer */
#define BATCH_MAX    256           /* entries flushed per write */
#define LOG_FILE     "sim.log"
/* ROTATE_BYTES is now a runtime variable (see g_rotate_bytes below).
 * Default is 1 MB. Override with LOG_ROTATE_BYTES env var at startup.
 * This lets tests set a huge threshold so rotation never fires mid-test. */
#define ROTATE_BYTES_DEFAULT (1024 * 1024)
#define ROTATE_KEEP  3             /* sim.log.1 .. sim.log.3 */

/* ---------- ring buffer (mutex + 2 condvars) ---------- */
static log_msg_t       ring[RING_CAP];
static size_t          head, tail, count;
static int             done;                 /* receiver finished */
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  not_full  = PTHREAD_COND_INITIALIZER;

static volatile sig_atomic_t g_stop;
static unsigned long g_received, g_written;
static long g_rotate_bytes;   /* set from LOG_ROTATE_BYTES env var in main() */

static void on_signal(int s) { (void)s; g_stop = 1; }

static void ring_push(const log_msg_t *m)
{
    pthread_mutex_lock(&mu);
    while (count == RING_CAP)               /* block: no message is lost */
        pthread_cond_wait(&not_full, &mu);
    ring[head] = *m;
    head = (head + 1) % RING_CAP;
    count++;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mu);
}

/* ---------- formatting & rotation ---------- */
static const char *level_str(uint8_t l)
{
    switch (l) {
    case 0:  return "INFO ";
    case 1:  return "WARN ";
    case 2:  return "ERROR";
    default: return "?????";
    }
}

static void format_line(FILE *f, const log_msg_t *m)
{
    uint64_t ns = m->ts_ns;
    if (ns == 0) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ns = (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
    }
    time_t sec = (time_t)(ns / 1000000000ull);
    struct tm tmv;
    localtime_r(&sec, &tmv);
    char buf[32];
    strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", &tmv);
    fprintf(f, "%s.%06u [%s] [%-7s] %s\n", buf,
            (unsigned)((ns % 1000000000ull) / 1000), level_str(m->level),
            m->src, m->text);
}

static FILE *open_log(void)
{
    FILE *f = fopen(LOG_FILE, "a");
    if (!f) perror("fopen sim.log");
    return f;
}

static FILE *rotate(FILE *f)
{
    char a[64], b[64];
    fclose(f);
    for (int i = ROTATE_KEEP - 1; i >= 1; i--) {
        snprintf(a, sizeof a, LOG_FILE ".%d", i);
        snprintf(b, sizeof b, LOG_FILE ".%d", i + 1);
        rename(a, b);
    }
    snprintf(b, sizeof b, LOG_FILE ".1");
    rename(LOG_FILE, b);
    return open_log();
}

/* ---------- writer thread ---------- */
static void *writer_thread(void *arg)
{
    (void)arg;
    FILE *f = open_log();
    if (!f) return NULL;
    static log_msg_t batch[BATCH_MAX];

    for (;;) {
        pthread_mutex_lock(&mu);
        while (count == 0 && !done)
            pthread_cond_wait(&not_empty, &mu);
        if (count == 0 && done) {           /* fully drained: exit */
            pthread_mutex_unlock(&mu);
            break;
        }
        size_t n = 0;
        while (count > 0 && n < BATCH_MAX) {
            batch[n++] = ring[tail];
            tail = (tail + 1) % RING_CAP;
            count--;
        }
        pthread_cond_broadcast(&not_full);
        pthread_mutex_unlock(&mu);

        for (size_t i = 0; i < n; i++)
            format_line(f, &batch[i]);
        fflush(f);                          /* one flush per batch */
        g_written += n;

        if (ftell(f) >= g_rotate_bytes) {
            f = rotate(f);
            if (!f) return NULL;
        }
    }
    fclose(f);
    return NULL;
}

/* ---------- main (acts as receiver thread) ---------- */
int main(void)
{
    /* Read rotation threshold from environment; fall back to 1 MB default.
     * Tests set LOG_ROTATE_BYTES=1000000000 so rotation never fires mid-run,
     * allowing a line-count check on a single sim.log without extra rotated files. */
    {
        const char *env = getenv("LOG_ROTATE_BYTES");
        g_rotate_bytes = (env && atol(env) > 0) ? atol(env) : ROTATE_BYTES_DEFAULT;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;              /* no SA_RESTART: wake mq calls */
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    struct mq_attr attr;
    memset(&attr, 0, sizeof attr);
    attr.mq_msgsize = sizeof(log_msg_t);
    attr.mq_maxmsg  = 256;                  /* may exceed msg_max (default 10) */

    mq_unlink(LOG_QUEUE_NAME);              /* remove stale queue */
    mqd_t q = mq_open(LOG_QUEUE_NAME, O_CREAT | O_RDONLY, 0660, &attr);
    if (q == (mqd_t)-1 && (errno == EINVAL || errno == EPERM)) {
        attr.mq_maxmsg = 10;                /* fall back to system default */
        q = mq_open(LOG_QUEUE_NAME, O_CREAT | O_RDONLY, 0660, &attr);
    }
    if (q == (mqd_t)-1) { perror("mq_open"); return 1; }

    pthread_t wt;
    if (pthread_create(&wt, NULL, writer_thread, NULL) != 0) {
        perror("pthread_create");
        mq_close(q); mq_unlink(LOG_QUEUE_NAME);
        return 1;
    }
    fprintf(stderr, "[logger] up, queue %s, msgsize=%zu maxmsg=%ld\n",
            LOG_QUEUE_NAME, sizeof(log_msg_t), attr.mq_maxmsg);

    while (!g_stop) {
        log_msg_t m;
        struct timespec dl;
        clock_gettime(CLOCK_REALTIME, &dl);
        dl.tv_sec += 1;                     /* wake each second to check g_stop */

        ssize_t r = mq_timedreceive(q, (char *)&m, sizeof m, NULL, &dl);
        if (r < 0) {
            if (errno == ETIMEDOUT || errno == EINTR) continue;
            perror("mq_timedreceive");
            break;
        }
        if ((size_t)r != sizeof m) continue;    /* wrong size: ignore */

        m.src[sizeof m.src - 1]   = '\0';       /* never trust the sender */
        m.text[sizeof m.text - 1] = '\0';

        if (m.level == LVL_SHUTDOWN) break;
        g_received++;
        ring_push(&m);
    }

    /* clean shutdown: let the writer drain everything first */
    pthread_mutex_lock(&mu);
    done = 1;
    pthread_cond_broadcast(&not_empty);
    pthread_mutex_unlock(&mu);
    pthread_join(wt, NULL);

    mq_close(q);
    mq_unlink(LOG_QUEUE_NAME);
    fprintf(stderr, "[logger] shutdown: received=%lu written=%lu\n",
            g_received, g_written);
    return 0;
}
