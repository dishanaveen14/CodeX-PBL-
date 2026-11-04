/**
 * @file memory.h
 * @brief Memory management utilities and lifecycle management for simulation data structures.
 *
 * Owned jointly / coordinated by Person 4 and Person 2.
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include "core/simulation_types.h"

/**
 * @brief Allocates heap memory with error checking.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to allocated memory, or NULL on failure.
 */
void *safe_malloc(size_t size);

/**
 * @brief Allocates zero-initialized heap memory with error checking.
 *
 * @param count Number of elements.
 * @param size Size of each element.
 * @return Pointer to allocated memory, or NULL on failure.
 */
void *safe_calloc(size_t count, size_t size);

/**
 * @brief Safely frees memory and sets the pointer to NULL.
 *
 * @param ptr Pointer to pointer to free.
 */
void safe_free(void **ptr);

/**
 * @brief Allocates and initializes a SimulationResult structure including its step frame arrays.
 *
 * @param algorithm_name Display name of the algorithm.
 * @param reference_count Number of page requests to be simulated.
 * @param frame_count Number of memory frames.
 * @return Allocated SimulationResult pointer or NULL on allocation error.
 */
SimulationResult *create_simulation_result(
    const char *algorithm_name,
    int reference_count,
    int frame_count
);

/**
 * @brief Releases all dynamically allocated memory for a SimulationResult structure.
 *
 * Frees every step's frames array, the steps array, and the result structure itself.
 *
 * @param result Pointer to SimulationResult to free (safe to call with NULL).
 */
void free_simulation_result(SimulationResult *result);

/**
 * @brief Releases an array of SimulationResult pointers.
 *
 * @param results Array of SimulationResult pointers.
 * @param result_count Number of elements in the results array.
 */
void free_simulation_results(SimulationResult **results, int result_count);

#endif /* MEMORY_H */

