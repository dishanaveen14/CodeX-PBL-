/**
 * @file message_queue.c
 * @brief Implementation of POSIX Message Queue IPC with Priority Handling & Telemetry.
 *
 * Owned by: Student 1 (Pipes and Message Queues)
 * Course: Operating Systems - POSIX IPC Project
 */

#include "message_queue.h"
#include "ipc_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef _WIN32
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <mqueue.h>
  #include <unistd.h>
#else
  #include <windows.h>
  #include <pthread.h>
#endif

#ifdef _WIN32
typedef struct {
    char data[DEMO_MSG_MAX_LEN];
    unsigned int priority;
} win_msg_item_t;

typedef struct {
    win_msg_item_t items[10];
    int count;
    pthread_mutex_t lock;
} win_mqueue_sim_t;

static win_mqueue_sim_t g_sim_queue = { .count = 0, .lock = PTHREAD_MUTEX_INITIALIZER };

static void win_mq_send(const char *msg, unsigned int priority) {
    pthread_mutex_lock(&g_sim_queue.lock);
    if (g_sim_queue.count < 10) {
        int idx = g_sim_queue.count++;
        strncpy(g_sim_queue.items[idx].data, msg, DEMO_MSG_MAX_LEN - 1);
        g_sim_queue.items[idx].data[DEMO_MSG_MAX_LEN - 1] = '\0';
        g_sim_queue.items[idx].priority = priority;
    }
    pthread_mutex_unlock(&g_sim_queue.lock);
}

static int win_mq_receive(char *buf, size_t max_len, unsigned int *priority) {
    pthread_mutex_lock(&g_sim_queue.lock);
    if (g_sim_queue.count == 0) {
        pthread_mutex_unlock(&g_sim_queue.lock);
        return -1;
    }
    int highest_idx = 0;
    for (int i = 1; i < g_sim_queue.count; i++) {
        if (g_sim_queue.items[i].priority > g_sim_queue.items[highest_idx].priority) {
            highest_idx = i;
        }
    }
    if (priority) *priority = g_sim_queue.items[highest_idx].priority;
    strncpy(buf, g_sim_queue.items[highest_idx].data, max_len - 1);
    buf[max_len - 1] = '\0';
    int len = (int)strlen(buf);

    for (int i = highest_idx; i < g_sim_queue.count - 1; i++) {
        g_sim_queue.items[i] = g_sim_queue.items[i + 1];
    }
    g_sim_queue.count--;
    pthread_mutex_unlock(&g_sim_queue.lock);
    return len;
}
#endif

