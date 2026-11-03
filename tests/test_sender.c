/* test_sender.c : pretend Core. Usage: ./test_sender [count]  (default 6 sample msgs) */
#define _GNU_SOURCE
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../common/ipc.h"

static void send_one(mqd_t q, uint8_t lvl, const char *src, const char *txt)
{
    log_msg_t m;
    struct timespec ts;
    memset(&m, 0, sizeof m);
    clock_gettime(CLOCK_REALTIME, &ts);
    m.ts_ns = (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
    m.level = lvl;
    strncpy(m.src, src, sizeof m.src - 1);
    strncpy(m.text, txt, sizeof m.text - 1);
    if (mq_send(q, (const char *)&m, sizeof m, 0) < 0) perror("mq_send");
}

int main(int argc, char **argv)
{
    mqd_t q = mq_open(LOG_QUEUE_NAME, O_WRONLY);
    if (q == (mqd_t)-1) { perror("mq_open (is the logger running?)"); return 1; }

    if (argc > 1 && strcmp(argv[1], "shutdown") == 0) {
        send_one(q, LVL_SHUTDOWN, "TEST", "bye");
    } else {
        send_one(q, LVL_INFO,  "UI",   "User typed: LOAD prog.bin");
        send_one(q, LVL_INFO,  "CORE", "Program loaded, 42 instructions");
        send_one(q, LVL_INFO,  "CORE", "Executed ADD R1, R2");
        send_one(q, LVL_WARN,  "CORE", "Queue is 90% full");
        send_one(q, LVL_ERROR, "CORE", "Stack overflow at SP=0xFFFC");
        send_one(q, LVL_ERROR, "CORE", "Bad memory address 0xDEADBEEF");
    }
    mq_close(q);
    return 0;
}
