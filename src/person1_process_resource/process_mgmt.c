/**
 * @file process_mgmt.c
 * @brief Person 1: Process Management Stub Implementations.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 1 (Process and Resource Management)
 * 
 * IMPORTANT: STUBS ONLY. Actual feature implementation will be done by Person 1.
 */

#include "../../include/person1_process_resource/process_mgmt.h"

ErrorCode sys_process_create(SystemState *state, const char *name, int *out_process_id) {
    (void)state;
    (void)name;
    (void)out_process_id;
    /* TODO: Person 1 - Implement process creation logic */
    return ERR_NOT_IMPLEMENTED;
}

ErrorCode sys_process_remove(SystemState *state, int process_id) {
    (void)state;
    (void)process_id;
    /* TODO: Person 1 - Implement process safe removal logic */
    return ERR_NOT_IMPLEMENTED;
}

ErrorCode sys_process_get(const SystemState *state, int process_id, Process *out_process) {
    (void)state;
    (void)process_id;
    (void)out_process;
    /* TODO: Person 1 - Implement process lookup logic */
    return ERR_NOT_IMPLEMENTED;
}

ErrorCode sys_process_get_count(const SystemState *state, int *out_count) {
    (void)state;
    (void)out_count;
    /* TODO: Person 1 - Implement process count query */
    return ERR_NOT_IMPLEMENTED;
}

