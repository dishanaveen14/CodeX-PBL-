/**
 * @file ipc_logger.h
 * @brief Unified Logging and Telemetry Subsystem for POSIX IPC Project.
 *
 * Appends timestamped events to sim.log and displays formatted logs to console/GUI.
 */

#ifndef IPC_LOGGER_H
#define IPC_LOGGER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_LVL_INFO  "INFO "
#define LOG_LVL_WARN  "WARN "
#define LOG_LVL_ERROR "ERROR"

/**
 * @brief Logs a formatted telemetry message.
 *
 * Writes to:
 * 1. stdout (formatted console output)
 * 2. sim.log file on disk (persistent audit trail)
 * 3. In-memory capture buffer (if enabled for GUI)
 *
 * @param level Log severity (INFO, WARN, ERROR).
 * @param module Component name (PIPES, MQUEUE, SOCKETS, SHM, SEM, MUTEX, CORE).
 * @param fmt Printf-style format string.
 */
void ipc_log(const char *level, const char *module, const char *fmt, ...);

/**
 * @brief Sets a buffer to capture all ipc_log messages (used by GUI).
 */
void ipc_log_set_capture(char *buf, size_t max_size);

/**
 * @brief Stops capturing logs to the memory buffer.
 */
void ipc_log_stop_capture(void);

#ifdef __cplusplus
}
#endif

#endif /* IPC_LOGGER_H */

