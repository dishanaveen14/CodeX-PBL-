/**
 * @file lru.c
 * @brief Implementation of Least Recently Used (LRU) page replacement algorithm.
 *
 * Owned by Person 1.
 */

#include "algorithms/lru.h"
#include "core/statistics.h"
#include "utils/memory.h"
#include <stdio.h>
#include <stdlib.h>

SimulationResult *run_lru(
    const int references[],
    int reference_count,
    int frame_count
) {
    if (!references || reference_count <= 0 || frame_count <= 0) {
        return NULL;
    }

    /* Reject invalid negative page numbers (-1 is reserved for EMPTY_FRAME) */
    for (int i = 0; i < reference_count; ++i) {
        if (references[i] < 0) {
            return NULL;
        }
    }

    SimulationResult *result = create_simulation_result("LRU", reference_count, frame_count);
    if (!result) {
        return NULL;
    }

    /* Working array for resident page frames */
    int *current_frames = (int *)safe_malloc((size_t)frame_count * sizeof(int));
    if (!current_frames) {
        free_simulation_result(result);
        return NULL;
    }

    /* Array tracking the last reference step index for each frame */
    int *last_used = (int *)safe_malloc((size_t)frame_count * sizeof(int));
    if (!last_used) {
        free(current_frames);
        free_simulation_result(result);
        return NULL;
    }

    /* Initialize frames to EMPTY_FRAME (-1) and timestamps to -1 */
    for (int f = 0; f < frame_count; ++f) {
        current_frames[f] = EMPTY_FRAME;
        last_used[f] = -1;
    }

    /* Process references sequentially */
    for (int i = 0; i < reference_count; ++i) {
        int current_page = references[i];
        result->steps[i].page = current_page;

        /* Check if requested page is already resident (Hit) */
        int hit_index = -1;
        for (int f = 0; f < frame_count; ++f) {
            if (current_frames[f] == current_page) {
                hit_index = f;
                break;
            }
        }

        if (hit_index != -1) {
            /* Page Hit: update access timestamp for this frame */
            result->steps[i].status = PAGE_HIT;
            last_used[hit_index] = i;
        } else {
            /* Page Fault */
            result->steps[i].status = PAGE_FAULT;

            /* Check if an empty frame is available */
            int empty_index = -1;
            for (int f = 0; f < frame_count; ++f) {
                if (current_frames[f] == EMPTY_FRAME) {
                    empty_index = f;
                    break;
                }
            }

            if (empty_index != -1) {
                /* Place page into first available empty frame */
                current_frames[empty_index] = current_page;
                last_used[empty_index] = i;
            } else {
                /* All frames occupied: evict the least recently used frame */
                int lru_index = 0;
                int min_timestamp = last_used[0];

                for (int f = 1; f < frame_count; ++f) {
                    if (last_used[f] < min_timestamp) {
                        min_timestamp = last_used[f];
                        lru_index = f;
                    }
                }

                current_frames[lru_index] = current_page;
                last_used[lru_index] = i;
            }
        }

        /* Snapshot current frame state into result step */
        for (int f = 0; f < frame_count; ++f) {
            result->steps[i].frames[f] = current_frames[f];
        }
    }

    free(last_used);
    free(current_frames);

    /* Update summary statistics (total hits, faults, ratios) */
    update_simulation_statistics(result);

    return result;
}
