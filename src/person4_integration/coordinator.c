/**
 * @file coordinator.c
 * @brief Person 4: System Integration Coordinator Implementation.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 4 (Architecture, Integration, GUI Coordination, and Final Testing)
 */

#include "../../include/person4_integration/coordinator.h"
#include "../../include/person1_process_resource/matrix_mgmt.h"
#include "../../include/person1_process_resource/process_mgmt.h"
#include "../../include/person1_process_resource/resource_mgmt.h"
#include "../../include/person2_bankers/bankers.h"
#include "../../include/person3_detection_recovery/detection.h"
#include "../../include/person3_detection_recovery/rag.h"
#include "../../include/person3_detection_recovery/recovery.h"
#include <string.h>
#include <stdio.h>

ErrorCode coordinator_init(SystemContext *ctx) {
    if (ctx == NULL) {
        return ERR_INVALID_PARAM;
    }

    memset(ctx, 0, sizeof(SystemContext));
    system_state_init(&ctx->live_state);
    rag_reset(&ctx->latest_rag);
    ctx->state_modified_since_last_check = false;

    return ERR_SUCCESS;
}

ErrorCode coordinator_run_full_analysis(SystemContext *ctx) {
    if (ctx == NULL) {
        return ERR_INVALID_PARAM;
    }

    /* 1. Recalculate derived matrices for live state (Person 1) */
    sys_matrix_calculate_need(&ctx->live_state);
    sys_matrix_calculate_available(&ctx->live_state);

    /* 2. Run Banker's Safety Algorithm (Person 2) */
    banker_run_safety_check(&ctx->live_state, &ctx->latest_safety_result);

    /* 3. Run Deadlock Detection (Person 3) */
    detection_run(&ctx->live_state, &ctx->latest_deadlock_result);

    /* 4. Rebuild Resource Allocation Graph (Person 3) */
    rag_build_from_state(&ctx->live_state, &ctx->latest_rag);

    /* 5. Synchronize overall system status */
    if (ctx->latest_deadlock_result.deadlock_detected) {
        ctx->live_state.overall_status = SYSTEM_STATUS_DEADLOCKED;
        snprintf(ctx->live_state.status_message, sizeof(ctx->live_state.status_message),
                 "Deadlock detected: %d process(es) in circular wait.",
                 ctx->latest_deadlock_result.deadlocked_count);
    } else if (ctx->latest_safety_result.is_safe) {
        ctx->live_state.overall_status = SYSTEM_STATUS_SAFE;
        char seq_str[MAX_DESC_LEN] = "";
        banker_format_safe_sequence(&ctx->live_state, &ctx->latest_safety_result.safe_sequence, seq_str, sizeof(seq_str));
        snprintf(ctx->live_state.status_message, sizeof(ctx->live_state.status_message),
                 "System is SAFE. Safe sequence: %.200s", seq_str);
    } else {
        ctx->live_state.overall_status = SYSTEM_STATUS_UNSAFE;
        snprintf(ctx->live_state.status_message, sizeof(ctx->live_state.status_message),
                 "System is UNSAFE. No guaranteed safe sequence exists.");
    }

    ctx->state_modified_since_last_check = false;
    return ERR_SUCCESS;
}

ErrorCode coordinator_handle_resource_request(SystemContext *ctx, int process_id, const int *request_vector) {
    if (ctx == NULL || request_vector == NULL) {
        return ERR_INVALID_PARAM;
    }
    if (process_id < 0 || process_id >= ctx->live_state.process_count) {
        return ERR_INVALID_PARAM;
    }

    /* 1. Preliminary validation via Person 1 (bounds, Need, Available) */
    ErrorCode err = sys_request_validate_preliminary(&ctx->live_state, process_id, request_vector);
    if (err != ERR_SUCCESS) {
        return err;
    }

    /* 2. Banker's trial evaluation via Person 2 */
    BankerRequestResult req_eval;
    err = banker_evaluate_resource_request(&ctx->live_state, process_id, request_vector, &req_eval);
    if (err == ERR_SUCCESS && req_eval.is_granted) {
        /* Safe to grant: commit allocation via Person 1 */
        err = sys_allocation_apply(&ctx->live_state, process_id, request_vector);
        if (err != ERR_SUCCESS) {
            return err;
        }

        /* If this process had an outstanding request, clear it */
        detection_clear_outstanding_request(&ctx->live_state, process_id);

        /* Re-run full analysis to synchronize all dependent models */
        coordinator_run_full_analysis(ctx);
        return ERR_SUCCESS;
    } else {
        /* Denied because granting would lead to an unsafe state:
         * Record as outstanding request via Person 3 per coordinator contract */
        detection_record_outstanding_request(&ctx->live_state, process_id, request_vector);
        coordinator_run_full_analysis(ctx);
        return (err != ERR_SUCCESS) ? err : ERR_SYSTEM_UNSAFE;
    }
}

ErrorCode coordinator_execute_recovery(SystemContext *ctx, int victim_process_id, RecoveryResult *out_recovery) {
    if (ctx == NULL) {
        return ERR_INVALID_PARAM;
    }
    if (victim_process_id < 0 || victim_process_id >= ctx->live_state.process_count) {
        return ERR_INVALID_PARAM;
    }

    /* Execute victim termination and resource reclamation via Person 3 */
    RecoveryResult dummy_rec;
    RecoveryResult *rec_target = (out_recovery != NULL) ? out_recovery : &dummy_rec;

    ErrorCode err = recovery_execute_victim_termination(&ctx->live_state, victim_process_id, rec_target);
    if (err != ERR_SUCCESS) {
        return err;
    }

    /* Re-run full analysis to update matrices, Banker's safety, Deadlock Detection, and RAG */
    coordinator_run_full_analysis(ctx);
    return ERR_SUCCESS;
}

ErrorCode coordinator_sync_all_views(SystemContext *ctx) {
    if (ctx == NULL) {
        return ERR_INVALID_PARAM;
    }
    return coordinator_run_full_analysis(ctx);
}
