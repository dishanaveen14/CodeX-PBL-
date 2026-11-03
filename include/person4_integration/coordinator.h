/**
 * @file coordinator.h
 * @brief Person 4: System Integration, State Orchestration, and Workflow Coordination.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 4 (Architecture, Integration, GUI Coordination, and Final Testing)
 * 
 * Note: FUNCTION DECLARATIONS ONLY. Do not add algorithm implementations here.
 */

#ifndef COORDINATOR_H
#define COORDINATOR_H

#include "../common/types.h"
#include "../common/system_state.h"
#include "../person2_bankers/bankers.h"
#include "../person3_detection_recovery/detection.h"
#include "../person3_detection_recovery/rag.h"
#include "../person3_detection_recovery/recovery.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Integrated Application Context                                            */
/* ========================================================================= */

/**
 * @brief Central application container managing live state, latest algorithm
 *        results, and visual models across all 4 team modules.
 */
typedef struct {
    SystemState live_state;                    /**< The single authoritative source of system state */
    BankerSafetyResult latest_safety_result;   /**< Cache of most recent Banker's safety evaluation */
    DeadlockResult latest_deadlock_result;     /**< Cache of most recent Deadlock Detection result */
    RAGGraph latest_rag;                       /**< Cache of most recent Resource Allocation Graph */
    bool state_modified_since_last_check;      /**< Flag to trigger UI re-computation */
} SystemContext;

/* ========================================================================= */
/* Integration Coordinator Prototypes                                       */
/* ========================================================================= */

/**
 * @brief Initializes the complete system context.
 * 
 * @param ctx Pointer to the application context.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM if ctx is NULL.
 */
ErrorCode coordinator_init(SystemContext *ctx);

/**
 * @brief End-to-end coordinated workflow for handling a resource request:
 *  1. Validates request via Person 1.
 *  2. Evaluates safety via Person 2.
 *  3. If safe, commits allocation via Person 1.
 *  4. If unsafe, records outstanding request via Person 3 and notifies user.
 *  5. Rebuilds RAG via Person 3.
 * 
 * @param ctx Pointer to application context.
 * @param process_id Requesting process ID.
 * @param request_vector Array of requested instances.
 * @return ERR_SUCCESS if handled, error code otherwise.
 */
ErrorCode coordinator_handle_resource_request(SystemContext *ctx, int process_id, const int *request_vector);

/**
 * @brief End-to-end coordinated workflow for running full analysis:
 *  1. Recalculates matrices (Need, Available).
 *  2. Runs Banker's Safety Algorithm.
 *  3. Runs Deadlock Detection.
 *  4. Rebuilds Resource Allocation Graph.
 * 
 * @param ctx Pointer to application context.
 * @return ERR_SUCCESS on success.
 */
ErrorCode coordinator_run_full_analysis(SystemContext *ctx);

/**
 * @brief End-to-end coordinated workflow for deadlock recovery:
 *  1. Verifies system is currently deadlocked.
 *  2. Executes victim termination via Person 3.
 *  3. Releases resources via Person 1.
 *  4. Updates RAG and reruns detection.
 * 
 * @param ctx Pointer to application context.
 * @param victim_process_id Victim process ID.
 * @param out_recovery Pointer to receive recovery details.
 * @return ERR_SUCCESS on success.
 */
ErrorCode coordinator_execute_recovery(SystemContext *ctx, int victim_process_id, RecoveryResult *out_recovery);

/**
 * @brief Synchronizes all dependent models and UI buffers with the current live state.
 * 
 * @param ctx Pointer to application context.
 * @return ERR_SUCCESS on success.
 */
ErrorCode coordinator_sync_all_views(SystemContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* COORDINATOR_H */

