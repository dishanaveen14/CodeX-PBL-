/**
 * @file memory.c
 * @brief Implementation of memory management functions.
 */

#include "utils/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *safe_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "[Memory Error] Failed to allocate %zu bytes\n", size);
    }
    return ptr;
}

void *safe_calloc(size_t count, size_t size) {
    if (count == 0 || size == 0) {
        return NULL;
    }
    void *ptr = calloc(count, size);
    if (!ptr) {
        fprintf(stderr, "[Memory Error] Failed to calloc %zu elements of %zu bytes\n", count, size);
    }
    return ptr;
}

void safe_free(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

SimulationResult *create_simulation_result(
    const char *algorithm_name,
    int reference_count,
    int frame_count
) {
    if (reference_count < 0 || frame_count <= 0) {
        return NULL;
    }

    SimulationResult *result = (SimulationResult *)safe_calloc(1, sizeof(SimulationResult));
    if (!result) {
        return NULL;
    }

    if (algorithm_name) {
        strncpy(result->algorithm_name, algorithm_name, sizeof(result->algorithm_name) - 1);
        result->algorithm_name[sizeof(result->algorithm_name) - 1] = '\0';
    }

    result->total_references = reference_count;
    result->frame_count = frame_count;
    result->total_hits = 0;
    result->total_faults = 0;
    result->hit_ratio = 0.0;
    result->fault_ratio = 0.0;

    if (reference_count > 0) {
        result->steps = (SimulationStep *)safe_calloc((size_t)reference_count, sizeof(SimulationStep));
        if (!result->steps) {
            free(result);
            return NULL;
        }

        for (int i = 0; i < reference_count; ++i) {
            result->steps[i].page = 0;
            result->steps[i].status = PAGE_FAULT;
            result->steps[i].frames = (int *)safe_malloc((size_t)frame_count * sizeof(int));
            if (!result->steps[i].frames) {
                /* Cleanup partially allocated steps */
                for (int j = 0; j < i; ++j) {
                    free(result->steps[j].frames);
                }
                free(result->steps);
                free(result);
                return NULL;
            }
            /* Initialize frames to empty */
            for (int f = 0; f < frame_count; ++f) {
                result->steps[i].frames[f] = EMPTY_FRAME;
            }
        }
    } else {
        result->steps = NULL;
    }

    return result;
}

void free_simulation_result(SimulationResult *result) {
    if (!result) {
        return;
    }

    if (result->steps) {
        for (int i = 0; i < result->total_references; ++i) {
            if (result->steps[i].frames) {
                free(result->steps[i].frames);
                result->steps[i].frames = NULL;
            }
        }
        free(result->steps);
        result->steps = NULL;
    }

    free(result);
}

void free_simulation_results(SimulationResult **results, int result_count) {
    if (!results) {
        return;
    }

    for (int i = 0; i < result_count; ++i) {
        if (results[i]) {
            free_simulation_result(results[i]);
            results[i] = NULL;
        }
    }
    free(results);
}

