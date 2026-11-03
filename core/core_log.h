/*
 * core/core_log.h  –  thin logging façade for the Core process.
 *
 * Usage
 * -----
 *   1. Call core_log_init() once at startup.  It opens /sim_log O_WRONLY.
 *      If the Logger is not running yet, the queue may not exist; the call
 *      returns -1 and every subsequent core_log() call is a no-op.
 *
 *   2. Call core_log(level, fmt, ...) anywhere in Core code to emit a
 *      structured log message.  level is LVL_INFO / LVL_WARN / LVL_ERROR.
 *
 *   3. Call core_log_close() at shutdown (optional – OS will clean up).
 *
 * Design notes
 * ------------
 *  • src is always "CORE" (7 chars + NUL fits src[8]).
 *  • ts_ns is stamped here with CLOCK_REALTIME so the Logger's per-arrival
 *    timestamp is not needed (but 0 would also be fine).
 *  • mq_send uses O_NONBLOCK so the Core never blocks on a full queue.
 *  • The header is self-contained; it #includes common/ipc.h via a
 *    relative path that matches the project's directory layout.
 */
#ifndef CORE_LOG_H
#define CORE_LOG_H

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../common/ipc.h"   /* log_msg_t, LOG_QUEUE_NAME, LVL_* */

/* Module-private state (defined in a single translation unit via the header
 * being included from core_main.c only; all other TUs use extern declarations
 * below or call through the inline helpers).                                */

#ifdef CORE_LOG_IMPL          /* defined once, in core_main.c */
static mqd_t  g_log_q  = (mqd_t)-1;
static int    g_log_ok = 0;   /* 1 after a successful init */
#else
extern mqd_t  g_log_q;
extern int    g_log_ok;
#endif

/* -------------------------------------------------------------------------
 * core_log_init() – open the Logger queue.
 *
 * Returns  0 on success, -1 if mq_open fails (Logger not running).
 * ------------------------------------------------------------------------- */
static inline int core_log_init(void)
{
    /* Open write-only (blocking).  We use mq_timedsend with a short deadline
     * in core_log() so the Core never blocks indefinitely, but we do not
     * use O_NONBLOCK here because many kernels limit mq_maxmsg to 10,
     * which would cause spurious drops on even small bursts.              */
    g_log_q = mq_open(LOG_QUEUE_NAME, O_WRONLY);
    if (g_log_q == (mqd_t)-1) {
        /* Best-effort: print a warning to stderr but do not abort. */
        fprintf(stderr,
                "[CORE] Warning: cannot open log queue %s (%s). "
                "Logging disabled.\n",
                LOG_QUEUE_NAME, strerror(errno));
        return -1;
    }
    g_log_ok = 1;
    return 0;
}

/* -------------------------------------------------------------------------
 * core_log() – format and send one log message.
 *
 * Uses mq_timedsend with a 10 ms deadline: if the Logger queue is still full
 * after 10 ms the message is silently dropped so the Core never stalls for
 * longer than that on a logging call.
 * ------------------------------------------------------------------------- */
static inline void core_log(uint8_t level, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

static inline void core_log(uint8_t level, const char *fmt, ...)
{
    if (!g_log_ok) return;

    log_msg_t m;
    memset(&m, 0, sizeof m);

    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
        m.ts_ns = (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;

    m.level = level;
    strncpy(m.src, "CORE", sizeof m.src - 1);

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(m.text, sizeof m.text, fmt, ap);
    va_end(ap);

    /* mq_timedsend: wait up to 10 ms for space; drop silently on timeout. */
    struct timespec dl;
    clock_gettime(CLOCK_REALTIME, &dl);
    dl.tv_nsec += 10000000L;   /* +10 ms */
    if (dl.tv_nsec >= 1000000000L) { dl.tv_sec++; dl.tv_nsec -= 1000000000L; }

    (void)mq_timedsend(g_log_q, (const char *)&m, sizeof m, 0, &dl);
}

/* -------------------------------------------------------------------------
 * core_log_close() – release the queue handle.
 * ------------------------------------------------------------------------- */
static inline void core_log_close(void)
{
    if (g_log_ok) {
        mq_close(g_log_q);
        g_log_q  = (mqd_t)-1;
        g_log_ok = 0;
    }
}

#endif /* CORE_LOG_H */
