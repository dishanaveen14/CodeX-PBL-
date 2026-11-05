/**
 * @file semaphores.h
 * @brief Header for POSIX Semaphores Mechanism with Telemetry.
 *
 * Owned by: Student 3 (Semaphores and Mutex Locks)
 * Course: Operating Systems - POSIX IPC Project
 */

#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SEM_DEMO_NAME "/ipc_posix_demo_sem"

/**
 * @brief Interactive CLI menu entry point for Semaphores.
 * Prompts user for worker count and iteration count, demonstrating synchronization.
 */
void run_semaphores(void);

/**
 * @brief Core execution function for semaphore demonstration.
 *
 * @param worker_count Number of concurrent workers (threads/processes).
 * @param iterations Number of increment iterations per worker.
 * @param output_buffer Buffer to store execution log for GUI (optional, can be NULL).
 * @param buffer_size Size of the output buffer.
 * @return 0 on success, negative error code on failure.
 */
int execute_semaphores_demo(int worker_count, int iterations, char *output_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* SEMAPHORES_H */
