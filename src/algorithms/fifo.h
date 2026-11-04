/**
 * @file fifo.h
 * @brief First-In First-Out (FIFO) page replacement algorithm interface.
 *
 * Owned by Person 1.
 */

#ifndef FIFO_H
#define FIFO_H

#include "core/simulation_types.h"

/**
 * @brief Executes the First-In First-Out (FIFO) page replacement simulation.
 *
 * @param references Array of page reference numbers.
 * @param reference_count Number of references in the array.
 * @param frame_count Number of page frames available in memory.
 * @return Pointer to dynamically allocated SimulationResult, or NULL on error.
 *         Caller is responsible for freeing with free_simulation_result().
 */
SimulationResult *run_fifo(
    const int references[],
    int reference_count,
    int frame_count
);

#endif /* FIFO_H */

