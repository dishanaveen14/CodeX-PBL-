/**
 * @file optimal.h
 * @brief Optimal (Belady's Min) page replacement algorithm interface.
 *
 * Owned by Person 2.
 */

#ifndef OPTIMAL_H
#define OPTIMAL_H

#include "core/simulation_types.h"

/**
 * @brief Executes the Optimal page replacement simulation.
 *
 * Replaces the page that will not be used for the longest period of time in the future.
 *
 * @param references Array of page reference numbers.
 * @param reference_count Number of references in the array.
 * @param frame_count Number of page frames available in memory.
 * @return Pointer to dynamically allocated SimulationResult, or NULL on error.
 *         Caller is responsible for freeing with free_simulation_result().
 */
SimulationResult *run_optimal(
    const int references[],
    int reference_count,
    int frame_count
);

#endif /* OPTIMAL_H */

