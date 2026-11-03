/**
 * @file process_mgmt.h
 * @brief Person 1: Process management interface declarations.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 1 (Process and Resource Management)
 * 
 * Note: FUNCTION DECLARATIONS ONLY. Do not add algorithm implementations here.
 */

#ifndef PROCESS_MGMT_H
#define PROCESS_MGMT_H

#include "../common/types.h"
#include "../common/system_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates a new process and registers it with the system state.
 * 
 * @param state Pointer to the system state.
 * @param name Display identifier (e.g. "P0", "P1"). If NULL, auto-generates identifier.
 * @param out_process_id Pointer to receive the assigned process ID index.
 * @return ERR_SUCCESS on success, ERR_CAPACITY_EXCEEDED if MAX_PROCESSES reached,
 *         or ERR_INVALID_PARAM.
 */
ErrorCode sys_process_create(SystemState *state, const char *name, int *out_process_id);

/**
 * @brief Removes a process from the system state if it has no active allocations.
 * 
 * @param state Pointer to the system state.
 * @param process_id The process ID index to remove.
 * @return ERR_SUCCESS on success, ERR_PROCESS_NOT_FOUND, or ERR_INVALID_PARAM.
 */
ErrorCode sys_process_remove(SystemState *state, int process_id);

/**
 * @brief Retrieves metadata for a specific process.
 * 
 * @param state Pointer to the system state.
 * @param process_id The process ID index to query.
 * @param out_process Pointer to receive the process metadata.
 * @return ERR_SUCCESS on success, ERR_PROCESS_NOT_FOUND, or ERR_INVALID_PARAM.
 */
ErrorCode sys_process_get(const SystemState *state, int process_id, Process *out_process);

/**
 * @brief Returns the total number of currently registered processes.
 * 
 * @param state Pointer to the system state.
 * @param out_count Pointer to receive the process count.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM if pointer is NULL.
 */
ErrorCode sys_process_get_count(const SystemState *state, int *out_count);

#ifdef __cplusplus
}
#endif

#endif /* PROCESS_MGMT_H */

