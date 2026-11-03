/**
 * @file resource_mgmt.c
 * @brief Person 1: Resource Management Implementation.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 1 (Process and Resource Management)
 */

#include "../../include/person1_process_resource/resource_mgmt.h"
#include "../../include/person1_process_resource/matrix_mgmt.h"
#include <string.h>
#include <stdio.h>

ErrorCode sys_resource_create(SystemState *state, const char *name, int total_instances, int *out_resource_id) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (state->resource_count >= MAX_RESOURCES) {
        return ERR_CAPACITY_EXCEEDED;
    }

    if (total_instances < 0) {
        return ERR_INVALID_PARAM;
    }

    int rid = state->resource_count;
    state->resources[rid].id = rid;
    state->resources[rid].total_instances = total_instances;

    if (name != NULL && strlen(name) > 0) {
        strncpy(state->resources[rid].name, name, MAX_NAME_LEN - 1);
        state->resources[rid].name[MAX_NAME_LEN - 1] = '\0';
    } else {
        if (rid < 26) {
            snprintf(state->resources[rid].name, MAX_NAME_LEN, "%c", 'A' + rid);
        } else {
            snprintf(state->resources[rid].name, MAX_NAME_LEN, "R%d", rid);
        }
    }

    state->total[rid] = total_instances;
    state->available[rid] = total_instances;

    /* Initialize resource columns for all processes to zero */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        state->allocation[i][rid] = 0;
        state->maximum[i][rid] = 0;
        state->need[i][rid] = 0;
        state->outstanding_requests[i][rid] = 0;
    }

    state->resource_count++;

    if (out_resource_id != NULL) {
        *out_resource_id = rid;
    }

    /* Recalculate Available Vector */
    sys_matrix_calculate_available(state);

    return ERR_SUCCESS;
}

ErrorCode sys_resource_set_total(SystemState *state, int resource_id, int total_instances) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (resource_id < 0 || resource_id >= state->resource_count) {
        return ERR_RESOURCE_NOT_FOUND;
    }

    if (total_instances < 0) {
        return ERR_INVALID_PARAM;
    }

    /* Verify that new total is not less than the sum of currently allocated instances */
    int current_allocated_sum = 0;
    for (int i = 0; i < state->process_count; i++) {
        current_allocated_sum += state->allocation[i][resource_id];
    }

    if (total_instances < current_allocated_sum) {
        return ERR_OVER_ALLOCATION;
    }

    state->resources[resource_id].total_instances = total_instances;
    state->total[resource_id] = total_instances;

    /* Update Available Vector */
    sys_matrix_calculate_available(state);

    return ERR_SUCCESS;
}

ErrorCode sys_resource_get(const SystemState *state, int resource_id, Resource *out_resource) {
    if (state == NULL || out_resource == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (resource_id < 0 || resource_id >= state->resource_count) {
        return ERR_RESOURCE_NOT_FOUND;
    }

    *out_resource = state->resources[resource_id];
    return ERR_SUCCESS;
}

ErrorCode sys_resource_get_count(const SystemState *state, int *out_count) {
    if (state == NULL || out_count == NULL) {
        return ERR_INVALID_PARAM;
    }

    *out_count = state->resource_count;
    return ERR_SUCCESS;
}
