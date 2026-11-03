/**
 * @file types.h
 * @brief Common types, limits, enums, and error codes for Deadlock Simulation System.
 * 
 * Deadlock Simulation System - PBL Project
 * Shared architecture foundation.
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* System Capacity Limits                                                    */
/* ========================================================================= */

#define MAX_PROCESSES       16   /**< Maximum number of concurrent processes */
#define MAX_RESOURCES       16   /**< Maximum number of distinct resource types */
#define MAX_NAME_LEN        32   /**< Maximum length for process/resource identifiers */
#define MAX_DESC_LEN        256  /**< Maximum length for status descriptions & reasons */
#define MAX_STEPS           64   /**< Maximum explanation/trace steps */

/* ========================================================================= */
/* Enums: Process, System, and Algorithm Status                              */
/* ========================================================================= */

/**
 * @brief Current lifecycle status of an individual process.
 */
typedef enum {
    PROCESS_STATUS_UNUSED = 0,   /**< Slot unallocated */
    PROCESS_STATUS_ACTIVE,       /**< Active, executing / holding resources */
    PROCESS_STATUS_WAITING,      /**< Waiting on an outstanding resource request */
    PROCESS_STATUS_DEADLOCKED,   /**< Identified as part of a deadlock set */
    PROCESS_STATUS_TERMINATED    /**< Terminated (e.g. through deadlock recovery) */
} ProcessStatus;

/**
 * @brief Global system safety and deadlock classification.
 * 
 * Note: UNSAFE and DEADLOCKED are strictly separated concepts.
 * A system can be UNSAFE without being DEADLOCKED.
 */
typedef enum {
    SYSTEM_STATUS_UNCHECKED = 0, /**< Initial state or state changed, not yet evaluated */
    SYSTEM_STATUS_SAFE,          /**< A safe sequence exists (Banker's check passes) */
    SYSTEM_STATUS_UNSAFE,        /**< No safe sequence exists, potential future deadlock */
    SYSTEM_STATUS_DEADLOCKED     /**< One or more processes are actively deadlocked */
} SystemSafetyStatus;

/**
 * @brief Error codes returned by functions across all modules.
 */
typedef enum {
    ERR_SUCCESS                  = 0,   /**< Operation completed successfully */
    ERR_NOT_IMPLEMENTED          = -1,  /**< Module stub - implementation pending */
    ERR_INVALID_PARAM            = -2,  /**< Null pointer or out-of-range value passed */
    ERR_CAPACITY_EXCEEDED        = -3,  /**< Max processes or resources exceeded */
    ERR_PROCESS_NOT_FOUND        = -4,  /**< Process ID does not exist */
    ERR_RESOURCE_NOT_FOUND       = -5,  /**< Resource ID does not exist */
    ERR_OVER_ALLOCATION          = -6,  /**< Allocation sum exceeds total available instances */
    ERR_EXCEEDS_MAX_DEMAND       = -7,  /**< Allocation or request exceeds declared maximum */
    ERR_EXCEEDS_NEED             = -8,  /**< Request exceeds remaining Need */
    ERR_INSUFFICIENT_AVAILABLE   = -9,  /**< Request exceeds currently Available resources */
    ERR_SYSTEM_UNSAFE            = -10, /**< Request would leave system in unsafe state */
    ERR_NO_DEADLOCK_DETECTED     = -11, /**< Recovery attempted but system is not deadlocked */
    ERR_VICTIM_INVALID           = -12, /**< Specified recovery victim is invalid */
    ERR_OUT_OF_BOUNDS            = -13  /**< Index or coordinate out of valid range */
} ErrorCode;

#ifdef __cplusplus
}
#endif

#endif /* TYPES_H */

