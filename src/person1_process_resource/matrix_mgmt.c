/**
 * @file matrix_mgmt.c
 * @brief Person 1: Matrix Management, Calculation, and Allocation Implementation.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 1 (Process and Resource Management)
 */

#include "../../include/person1_process_resource/matrix_mgmt.h"
#include "../../include/person1_process_resource/process_mgmt.h"
#include "../../include/person1_process_resource/resource_mgmt.h"
#include <string.h>
#include <stdio.h>

ErrorCode sys_matrix_calculate_need(SystemState *state) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    for (int i = 0; i < state->process_count; i++) {
        for (int j = 0; j < state->resource_count; j++) {
            int need_val = state->maximum[i][j] - state->allocation[i][j];
            state->need[i][j] = (need_val >= 0) ? need_val : 0;
        }
    }

    return ERR_SUCCESS;
}

ErrorCode sys_matrix_calculate_available(SystemState *state) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    for (int j = 0; j < state->resource_count; j++) {
        int allocated_sum = 0;
        for (int i = 0; i < state->process_count; i++) {
            allocated_sum += state->allocation[i][j];
        }

        if (allocated_sum > state->total[j]) {
            return ERR_OVER_ALLOCATION;
        }

        state->available[j] = state->total[j] - allocated_sum;
    }

    return ERR_SUCCESS;
}

ErrorCode sys_matrix_set_allocation(SystemState *state, int process_id, int resource_id, int instances) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (process_id < 0 || process_id >= state->process_count ||
        resource_id < 0 || resource_id >= state->resource_count) {
        return ERR_OUT_OF_BOUNDS;
    }

    if (instances < 0) {
        return ERR_INVALID_PARAM;
    }

    /* If maximum is set, allocation must not exceed declared maximum requirement */
    if (state->maximum[process_id][resource_id] > 0 && 
        instances > state->maximum[process_id][resource_id]) {
        return ERR_EXCEEDS_MAX_DEMAND;
    }

    /* Check if this new allocation would exceed total resource instances across all processes */
    int current_sum = 0;
    for (int i = 0; i < state->process_count; i++) {
        if (i != process_id) {
            current_sum += state->allocation[i][resource_id];
        }
    }

    if (current_sum + instances > state->total[resource_id]) {
        return ERR_OVER_ALLOCATION;
    }

    state->allocation[process_id][resource_id] = instances;

    /* Recalculate derived matrices */
    sys_matrix_calculate_need(state);
    sys_matrix_calculate_available(state);

    return ERR_SUCCESS;
}

ErrorCode sys_matrix_set_maximum(SystemState *state, int process_id, int resource_id, int instances) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (process_id < 0 || process_id >= state->process_count ||
        resource_id < 0 || resource_id >= state->resource_count) {
        return ERR_OUT_OF_BOUNDS;
    }

    if (instances < 0) {
        return ERR_INVALID_PARAM;
    }

    /* Maximum requirement cannot be less than current allocation */
    if (instances < state->allocation[process_id][resource_id]) {
        return ERR_INVALID_PARAM;
    }

    /* Maximum requirement cannot exceed total physical system capacity */
    if (instances > state->total[resource_id]) {
        return ERR_EXCEEDS_MAX_DEMAND;
    }

    state->maximum[process_id][resource_id] = instances;

    /* Recalculate Need Matrix */
    sys_matrix_calculate_need(state);

    return ERR_SUCCESS;
}

ErrorCode sys_matrix_validate_all(const SystemState *state) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    for (int j = 0; j < state->resource_count; j++) {
        int allocated_sum = 0;

        for (int i = 0; i < state->process_count; i++) {
            if (state->allocation[i][j] < 0 || state->maximum[i][j] < 0) {
                return ERR_INVALID_PARAM;
            }

            if (state->allocation[i][j] > state->maximum[i][j]) {
                return ERR_EXCEEDS_MAX_DEMAND;
            }

            if (state->maximum[i][j] > state->total[j]) {
                return ERR_EXCEEDS_MAX_DEMAND;
            }

            allocated_sum += state->allocation[i][j];
        }

        if (allocated_sum > state->total[j]) {
            return ERR_OVER_ALLOCATION;
        }

        if (state->available[j] != state->total[j] - allocated_sum) {
            return ERR_INVALID_PARAM;
        }
    }

    return ERR_SUCCESS;
}

