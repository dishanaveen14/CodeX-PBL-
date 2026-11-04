/**
 * @file optimal.c
 * @brief Implementation of the Optimal (Belady's Min) page replacement algorithm.
 *
 * Owned by Person 2.
 */

#include "algorithms/optimal.h"
#include "core/statistics.h"
#include "utils/memory.h"
#include <stdio.h>
#include <stdlib.h>

SimulationResult *run_optimal(
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

    SimulationResult *result = create_simulation_result("Optimal", reference_count, frame_count);
    if (!result) {
        return NULL;
    }

    /* Allocate working frame array to track resident pages */
    int *current_frames = (int *)safe_malloc((size_t)frame_count * sizeof(int));
    if (!current_frames) {
        free_simulation_result(result);
        return NULL;
    }

    /* Initialize all frames as empty */
    for (int f = 0; f < frame_count; ++f) {
        current_frames[f] = EMPTY_FRAME;
    }

    /* Process references sequentially */
    for (int i = 0; i < reference_count; ++i) {
        int current_page = references[i];
        result->steps[i].page = current_page;

        /* 1. Check if the requested page is already resident in any frame (Hit) */
        int hit_index = -1;
        for (int f = 0; f < frame_count; ++f) {
            if (current_frames[f] == current_page) {
                hit_index = f;
                break;
            }
        }

        if (hit_index != -1) {
            /* Page Hit: no frame replacement occurs */
            result->steps[i].status = PAGE_HIT;
        } else {
            /* Page Fault */
            result->steps[i].status = PAGE_FAULT;

            /* 2. Check if an empty frame is available */
            int empty_index = -1;
            for (int f = 0; f < frame_count; ++f) {
                if (current_frames[f] == EMPTY_FRAME) {
                    empty_index = f;
                    break;
                }
            }

            if (empty_index != -1) {
                /* Load page into first available empty frame */
                current_frames[empty_index] = current_page;
            } else {
                /* 3. All frames occupied: examine future references to select victim */
                int victim_frame = -1;
                int farthest_use = -1;

                for (int f = 0; f < frame_count; ++f) {
                    int next_use = -1;

                    /* Look ahead strictly in remaining references (from i + 1 to reference_count - 1) */
                    for (int j = i + 1; j < reference_count; ++j) {
                        if (references[j] == current_frames[f]) {
                            next_use = j;
                            break;
                        }
                    }

                    /* If a page is never referenced again, it is the top candidate for eviction */
                    if (next_use == -1) {
                        victim_frame = f;
                        break;
                    }

                    /* Otherwise, pick the page whose next reference occurs farthest in future */
                    if (next_use > farthest_use) {
                        farthest_use = next_use;
                        victim_frame = f;
                    }
                }

                if (victim_frame < 0 || victim_frame >= frame_count) {
                    victim_frame = 0;
                }

                current_frames[victim_frame] = current_page;
            }
        }

        /* 4. Record current frame snapshot in the simulation step */
        for (int f = 0; f < frame_count; ++f) {
            result->steps[i].frames[f] = current_frames[f];
        }
    }

    free(current_frames);

    /* 5. Calculate and populate summary metrics */
    update_simulation_statistics(result);

    return result;
}
