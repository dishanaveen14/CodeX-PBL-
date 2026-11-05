/**
 * @file pipes.h
 * @brief Header for POSIX Pipe IPC Mechanism.
 *
 * Owned by: Student 1 (Pipes and Message Queues)
 * Course: Operating Systems - POSIX IPC Project
 */

#ifndef PIPES_H
#define PIPES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Interactive CLI menu entry point for Pipes.
 * Prompts user for inputs and demonstrates parent-child IPC.
 */
void run_pipes(void);

/**
 * @brief Core execution function for pipes demonstration.
 *
 * @param num1 First number (Parent process).
 * @param num2 Second number (Child process).
 * @param custom_tag Optional label or message tag.
 * @param output_buffer Buffer to store execution log for GUI (optional, can be NULL).
 * @param buffer_size Size of the output buffer.
 * @return 0 on success, negative error code on failure.
 */
int execute_pipes_demo(long long num1, long long num2, const char *custom_tag, char *output_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* PIPES_H */
