/**
 * @file error.c
 * @brief Error and status string conversion utilities.
 * 
 * Deadlock Simulation System - PBL Project
 */

#include "../../include/common/error.h"

const char* error_code_to_string(ErrorCode code) {
    switch (code) {
        case ERR_SUCCESS:
            return "Success";
        case ERR_NOT_IMPLEMENTED:
            return "Operation not implemented (TODO stub)";
        case ERR_INVALID_PARAM:
            return "Invalid parameter or NULL pointer";
        case ERR_CAPACITY_EXCEEDED:
            return "Maximum capacity exceeded";
        case ERR_PROCESS_NOT_FOUND:
            return "Process ID not found";
        case ERR_RESOURCE_NOT_FOUND:
            return "Resource ID not found";
        case ERR_OVER_ALLOCATION:
            return "Allocations exceed total resource capacity";
        case ERR_EXCEEDS_MAX_DEMAND:
            return "Allocation exceeds declared maximum requirement";
        case ERR_EXCEEDS_NEED:
            return "Request exceeds remaining Need";
        case ERR_INSUFFICIENT_AVAILABLE:
            return "Request exceeds currently Available resources";
        case ERR_SYSTEM_UNSAFE:
            return "Request rejected: trial state is unsafe";
        case ERR_NO_DEADLOCK_DETECTED:
            return "No deadlock currently detected";
        case ERR_VICTIM_INVALID:
            return "Selected recovery victim is invalid";
        default:
            return "Unknown error code";
    }
}

const char* safety_status_to_string(SystemSafetyStatus status) {
    switch (status) {
        case SYSTEM_STATUS_SAFE:
            return "SAFE";
        case SYSTEM_STATUS_UNSAFE:
            return "UNSAFE";
        case SYSTEM_STATUS_DEADLOCKED:
            return "DEADLOCKED";
        case SYSTEM_STATUS_UNCHECKED:
        default:
            return "UNCHECKED";
    }
}

const char* process_status_to_string(ProcessStatus status) {
    switch (status) {
        case PROCESS_STATUS_ACTIVE:
            return "ACTIVE";
        case PROCESS_STATUS_WAITING:
            return "WAITING";
        case PROCESS_STATUS_DEADLOCKED:
            return "DEADLOCKED";
        case PROCESS_STATUS_TERMINATED:
            return "TERMINATED";
        case PROCESS_STATUS_UNUSED:
        default:
            return "UNUSED";
    }
}

