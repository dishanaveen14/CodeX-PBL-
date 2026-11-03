/**
 * @file error.h
 * @brief Error code definitions and string translation helpers.
 * 
 * Deadlock Simulation System - PBL Project
 */

#ifndef ERROR_H
#define ERROR_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Converts an ErrorCode enum value to a human-readable English string.
 * @param code ErrorCode value.
 * @return Constant string description of the error.
 */
const char* error_code_to_string(ErrorCode code);

/**
 * @brief Converts a SystemSafetyStatus enum value to a string.
 * @param status SystemSafetyStatus value.
 * @return Constant string description ("SAFE", "UNSAFE", "DEADLOCKED", "UNCHECKED").
 */
const char* safety_status_to_string(SystemSafetyStatus status);

/**
 * @brief Converts a ProcessStatus enum value to a string.
 * @param status ProcessStatus value.
 * @return Constant string description ("ACTIVE", "WAITING", "DEADLOCKED", "TERMINATED").
 */
const char* process_status_to_string(ProcessStatus status);

#ifdef __cplusplus
}
#endif

#endif /* ERROR_H */
