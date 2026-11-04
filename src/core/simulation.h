/**
 * @file simulation.h
 * @brief Common simulation engine interface.
 *
 * Owned by Person 2.
 * Consumed by GUI (Person 3) and Integration (Person 4).
 */

#ifndef SIMULATION_H
#define SIMULATION_H

#include "core/simulation_types.h"

/**
 * @brief User selection for which algorithm(s) to simulate.
 */
typedef enum {
    ALGORITHM_FIFO,
    ALGORITHM_LRU,
    ALGORITHM_OPTIMAL,
    ALGORITHM_ALL
} AlgorithmChoice;

/**
 * @brief Central simulation dispatcher.
 *
 * Runs the requested algorithm or all three algorithms sequentially on the same input,
 * calculates statistics, and returns an array of SimulationResult pointers.
 *
 * @param references Array of page reference numbers.
 * @param reference_count Length of the reference string.
 * @param frame_count Number of physical frames.
 * @param choice Algorithm selection (FIFO, LRU, Optimal, or ALL).
 * @param result_count Output pointer receiving the number of SimulationResult items returned (1 or 3).
 * @return Dynamically allocated array of SimulationResult pointers, or NULL on error.
 *         The caller must free the returned array using free_simulation_results().
 */
SimulationResult **run_simulation(
    const int references[],
    int reference_count,
    int frame_count,
    AlgorithmChoice choice,
    int *result_count
);

/**
 * @brief Helper to convert AlgorithmChoice to human-readable string.
 *
 * @param choice The algorithm selection enum value.
 * @return Constant string representation.
 */
const char *algorithm_choice_to_string(AlgorithmChoice choice);

/**
 * @brief Releases memory for a single simulation result.
 *
 * @param result Pointer to SimulationResult to free.
 */
void free_simulation_result(SimulationResult *result);

/**
 * @brief Releases memory for an array of simulation results.
 *
 * @param results Array of SimulationResult pointers to free.
 * @param result_count Number of elements in results array.
 */
void free_simulation_results(SimulationResult **results, int result_count);

#endif /* SIMULATION_H */
