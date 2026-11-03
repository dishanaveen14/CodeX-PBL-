/**
 * @file recovery.c
 * @brief Person 3: Deadlock Recovery Implementation.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 3 (Deadlock Detection, Recovery, and RAG)
 */

#include "../../include/person3_detection_recovery/recovery.h"
#include "../../include/person1_process_resource/matrix_mgmt.h"
#include <string.h>
#include <stdio.h>

ErrorCode recovery_select_victim(const SystemState *state, 
                                const DeadlockResult *deadlock, 
                                int *out_victim_process_id) {
    if (state == NULL || deadlock == NULL || out_victim_process_id == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (!deadlock->deadlock_detected || deadlock->deadlocked_count == 0) {
        return ERR_NO_DEADLOCK_DETECTED;
    }

    /* Selection Heuristic: Terminate the deadlocked process holding the maximum resources */
    int best_victim = -1;
    int max_held_instances = -1;

    for (int k = 0; k < deadlock->deadlocked_count; k++) {
        int pid = deadlock->deadlocked_processes[k];
        if (pid < 0 || pid >= state->process_count) continue;

        int held = 0;
        for (int j = 0; j < state->resource_count; j++) {
            held += state->allocation[pid][j];
        }

        if (held > max_held_instances) {
            max_held_instances = held;
            best_victim = pid;
        }
    }

    if (best_victim == -1) {
        best_victim = deadlock->deadlocked_processes[0];
    }

    *out_victim_process_id = best_victim;
    return ERR_SUCCESS;
}

ErrorCode recovery_execute_victim_termination(SystemState *state, 
                                             int victim_process_id, 
                                             RecoveryResult *out_result) {
    if (state == NULL || out_result == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (victim_process_id < 0 || victim_process_id >= state->process_count) {
        return ERR_VICTIM_INVALID;
    }

    if (state->processes[victim_process_id].status == PROCESS_STATUS_TERMINATED) {
        return ERR_VICTIM_INVALID;
    }

    memset(out_result, 0, sizeof(RecoveryResult));
    out_result->victim_process_id = victim_process_id;

    /* 1. Reclaim all resources held by victim into Available */
    int total_reclaimed = 0;
    for (int j = 0; j < state->resource_count; j++) {
        int alloc = state->allocation[victim_process_id][j];
        out_result->released_resources[j] = alloc;
        total_reclaimed += alloc;

        state->available[j] += alloc;
        state->allocation[victim_process_id][j] = 0;
        state->outstanding_requests[victim_process_id][j] = 0;
        state->need[victim_process_id][j] = state->maximum[victim_process_id][j];
    }

    /* 2. Mark process status as TERMINATED */
    state->processes[victim_process_id].status = PROCESS_STATUS_TERMINATED;

    /* 3. Recalculate derived matrices */
    sys_matrix_calculate_need(state);
    sys_matrix_calculate_available(state);

    /* 4. Run deadlock detection on the updated state */
    detection_run(state, &out_result->post_recovery_detection);

    out_result->success = true;

    /* 5. Update overall system status */
    if (out_result->post_recovery_detection.deadlock_detected) {
        state->overall_status = SYSTEM_STATUS_DEADLOCKED;
        snprintf(out_result->description, MAX_DESC_LEN,
            "Recovery executed: Process %s terminated. Released %d instance(s). System remains DEADLOCKED with %d process(es).",
            state->processes[victim_process_id].name, total_reclaimed,
            out_result->post_recovery_detection.deadlocked_count);
    } else {
        state->overall_status = SYSTEM_STATUS_UNCHECKED;
        snprintf(out_result->description, MAX_DESC_LEN,
            "Recovery SUCCESS! Process %s terminated. Released %d instance(s). Deadlock was successfully RESOLVED!",
            state->processes[victim_process_id].name, total_reclaimed);
    }

    strncpy(state->status_message, out_result->description, MAX_DESC_LEN - 1);

    return ERR_SUCCESS;
}
