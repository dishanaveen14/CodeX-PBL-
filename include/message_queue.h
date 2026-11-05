/**
 * @file message_queue.h
 * @brief Header for POSIX Message Queue IPC Mechanism with Telemetry.
 *
 * Owned by: Student 1 (Pipes and Message Queues)
 * Course: Operating Systems - POSIX IPC Project
 */

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEMO_QUEUE_NAME "/ipc_posix_demo_queue"
#define DEMO_MSG_MAX_LEN 256

/**
 * @brief Interactive CLI menu entry point for Message Queue.
 * Prompts user for message text, priority levels, and demonstrates priority
 * queuing.
 */
void run_message_queue(void);

/**
 * @brief Core execution function for POSIX message queue demonstration.
 *
 * @param user_msg Custom message payload entered by the user.
 * @param priority Priority level (1 to 10, 10 = Highest).
 * @param output_buffer Buffer to store execution log for GUI (optional, can be
 * NULL).
 * @param buffer_size Size of the output buffer.
 * @return 0 on success, negative error code on failure.
 */
int execute_message_queue_demo(const char *user_msg, int priority,
                               char *output_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* MESSAGE_QUEUE_H */
