/**
 * @file detection.c
 * @brief Person 3: Deadlock Detection and Outstanding Request Tracking Stubs.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 3 (Deadlock Detection, Recovery, and RAG)
 * 
 * IMPORTANT: STUBS ONLY. Actual deadlock detection algorithm will be implemented by Person 3.
 */

#include "../../include/person3_detection_recovery/detection.h"

ErrorCode detection_run(const SystemState *state, DeadlockResult *out_result) {
    (void)state;
    (void)out_result;
    /* TODO: Person 3 - Implement Deadlock Detection using Available, Allocation, and Outstanding Requests */
    return ERR_NOT_IMPLEMENTED;
}

ErrorCode detection_record_outstanding_request(SystemState *state, int process_id, const int *request_vector) {
    (void)state;
    (void)process_id;
    (void)request_vector;
    /* TODO: Person 3 - Implement recording of waiting/outstanding request */
    return ERR_NOT_IMPLEMENTED;
}

ErrorCode detection_clear_outstanding_request(SystemState *state, int process_id) {
    (void)state;
    (void)process_id;
    /* TODO: Person 3 - Implement clearing outstanding request for a process */
    return ERR_NOT_IMPLEMENTED;
}

ErrorCode detection_is_process_waiting(const SystemState *state, int process_id, bool *out_is_waiting) {
    (void)state;
    (void)process_id;
    (void)out_is_waiting;
    /* TODO: Person 3 - Implement checking whether process has outstanding requests */
    return ERR_NOT_IMPLEMENTED;
}

