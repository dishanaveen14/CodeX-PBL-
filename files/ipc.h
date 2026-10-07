/* common/ipc.h : shared contract. Draft from Logger owner; Team Leader to freeze. */
#ifndef IPC_H
#define IPC_H
#include <stdint.h>

#define LOG_QUEUE_NAME "/sim_log"

#define LVL_INFO     0
#define LVL_WARN     1
#define LVL_ERROR    2
#define LVL_SHUTDOWN 255

typedef struct {
    uint64_t ts_ns;     /* CLOCK_REALTIME nanoseconds, 0 = Logger stamps on arrival */
    uint8_t  level;     /* LVL_* */
    char     src[8];    /* "UI", "CORE" ... */
    char     text[200];
} log_msg_t;
#endif
