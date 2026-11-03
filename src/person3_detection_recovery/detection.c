/**
 * @file detection.c
 * @brief Person 3: Deadlock Detection and Outstanding Request Implementation.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 3 (Deadlock Detection, Recovery, and RAG)
 */

#include "../../include/person3_detection_recovery/detection.h"
#include <string.h>
#include <stdio.h>

ErrorCode detection_record_outstanding_request(SystemState *state, int process_id, const int *request_vector) {
    if (state == NULL || request_vector == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (process_id < 0 || process_id >= state->process_count) {
        return ERR_PROCESS_NOT_FOUND;
    }

    bool has_nonzero_request = false;
    for (int j = 0; j < state->resource_count; j++) {
        if (request_vector[j] < 0) {
            return ERR_INVALID_PARAM;
        }
        state->outstanding_requests[process_id][j] = request_vector[j];
        if (request_vector[j] > 0) {
            has_nonzero_request = true;
        }
    }

    if (state->processes[process_id].status != PROCESS_STATUS_TERMINATED) {
        state->processes[process_id].status = has_nonzero_request ? 
            PROCESS_STATUS_WAITING : PROCESS_STATUS_ACTIVE;
    }

    return ERR_SUCCESS;
}

ErrorCode detection_clear_outstanding_request(SystemState *state, int process_id) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (process_id < 0 || process_id >= state->process_count) {
        return ERR_PROCESS_NOT_FOUND;
    }

    for (int j = 0; j < state->resource_count; j++) {
        state->outstanding_requests[process_id][j] = 0;
    }

    if (state->processes[process_id].status == PROCESS_STATUS_WAITING ||
        state->processes[process_id].status == PROCESS_STATUS_DEADLOCKED) {
        state->processes[process_id].status = PROCESS_STATUS_ACTIVE;
    }

    return ERR_SUCCESS;
}

ErrorCode detection_is_process_waiting(const SystemState *state, int process_id, bool *out_is_waiting) {
    if (state == NULL || out_is_waiting == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (process_id < 0 || process_id >= state->process_count) {
        return ERR_PROCESS_NOT_FOUND;
    }

    *out_is_waiting = false;
    for (int j = 0; j < state->resource_count; j++) {
        if (state->outstanding_requests[process_id][j] > 0) {
            *out_is_waiting = true;
            break;
        }
    }

    return ERR_SUCCESS;
}

ErrorCode detection_run(const SystemState *state, DeadlockResult *out_result) {
    if (state == NULL || out_result == NULL) {
        return ERR_INVALID_PARAM;
    }

    memset(out_result, 0, sizeof(DeadlockResult));

    if (state->process_count == 0) {
        out_result->deadlock_detected = false;
        strncpy(out_result->explanation, "No active processes registered. No deadlock detected.", MAX_DESC_LEN - 1);
        return ERR_SUCCESS;
    }

    /* 1. Initialize Work vector as a copy of Available */
    int work[MAX_RESOURCES];
    for (int j = 0; j < state->resource_count; j++) {
        work[j] = state->available[j];
    }

    /* 2. Initialize Finish boolean vector (Silberschatz Section 8.6.2) */
    bool finish[MAX_PROCESSES];
    for (int i = 0; i < state->process_count; i++) {
        if (state->processes[i].status == PROCESS_STATUS_TERMINATED) {
            finish[i] = true;
            continue;
        }

        /* Check if process holds any allocated resources */
        bool holds_resources = false;
        for (int j = 0; j < state->resource_count; j++) {
            if (state->allocation[i][j] > 0) {
                holds_resources = true;
                break;
            }
        }

        /* If Allocation_i == 0, then Finish[i] = true; otherwise Finish[i] = false */
        finish[i] = !holds_resources;
    }

    /* 3. Search for process i such that Finish[i] == false and Request_i <= Work */
    while (1) {
        bool progress = false;

        for (int i = 0; i < state->process_count; i++) {
            if (!finish[i]) {
                bool can_satisfy_request = true;
                for (int j = 0; j < state->resource_count; j++) {
                    if (state->outstanding_requests[i][j] > work[j]) {
                        can_satisfy_request = false;
                        break;
                    }
                }

                if (can_satisfy_request) {
                    /* Work = Work + Allocation_i; Finish[i] = true */
                    for (int j = 0; j < state->resource_count; j++) {
                        work[j] += state->allocation[i][j];
                    }
                    finish[i] = true;
                    progress = true;
                    break;
                }
            }
        }

        if (!progress) {
            break;
        }
    }

    /* 4. Determine deadlocked processes */
    out_result->deadlocked_count = 0;
    for (int i = 0; i < state->process_count; i++) {
        if (!finish[i] && state->processes[i].status != PROCESS_STATUS_TERMINATED) {
            out_result->deadlocked_processes[out_result->deadlocked_count++] = i;
        }
    }

    if (out_result->deadlocked_count > 0) {
        out_result->deadlock_detected = true;

        /* Identify involved contested resource types */
        bool res_marked[MAX_RESOURCES] = {false};
        out_result->involved_resource_count = 0;

        for (int k = 0; k < out_result->deadlocked_count; k++) {
            int pid = out_result->deadlocked_processes[k];
            for (int j = 0; j < state->resource_count; j++) {
                if ((state->outstanding_requests[pid][j] > 0 || state->allocation[pid][j] > 0) && !res_marked[j]) {
                    res_marked[j] = true;
                    out_result->involved_resources[out_result->involved_resource_count++] = j;
                }
            }
        }

        /* Format description */
        char proc_list[MAX_DESC_LEN] = "";
        for (int k = 0; k < out_result->deadlocked_count; k++) {
            int pid = out_result->deadlocked_processes[k];
            char temp[48];
            snprintf(temp, sizeof(temp), "%s%s", (k > 0) ? ", " : "", state->processes[pid].name);
            strncat(proc_list, temp, sizeof(proc_list) - strlen(proc_list) - 1);
        }

        char res_list[MAX_DESC_LEN] = "";
        for (int k = 0; k < out_result->involved_resource_count; k++) {
            int rid = out_result->involved_resources[k];
            char temp[48];
            snprintf(temp, sizeof(temp), "%s%s", (k > 0) ? ", " : "", state->resources[rid].name);
            strncat(res_list, temp, sizeof(res_list) - strlen(res_list) - 1);
        }

        snprintf(out_result->explanation, MAX_DESC_LEN,
            "DEADLOCK DETECTED! %d process(es) in circular wait: [%.80s]. Contested resources: [%.80s].",
            out_result->deadlocked_count, proc_list, res_list);
    } else {
        out_result->deadlock_detected = false;
        out_result->involved_resource_count = 0;
        snprintf(out_result->explanation, MAX_DESC_LEN,
            "NO DEADLOCK DETECTED. All current outstanding requests can eventually be granted.");
    }

    return ERR_SUCCESS;
}
