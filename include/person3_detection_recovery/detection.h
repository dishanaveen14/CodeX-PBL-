/**
 * @file detection.h
 * @brief Person 3: Deadlock Detection and Outstanding Request interface declarations.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 3 (Deadlock Detection, Recovery, and RAG)
 * 
 * Note: FUNCTION DECLARATIONS ONLY. Do not add algorithm implementations here.
 */

#ifndef DETECTION_H
#define DETECTION_H

#include "../common/types.h"
#include "../common/system_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Deadlock Detection Data Structures                                        */
/* ========================================================================= */

/**
 * @brief Structured outcome of a deadlock detection check.
 */
typedef struct {
    bool deadlock_detected;                     /**< True if at least one process is deadlocked */
    int deadlocked_processes[MAX_PROCESSES];     /**< IDs of deadlocked processes */
    int deadlocked_count;                       /**< Number of deadlocked processes */
    int involved_resources[MAX_RESOURCES];      /**< IDs of resources contested in the deadlock */
    int involved_resource_count;                /**< Number of involved resource types */
    char explanation[MAX_DESC_LEN];             /**< Detailed explanation of the detection outcome */
} DeadlockResult;

/* ========================================================================= */
/* Function Prototypes                                                       */
/* ========================================================================= */

/**
 * @brief Runs the deadlock detection algorithm on the current system state.
 * 
 * Invariants:
 *  - Uses Available Resource Vector, Allocation Matrix, and Outstanding Request Matrix.
 *  - MUST NOT substitute the Need Matrix for Outstanding Requests.
 * 
 * @param state Pointer to the system state.
 * @param out_result Pointer to receive the detection outcome.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM if pointers are NULL.
 */
ErrorCode detection_run(const SystemState *state, DeadlockResult *out_result);

/**
 * @brief Records an outstanding (waiting) resource request for a process.
 * 
 * @param state Pointer to the system state.
 * @param process_id Process ID index.
 * @param request_vector Array of requested instances of length state->resource_count.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM, or ERR_OUT_OF_BOUNDS.
 */
ErrorCode detection_record_outstanding_request(SystemState *state, int process_id, const int *request_vector);

/**
 * @brief Clears the outstanding request for a process (e.g. when granted or cancelled).
 * 
 * @param state Pointer to the system state.
 * @param process_id Process ID index.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM, or ERR_OUT_OF_BOUNDS.
 */
ErrorCode detection_clear_outstanding_request(SystemState *state, int process_id);

/**
 * @brief Checks if a process currently has any non-zero outstanding requests.
 * 
 * @param state Pointer to the system state.
 * @param process_id Process ID index.
 * @param out_is_waiting Pointer to receive boolean waiting status.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM, or ERR_OUT_OF_BOUNDS.
 */
ErrorCode detection_is_process_waiting(const SystemState *state, int process_id, bool *out_is_waiting);

#ifdef __cplusplus
}
#endif

#endif /* DETECTION_H */