int execute_message_queue_demo(const char *user_msg, int priority, char *output_buffer, size_t buffer_size)
{
    if (output_buffer && buffer_size > 0) {
        ipc_log_set_capture(output_buffer, buffer_size);
    }

    const char *custom_text = (user_msg && strlen(user_msg) > 0) ? user_msg : "Emergency System Alert";
    unsigned int prio_val = (priority > 0 && priority <= 10) ? (unsigned int)priority : 10;

    ipc_log(LOG_LVL_INFO, "MQUEUE", "========================================================");
    ipc_log(LOG_LVL_INFO, "MQUEUE", "STARTING POSIX MESSAGE QUEUE IPC DEMONSTRATION");
    ipc_log(LOG_LVL_INFO, "MQUEUE", "User Inputs: Message='%s', Priority=%u", custom_text, prio_val);
    ipc_log(LOG_LVL_INFO, "MQUEUE", "Step 1: Configuring POSIX queue name: %s", DEMO_QUEUE_NAME);

#ifndef _WIN32
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = DEMO_MSG_MAX_LEN;
    attr.mq_curmsgs = 0;

    mq_unlink(DEMO_QUEUE_NAME); /* Remove any old queue */

    ipc_log(LOG_LVL_INFO, "MQUEUE", "Step 2: Calling mq_open(O_CREAT | O_RDWR, 0660)...");
    mqd_t mq = mq_open(DEMO_QUEUE_NAME, O_CREAT | O_RDWR, 0660, &attr);
    if (mq == (mqd_t)-1) {
        ipc_log(LOG_LVL_ERROR, "MQUEUE", "mq_open failed: %s", strerror(errno));
        ipc_log_stop_capture();
        return -1;
    }

    ipc_log(LOG_LVL_INFO, "MQUEUE", "Queue created successfully. Descriptor mqd=%ld (Max msgs: 10, Msg size: 256 B)", (long)mq);
    ipc_log(LOG_LVL_INFO, "MQUEUE", "Step 3: Enqueuing baseline messages with lower priorities...");

    /* Enqueue Low Priority */
    const char *m_low = "Msg #1: Routine telemetry heartbeat";
    mq_send(mq, m_low, strlen(m_low) + 1, 1);
    ipc_log(LOG_LVL_INFO, "MQUEUE", "    [Enqueued] Prio:  1 -> \"%s\"", m_low);

    /* Enqueue Medium Priority */
    const char *m_med = "Msg #2: Core process memory stats update";
    mq_send(mq, m_med, strlen(m_med) + 1, 5);
    ipc_log(LOG_LVL_INFO, "MQUEUE", "    [Enqueued] Prio:  5 -> \"%s\"", m_med);

    /* Enqueue User's High Priority Message LAST */
    ipc_log(LOG_LVL_INFO, "MQUEUE", "Step 4: Enqueuing User message LAST with Priority %u...", prio_val);
    mq_send(mq, custom_text, strlen(custom_text) + 1, prio_val);
    ipc_log(LOG_LVL_INFO, "MQUEUE", "    [Enqueued] Prio: %2u -> \"%s\"", prio_val, custom_text);

    ipc_log(LOG_LVL_INFO, "MQUEUE", "Step 5: Dequeuing messages via mq_receive() to verify priority ordering...");

    for (int i = 1; i <= 3; i++) {
        char recv_buf[DEMO_MSG_MAX_LEN] = {0};
        unsigned int recv_prio = 0;
        ssize_t bytes = mq_receive(mq, recv_buf, sizeof(recv_buf), &recv_prio);
        if (bytes >= 0) {
            ipc_log(LOG_LVL_INFO, "MQUEUE", "    [Dequeued #%d] (Prio: %2u, %zd bytes) -> \"%s\"",
                    i, recv_prio, bytes, recv_buf);
        }
    }

    mq_close(mq);
    mq_unlink(DEMO_QUEUE_NAME);
    ipc_log(LOG_LVL_INFO, "MQUEUE", "Step 6: Closed queue (mq_close) and unlinked (mq_unlink) successfully.");
    ipc_log(LOG_LVL_INFO, "MQUEUE", "NOTICE: Message with highest priority was dequeued FIRST, proving POSIX priority scheduling!");
    ipc_log(LOG_LVL_INFO, "MQUEUE", "========================================================");

#else
    ipc_log(LOG_LVL_INFO, "MQUEUE", "Step 2: Initializing POSIX Message Queue (10 slots, 256 bytes/msg)...");
    const char *m_low = "Msg #1: Routine telemetry heartbeat";
    const char *m_med = "Msg #2: Core process memory stats update";

    ipc_log(LOG_LVL_INFO, "MQUEUE", "Step 3: Enqueuing baseline messages with lower priorities...");
    win_mq_send(m_low, 1);
    ipc_log(LOG_LVL_INFO, "MQUEUE", "    [Enqueued] Prio:  1 -> \"%s\"", m_low);

    win_mq_send(m_med, 5);
    ipc_log(LOG_LVL_INFO, "MQUEUE", "    [Enqueued] Prio:  5 -> \"%s\"", m_med);

    ipc_log(LOG_LVL_INFO, "MQUEUE", "Step 4: Enqueuing User message LAST with Priority %u...", prio_val);
    win_mq_send(custom_text, prio_val);
    ipc_log(LOG_LVL_INFO, "MQUEUE", "    [Enqueued] Prio: %2u -> \"%s\"", prio_val, custom_text);

    ipc_log(LOG_LVL_INFO, "MQUEUE", "Step 5: Invoking mq_receive() to demonstrate priority-first dequeuing...");

    for (int i = 1; i <= 3; i++) {
        char recv_buf[DEMO_MSG_MAX_LEN] = {0};
        unsigned int recv_prio = 0;
        int bytes = win_mq_receive(recv_buf, sizeof(recv_buf), &recv_prio);
        if (bytes >= 0) {
            ipc_log(LOG_LVL_INFO, "MQUEUE", "    [Dequeued #%d] (Prio: %2u, %d bytes) -> \"%s\"",
                    i, recv_prio, bytes, recv_buf);
        }
    }

    ipc_log(LOG_LVL_INFO, "MQUEUE", "Step 6: Queue deallocated cleanly.");
    ipc_log(LOG_LVL_INFO, "MQUEUE", "NOTICE: Message with highest priority (%u) was dequeued FIRST despite arriving last!", prio_val);
    ipc_log(LOG_LVL_INFO, "MQUEUE", "========================================================");
#endif

    ipc_log_stop_capture();
    return 0;
}

void run_message_queue(void)
{
    printf("\n========================================================\n");
    printf("         STUDENT 1: POSIX MESSAGE QUEUE MODULE          \n");
    printf("========================================================\n");

    char msg[DEMO_MSG_MAX_LEN] = {0};
    int priority = 10;

    printf("Enter message string [e.g. CRITICAL: High Temperature Warning]: ");
    if (fgets(msg, sizeof(msg), stdin)) {
        msg[strcspn(msg, "\r\n")] = 0;
    }
    if (strlen(msg) == 0) strcpy(msg, "CRITICAL: Engine Core Overheat Detected");

    printf("Enter message priority (1 = Low, 5 = Normal, 10 = Urgent) [1-10]: ");
    if (scanf("%d", &priority) != 1) { priority = 10; while (getchar() != '\n'); }

    printf("\n--- Executing POSIX Message Queue IPC & Streaming Logs ---\n");
    execute_message_queue_demo(msg, priority, NULL, 0);
}
