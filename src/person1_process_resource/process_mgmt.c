/**
 * @file process_mgmt.c
 * @brief Person 1: Process Management Implementation.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 1 (Process and Resource Management)
 */

#include "../../include/person1_process_resource/process_mgmt.h"
#include "../../include/person1_process_resource/matrix_mgmt.h"
#include <string.h>
#include <stdio.h>

ErrorCode sys_process_create(SystemState *state, const char *name, int *out_process_id) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (state->process_count >= MAX_PROCESSES) {
        return ERR_CAPACITY_EXCEEDED;
    }

    int pid = state->process_count;
    state->processes[pid].id = pid;
    state->processes[pid].status = PROCESS_STATUS_ACTIVE;

    if (name != NULL && strlen(name) > 0) {
        strncpy(state->processes[pid].name, name, MAX_NAME_LEN - 1);
        state->processes[pid].name[MAX_NAME_LEN - 1] = '\0';
    } else {
        snprintf(state->processes[pid].name, MAX_NAME_LEN, "P%d", pid);
    }

    /* Initialize process matrix rows to zero */
    for (int j = 0; j < MAX_RESOURCES; j++) {
        state->allocation[pid][j] = 0;
        state->maximum[pid][j] = 0;
        state->need[pid][j] = 0;
        state->outstanding_requests[pid][j] = 0;
    }

    state->process_count++;

    if (out_process_id != NULL) {
        *out_process_id = pid;
    }

    /* Recalculate matrices with the new process row */
    sys_matrix_calculate_need(state);
    sys_matrix_calculate_available(state);

    return ERR_SUCCESS;
}

ErrorCode sys_process_remove(SystemState *state, int process_id) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (process_id < 0 || process_id >= state->process_count) {
        return ERR_PROCESS_NOT_FOUND;
    }

    /* Safe removal rule: release any resources held by this process before removal */
    for (int j = 0; j < state->resource_count; j++) {
        if (state->allocation[process_id][j] > 0) {
            state->available[j] += state->allocation[process_id][j];
            state->allocation[process_id][j] = 0;
        }
        state->outstanding_requests[process_id][j] = 0;
    }

    /* Shift remaining process entries downwards to maintain contiguous indexing */
    for (int i = process_id; i < state->process_count - 1; i++) {
        state->processes[i] = state->processes[i + 1];
        state->processes[i].id = i; /* Re-index ID to match array position */

        for (int j = 0; j < MAX_RESOURCES; j++) {
            state->allocation[i][j] = state->allocation[i + 1][j];
            state->maximum[i][j] = state->maximum[i + 1][j];
            state->need[i][j] = state->need[i + 1][j];
            state->outstanding_requests[i][j] = state->outstanding_requests[i + 1][j];
        }
    }

    /* Clear the last vacated slot */
    int last_idx = state->process_count - 1;
    memset(&state->processes[last_idx], 0, sizeof(Process));
    for (int j = 0; j < MAX_RESOURCES; j++) {
        state->allocation[last_idx][j] = 0;
        state->maximum[last_idx][j] = 0;
        state->need[last_idx][j] = 0;
        state->outstanding_requests[last_idx][j] = 0;
    }

    state->process_count--;

    /* Recalculate Need and Available after process removal */
    sys_matrix_calculate_need(state);
    sys_matrix_calculate_available(state);

    return ERR_SUCCESS;
}

ErrorCode sys_process_get(const SystemState *state, int process_id, Process *out_process) {
    if (state == NULL || out_process == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (process_id < 0 || process_id >= state->process_count) {
        return ERR_PROCESS_NOT_FOUND;
    }

    *out_process = state->processes[process_id];
    return ERR_SUCCESS;
}

ErrorCode sys_process_get_count(const SystemState *state, int *out_count) {
    if (state == NULL || out_count == NULL) {
        return ERR_INVALID_PARAM;
    }

    *out_count = state->process_count;
    return ERR_SUCCESS;
}
