/**
 * @file mutex.h
 * @brief Header for POSIX Mutex Locks Mechanism with Telemetry.
 *
 * Owned by: Student 3 (Semaphores and Mutex Locks)
 * Course: Operating Systems - POSIX IPC Project
 */

#ifndef MUTEX_H
#define MUTEX_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Interactive CLI menu entry point for Mutex Locks.
 * Prompts user for initial balance, thread count, and transaction count.
 */
void run_mutex(void);

/**
 * @brief Core execution function for mutex demonstration.
 *
 * @param init_balance Starting bank account balance.
 * @param num_threads Number of concurrent teller threads.
 * @param ops_per_thread Number of matched transactions per thread.
 * @param output_buffer Buffer to store execution log for GUI (optional, can be NULL).
 * @param buffer_size Size of the output buffer.
 * @return 0 on success, negative error code on failure.
 */
int execute_mutex_demo(double init_balance, int num_threads, int ops_per_thread, char *output_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* MUTEX_H */
