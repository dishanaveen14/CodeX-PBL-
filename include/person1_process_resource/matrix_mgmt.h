/**
 * @file matrix_mgmt.h
 * @brief Person 1: Matrix calculation, validation, and allocation interfaces.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 1 (Process and Resource Management)
 */

#ifndef MATRIX_MGMT_H
#define MATRIX_MGMT_H

#include "../common/types.h"
#include "../common/system_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sets the allocation value for a specific process and resource.
 * 
 * @param state Pointer to the system state.
 * @param process_id Process index (row).
 * @param resource_id Resource index (column).
 * @param instances Number of allocated instances (must be >= 0).
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM, or ERR_OUT_OF_BOUNDS.
 */
ErrorCode sys_matrix_set_allocation(SystemState *state, int process_id, int resource_id, int instances);

/**
 * @brief Sets the maximum requirement value for a specific process and resource.
 * 
 * @param state Pointer to the system state.
 * @param process_id Process index (row).
 * @param resource_id Resource index (column).
 * @param instances Number of maximum required instances (must be >= 0).
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM, or ERR_OUT_OF_BOUNDS.
 */
ErrorCode sys_matrix_set_maximum(SystemState *state, int process_id, int resource_id, int instances);

/**
 * @brief Recalculates the Need Matrix: Need[i][j] = Maximum[i][j] - Allocation[i][j].
 * 
 * Must be invoked whenever Allocation or Maximum changes.
 * 
 * @param state Pointer to the system state.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM if state is NULL.
 */
ErrorCode sys_matrix_calculate_need(SystemState *state);

/**
 * @brief Recalculates the Available Resource Vector:
 * Available[j] = Total[j] - Sum_{i=0..N-1} Allocation[i][j].
 * 
 * @param state Pointer to the system state.
 * @return ERR_SUCCESS on success, ERR_OVER_ALLOCATION if sum > total, or ERR_INVALID_PARAM.
 */
ErrorCode sys_matrix_calculate_available(SystemState *state);

/**
 * @brief Validates all matrices for mathematical consistency.
 * 
 * Verifies:
 *  1. Non-negative values across all matrices.
 *  2. Allocation[i][j] <= Maximum[i][j] for all i, j.
 *  3. Sum of Allocation[i][j] <= Total[j] for all j.
 * 
 * @param state Pointer to the system state.
 * @return ERR_SUCCESS on valid state, specific error code otherwise.
 */
ErrorCode sys_matrix_validate_all(const SystemState *state);

/**
 * @brief Preliminary request validation (Checks: Request <= Need and Request <= Available).
 * 
 * @param state Pointer to the system state.
 * @param process_id Requesting process ID index.
 * @param request_vector Array of requested instances of length state->resource_count.
 * @return ERR_SUCCESS if valid, ERR_EXCEEDS_NEED, ERR_INSUFFICIENT_AVAILABLE, or ERR_INVALID_PARAM.
 */
ErrorCode sys_request_validate_preliminary(const SystemState *state, int process_id, const int *request_vector);

/**
 * @brief Commits an approved resource allocation to the live system state.
 * 
 * Updates:
 *  - Available = Available - Request
 *  - Allocation = Allocation + Request
 *  - Need = Need - Request
 * 
 * @param state Pointer to the system state.
 * @param process_id Process ID index.
 * @param request_vector Array of requested instances.
 * @return ERR_SUCCESS on success, or error code on invalid parameters.
 */
ErrorCode sys_allocation_apply(SystemState *state, int process_id, const int *request_vector);

/**
 * @brief Releases allocated resources from a process back to the Available vector.
 * 
 * @param state Pointer to the system state.
 * @param process_id Process ID index.
 * @param release_vector Array of released instances (or NULL to release ALL held resources).
 * @return ERR_SUCCESS on success, or error code on failure.
 */
ErrorCode sys_resources_release(SystemState *state, int process_id, const int *release_vector);

/**
 * @brief Loads a standard sample configuration for testing and demonstration.
 * 
 * @param state Pointer to the system state.
 * @param sample_index 0 = Standard Textbook Safe State (5 processes, 3 resources),
 *                     1 = Deadlocked Scenario with outstanding requests.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM on invalid sample index.
 */
ErrorCode sys_load_sample_configuration(SystemState *state, int sample_index);

#ifdef __cplusplus
}
#endif

#endif /* MATRIX_MGMT_H */
