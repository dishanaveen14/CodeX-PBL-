/**
 * @file resource_mgmt.h
 * @brief Person 1: Resource management interface declarations.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 1 (Process and Resource Management)
 * 
 * Note: FUNCTION DECLARATIONS ONLY. Do not add algorithm implementations here.
 */

#ifndef RESOURCE_MGMT_H
#define RESOURCE_MGMT_H

#include "../common/types.h"
#include "../common/system_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates a new resource type with a specified total instance count.
 * 
 * @param state Pointer to the system state.
 * @param name Display label (e.g. "R0", "A", "Disk").
 * @param total_instances Number of physical resource instances (must be >= 0).
 * @param out_resource_id Pointer to receive the assigned resource ID index.
 * @return ERR_SUCCESS on success, ERR_CAPACITY_EXCEEDED if MAX_RESOURCES reached,
 *         or ERR_INVALID_PARAM.
 */
ErrorCode sys_resource_create(SystemState *state, const char *name, int total_instances, int *out_resource_id);

/**
 * @brief Updates the total instance count for an existing resource type.
 * 
 * @param state Pointer to the system state.
 * @param resource_id Resource ID index to update.
 * @param total_instances New total count (must be >= current allocated count).
 * @return ERR_SUCCESS on success, ERR_RESOURCE_NOT_FOUND, or ERR_INVALID_PARAM.
 */
ErrorCode sys_resource_set_total(SystemState *state, int resource_id, int total_instances);

/**
 * @brief Retrieves metadata for a specific resource type.
 * 
 * @param state Pointer to the system state.
 * @param resource_id Resource ID index to query.
 * @param out_resource Pointer to receive the resource descriptor.
 * @return ERR_SUCCESS on success, ERR_RESOURCE_NOT_FOUND, or ERR_INVALID_PARAM.
 */
ErrorCode sys_resource_get(const SystemState *state, int resource_id, Resource *out_resource);

/**
 * @brief Returns the total number of currently configured resource types.
 * 
 * @param state Pointer to the system state.
 * @param out_count Pointer to receive the resource count.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM if pointer is NULL.
 */
ErrorCode sys_resource_get_count(const SystemState *state, int *out_count);

#ifdef __cplusplus
}
#endif

#endif /* RESOURCE_MGMT_H */

