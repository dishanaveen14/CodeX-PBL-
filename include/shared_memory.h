/**
 * @file shared_memory.h
 * @brief Header for POSIX Shared Memory IPC Mechanism with Telemetry.
 *
 * Owned by: Student 2 (Sockets and Shared Memory)
 * Course: Operating Systems - POSIX IPC Project
 */

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHM_DEMO_NAME "/ipc_posix_demo_shm"

typedef struct {
    int32_t is_ready;          /* 1 when writer finished */
    int32_t transaction_id;    /* Transaction sequence */
    int64_t timestamp_usec;    /* Microsecond timestamp */
    char    author[32];        /* Author/process identifier */
    char    data[256];         /* Data payload */
} shm_payload_t;

/**
 * @brief Interactive CLI menu entry point for Shared Memory.
 * Prompts user for shared payload and transaction ID, demonstrating zero-copy RAM sharing.
 */
void run_shared_memory(void);

/**
 * @brief Core execution function for shared memory demonstration.
 *
 * @param shared_data String data to store in shared memory.
 * @param tx_id Transaction ID.
 * @param output_buffer Buffer to store execution log for GUI (optional, can be NULL).
 * @param buffer_size Size of the output buffer.
 * @return 0 on success, negative error code on failure.
 */
int execute_shared_memory_demo(const char *shared_data, int tx_id, char *output_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* SHARED_MEMORY_H */
