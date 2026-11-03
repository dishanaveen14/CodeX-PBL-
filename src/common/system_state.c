/**
 * @file system_state.c
 * @brief State lifecycle and memory management implementation.
 * 
 * Deadlock Simulation System - PBL Project
 */

#include "../../include/common/system_state.h"
#include <string.h>

ErrorCode system_state_init(SystemState *state) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    memset(state, 0, sizeof(SystemState));
    state->process_count = 0;
    state->resource_count = 0;
    state->overall_status = SYSTEM_STATUS_UNCHECKED;
    strncpy(state->status_message, "System initialized to empty state.", MAX_DESC_LEN - 1);

    return ERR_SUCCESS;
}

ErrorCode system_state_reset(SystemState *state) {
    if (state == NULL) {
        return ERR_INVALID_PARAM;
    }

    return system_state_init(state);
}

ErrorCode system_state_clone(const SystemState *src, SystemState *dest) {
    if (src == NULL || dest == NULL) {
        return ERR_INVALID_PARAM;
    }

    memcpy(dest, src, sizeof(SystemState));
    return ERR_SUCCESS;
}

