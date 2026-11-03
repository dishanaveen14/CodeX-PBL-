/**
 * @file recovery.h
 * @brief Person 3: Deadlock Recovery interface declarations.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 3 (Deadlock Detection, Recovery, and RAG)
 * 
 * Note: FUNCTION DECLARATIONS ONLY. Do not add algorithm implementations here.
 */

#ifndef RECOVERY_H
#define RECOVERY_H

#include "../common/types.h"
#include "../common/system_state.h"
#include "detection.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Recovery Strategy Data Structures                                         */
/* ========================================================================= */

/**
 * @brief Strategy used to resolve a deadlock.
 */
typedef enum {
    RECOVERY_STRATEGY_VICTIM_TERMINATION = 0, /**< Terminate victim process and reclaim all its resources */
    RECOVERY_STRATEGY_RESOURCE_PREEMPTION     /**< Preempt selected resources from victim process */
} RecoveryStrategy;

/**
 * @brief Structured outcome of a recovery operation.
 */
typedef struct {
    bool success;                               /**< True if recovery executed and changed state */
    int victim_process_id;                      /**< ID of process selected as victim */
    int released_resources[MAX_RESOURCES];      /**< Resources reclaimed back to Available */
    char description[MAX_DESC_LEN];             /**< Human-readable summary of recovery action */
    DeadlockResult post_recovery_detection;     /**< Detection results immediately following recovery */
} RecoveryResult;

/* ========================================================================= */
/* Function Prototypes                                                       */
/* ========================================================================= */

/**
 * @brief Automatically suggests a victim process to terminate to break the deadlock.
 * 
 * @param state Pointer to the system state.
 * @param deadlock Pointer to the current deadlock detection result.
 * @param out_victim_process_id Pointer to receive selected process ID.
 * @return ERR_SUCCESS on success, ERR_NO_DEADLOCK_DETECTED, or ERR_INVALID_PARAM.
 */
ErrorCode recovery_select_victim(const SystemState *state, 
                                const DeadlockResult *deadlock, 
                                int *out_victim_process_id);

/**
 * @brief Executes deadlock recovery by terminating the chosen victim process.
 * 
 * Workflow:
 *  1. Marks victim process status as PROCESS_STATUS_TERMINATED.
 *  2. Reclaims all resources allocated to the victim into Available.
 *  3. Clears victim's Allocation and Outstanding Requests.
 *  4. Recalculates Need and Available vectors.
 *  5. Re-runs deadlock detection to verify if deadlock was eliminated.
 * 
 * @param state Pointer to the live system state (mutated upon confirmation).
 * @param victim_process_id The selected victim process ID.
 * @param out_result Pointer to receive the recovery action outcome.
 * @return ERR_SUCCESS on success, ERR_VICTIM_INVALID, or ERR_INVALID_PARAM.
 */
ErrorCode recovery_execute_victim_termination(SystemState *state, 
                                             int victim_process_id, 
                                             RecoveryResult *out_result);

#ifdef __cplusplus
}
#endif

#endif /* RECOVERY_H */

