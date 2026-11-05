/**
 * @file sockets.h
 * @brief Header for Socket-based IPC Mechanism with Telemetry.
 *
 * Owned by: Student 2 (Sockets and Shared Memory)
 * Course: Operating Systems - POSIX IPC Project
 */

#ifndef SOCKETS_H
#define SOCKETS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SOCKET_BUFFER_SIZE 512

/**
 * @brief Interactive CLI menu entry point for Sockets.
 * Prompts user for client message and executes client-server stream communication.
 */
void run_sockets(void);

/**
 * @brief Core execution function for socket demonstration.
 *
 * @param client_message Message payload to transmit from client to server.
 * @param output_buffer Buffer to store execution log for GUI (optional, can be NULL).
 * @param buffer_size Size of the output buffer.
 * @return 0 on success, negative error code on failure.
 */
int execute_sockets_demo(const char *client_message, char *output_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* SOCKETS_H */