ErrorCode sys_request_validate_preliminary(const SystemState *state, int process_id, const int *request_vector) {
    if (state == NULL || request_vector == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (process_id < 0 || process_id >= state->process_count) {
        return ERR_PROCESS_NOT_FOUND;
    }

    for (int j = 0; j < state->resource_count; j++) {
        if (request_vector[j] < 0) {
            return ERR_INVALID_PARAM;
        }

        /* Check: Request <= Need */
        if (request_vector[j] > state->need[process_id][j]) {
            return ERR_EXCEEDS_NEED;
        }

        /* Check: Request <= Available */
        if (request_vector[j] > state->available[j]) {
            return ERR_INSUFFICIENT_AVAILABLE;
        }
    }

    return ERR_SUCCESS;
}

ErrorCode sys_allocation_apply(SystemState *state, int process_id, const int *request_vector) {
    if (state == NULL || request_vector == NULL) {
        return ERR_INVALID_PARAM;
    }

    ErrorCode val_err = sys_request_validate_preliminary(state, process_id, request_vector);
    if (val_err != ERR_SUCCESS) {
        return val_err;
    }

    for (int j = 0; j < state->resource_count; j++) {
        state->available[j] -= request_vector[j];
        state->allocation[process_id][j] += request_vector[j];
        state->need[process_id][j] -= request_vector[j];
    }

    return ERR_SUCCESS;
}

ErrorCode sys_resources_release(SystemState *state, int process_id, const int *release_vector) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (process_id < 0 || process_id >= state->process_count) {
        return ERR_PROCESS_NOT_FOUND;
    }

    if (release_vector == NULL) {
        /* Release ALL allocated resources held by process */
        for (int j = 0; j < state->resource_count; j++) {
            state->available[j] += state->allocation[process_id][j];
            state->allocation[process_id][j] = 0;
            state->need[process_id][j] = state->maximum[process_id][j];
        }
    } else {
        /* Release specified subset of resources */
        for (int j = 0; j < state->resource_count; j++) {
            if (release_vector[j] < 0 || release_vector[j] > state->allocation[process_id][j]) {
                return ERR_INVALID_PARAM;
            }
            state->available[j] += release_vector[j];
            state->allocation[process_id][j] -= release_vector[j];
            state->need[process_id][j] += release_vector[j];
        }
    }

    return ERR_SUCCESS;
}

ErrorCode sys_load_sample_configuration(SystemState *state, int sample_index) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    system_state_reset(state);

    if (sample_index == 0) {
        /* Sample 0: Silberschatz Standard 5-Process, 3-Resource Safe State */
        sys_resource_create(state, "A", 10, NULL);
        sys_resource_create(state, "B", 5, NULL);
        sys_resource_create(state, "C", 7, NULL);

        for (int i = 0; i < 5; i++) {
            sys_process_create(state, NULL, NULL);
        }

        /* Allocation Matrix */
        int alloc[5][3] = {
            {0, 1, 0},
            {2, 0, 0},
            {3, 0, 2},
            {2, 1, 1},
            {0, 0, 2}
        };

        /* Maximum Requirement Matrix */
        int max_req[5][3] = {
            {7, 5, 3},
            {3, 2, 2},
            {9, 0, 2},
            {2, 2, 2},
            {4, 3, 3}
        };

        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 3; j++) {
                state->maximum[i][j] = max_req[i][j];
                state->allocation[i][j] = alloc[i][j];
            }
        }

        sys_matrix_calculate_need(state);
        sys_matrix_calculate_available(state);
        strncpy(state->status_message, "Loaded Textbook 5-Process 3-Resource Safe Configuration.", MAX_DESC_LEN - 1);
        return ERR_SUCCESS;

    } else if (sample_index == 1) {
        /* Sample 1: Multi-resource Circular Deadlock Scenario (Available = [0, 0, 0]) */
        sys_resource_create(state, "R0", 1, NULL);
        sys_resource_create(state, "R1", 1, NULL);
        sys_resource_create(state, "R2", 1, NULL);

        for (int i = 0; i < 3; i++) {
            sys_process_create(state, NULL, NULL);
        }

        /* P0 holds R0 (1), requests R1 (1) */
        state->allocation[0][0] = 1; state->maximum[0][0] = 1;
        state->maximum[0][1] = 1; state->outstanding_requests[0][1] = 1;
        state->processes[0].status = PROCESS_STATUS_WAITING;

        /* P1 holds R1 (1), requests R2 (1) */
        state->allocation[1][1] = 1; state->maximum[1][1] = 1;
        state->maximum[1][2] = 1; state->outstanding_requests[1][2] = 1;
        state->processes[1].status = PROCESS_STATUS_WAITING;

        /* P2 holds R2 (1), requests R0 (1) */
        state->allocation[2][2] = 1; state->maximum[2][2] = 1;
        state->maximum[2][0] = 1; state->outstanding_requests[2][0] = 1;
        state->processes[2].status = PROCESS_STATUS_WAITING;

        sys_matrix_calculate_need(state);
        sys_matrix_calculate_available(state);
        strncpy(state->status_message, "Loaded 3-Process Circular Deadlock Configuration.", MAX_DESC_LEN - 1);
        return ERR_SUCCESS;
    }

    return ERR_INVALID_PARAM;
}
