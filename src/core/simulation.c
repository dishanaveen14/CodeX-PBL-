/**
 * @file simulation.c
 * @brief Common simulation engine dispatcher implementation.
 *
 * Owned by Person 2.
 */

#include "core/simulation.h"
#include "core/statistics.h"
#include "algorithms/fifo.h"
#include "algorithms/lru.h"
#include "algorithms/optimal.h"
#include "utils/memory.h"
#include <stdio.h>
#include <stdlib.h>

const char *algorithm_choice_to_string(AlgorithmChoice choice) {
    switch (choice) {
        case ALGORITHM_FIFO:
            return "FIFO";
        case ALGORITHM_LRU:
            return "LRU";
        case ALGORITHM_OPTIMAL:
            return "Optimal";
        case ALGORITHM_ALL:
            return "Compare All";
        default:
            return "Unknown";
    }
}

SimulationResult **run_simulation(
    const int references[],
    int reference_count,
    int frame_count,
    AlgorithmChoice choice,
    int *result_count
) {
    if (!result_count) {
        return NULL;
    }
    *result_count = 0;

    if (!references || reference_count <= 0 || frame_count <= 0) {
        return NULL;
    }

    SimulationResult **results = NULL;

    switch (choice) {
        case ALGORITHM_FIFO: {
            results = (SimulationResult **)safe_malloc(sizeof(SimulationResult *));
            if (!results) return NULL;
            results[0] = run_fifo(references, reference_count, frame_count);
            if (!results[0]) {
                free(results);
                return NULL;
            }
            update_simulation_statistics(results[0]);
            *result_count = 1;
            break;
        }

        case ALGORITHM_LRU: {
            results = (SimulationResult **)safe_malloc(sizeof(SimulationResult *));
            if (!results) return NULL;
            results[0] = run_lru(references, reference_count, frame_count);
            if (!results[0]) {
                free(results);
                return NULL;
            }
            update_simulation_statistics(results[0]);
            *result_count = 1;
            break;
        }

        case ALGORITHM_OPTIMAL: {
            results = (SimulationResult **)safe_malloc(sizeof(SimulationResult *));
            if (!results) return NULL;
            results[0] = run_optimal(references, reference_count, frame_count);
            if (!results[0]) {
                free(results);
                return NULL;
            }
            update_simulation_statistics(results[0]);
            *result_count = 1;
            break;
        }

        case ALGORITHM_ALL: {
            results = (SimulationResult **)safe_calloc(3, sizeof(SimulationResult *));
            if (!results) return NULL;

            results[0] = run_fifo(references, reference_count, frame_count);
            results[1] = run_lru(references, reference_count, frame_count);
            results[2] = run_optimal(references, reference_count, frame_count);

            if (!results[0] || !results[1] || !results[2]) {
                /* If any algorithm failed to allocate, clean up and return NULL */
                free_simulation_results(results, 3);
                return NULL;
            }

            update_simulation_statistics(results[0]);
            update_simulation_statistics(results[1]);
            update_simulation_statistics(results[2]);
            *result_count = 3;
            break;
        }

        default:
            return NULL;
    }

    return results;
}

