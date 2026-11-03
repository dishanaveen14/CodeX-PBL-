/**
 * @file system_state.h
 * @brief Core shared system state definition for Deadlock Simulation System.
 * 
 * Deadlock Simulation System - PBL Project
 * Defines the central data contract owned and updated by Person 1,
 * analyzed by Person 2 and Person 3, and integrated by Person 4.
 */

#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Process and Resource Descriptors                                         */
/* ========================================================================= */

/**
 * @brief Process metadata.
 */
typedef struct {
    int id;                      /**< Zero-based unique process index (0 .. process_count - 1) */
    char name[MAX_NAME_LEN];     /**< Display label, e.g. "P0", "P1" */
    ProcessStatus status;        /**< Current process state */
} Process;

/**
 * @brief Resource type descriptor.
 */
typedef struct {
    int id;                      /**< Zero-based unique resource index (0 .. resource_count - 1) */
    char name[MAX_NAME_LEN];     /**< Display label, e.g. "A", "B", "R0" */
    int total_instances;         /**< Total physical instances configured for this resource */
} Resource;

/* ========================================================================= */
/* Master System State Structure                                             */
/* ========================================================================= */

/**
 * @brief Complete state of the simulated operating system.
 * 
 * Invariants:
 *  - row index i refers to process with id i (0 <= i < process_count)
 *  - column index j refers to resource with id j (0 <= j < resource_count)
 *  - Need[i][j] = Maximum[i][j] - Allocation[i][j]
 *  - Available[j] = Total[j] - Sum_{i=0..process_count-1} Allocation[i][j]
 *  - Outstanding_requests[i][j] is maintained separately from Need[i][j]
 */
typedef struct {
    /* Dimensions */
    int process_count;                                  /**< Number of active processes */
    int resource_count;                                 /**< Number of configured resource types */

    /* Descriptors */
    Process processes[MAX_PROCESSES];                   /**< Process list */
    Resource resources[MAX_RESOURCES];                  /**< Resource types */

    /* Vectors */
    int total[MAX_RESOURCES];                           /**< Total resource instances */
    int available[MAX_RESOURCES];                       /**< Available resource vector */

    /* Matrices (rows = processes, columns = resources) */
    int allocation[MAX_PROCESSES][MAX_RESOURCES];       /**< Allocation Matrix */
    int maximum[MAX_PROCESSES][MAX_RESOURCES];          /**< Maximum Requirement Matrix */
    int need[MAX_PROCESSES][MAX_RESOURCES];             /**< Need Matrix (calculated) */
    int outstanding_requests[MAX_PROCESSES][MAX_RESOURCES]; /**< Outstanding Request Matrix */

    /* Status */
    SystemSafetyStatus overall_status;                  /**< Current overall system state */
    char status_message[MAX_DESC_LEN];                  /**< Human-readable state summary */
} SystemState;

/* ========================================================================= */
/* State Lifecycle Function Prototypes                                       */
/* ========================================================================= */

/**
 * @brief Initializes a SystemState struct to empty default values.
 * @param state Pointer to the SystemState structure.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM if pointer is NULL.
 */
ErrorCode system_state_init(SystemState *state);

/**
 * @brief Resets all matrices and processes in a SystemState struct.
 * @param state Pointer to the SystemState structure.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM if pointer is NULL.
 */
ErrorCode system_state_reset(SystemState *state);

/**
 * @brief Performs a deep copy of a SystemState struct (for trial allocations).
 * @param src Pointer to source SystemState.
 * @param dest Pointer to destination SystemState.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM if either pointer is NULL.
 */
ErrorCode system_state_clone(const SystemState *src, SystemState *dest);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_STATE_H */

