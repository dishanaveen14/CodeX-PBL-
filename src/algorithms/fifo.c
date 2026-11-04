/**
 * @file fifo.c
 * @brief Implementation of First-In First-Out (FIFO) page replacement algorithm.
 *
 * Owned by Person 1.
 */

#include "algorithms/fifo.h"
#include "core/statistics.h"
#include "utils/memory.h"
#include <stdio.h>
#include <stdlib.h>

SimulationResult *run_fifo(
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

    SimulationResult *result = create_simulation_result("FIFO", reference_count, frame_count);
    if (!result) {
        return NULL;
    }

    /* Working array for resident page frames */
    int *current_frames = (int *)safe_malloc((size_t)frame_count * sizeof(int));
    if (!current_frames) {
        free_simulation_result(result);
        return NULL;
    }

    /* Initialize all frames to EMPTY_FRAME (-1) */
    for (int f = 0; f < frame_count; ++f) {
        current_frames[f] = EMPTY_FRAME;
    }

    /* Circular pointer indicating the oldest frame to be replaced */
    int fifo_index = 0;

    /* Process each page reference in original order */
    for (int i = 0; i < reference_count; ++i) {
        int current_page = references[i];
        result->steps[i].page = current_page;

        /* Check if the requested page is already resident in a frame */
        int hit_found = 0;
        for (int f = 0; f < frame_count; ++f) {
            if (current_frames[f] == current_page) {
                hit_found = 1;
                break;
            }
        }

        if (hit_found) {
            /* Page Hit: record hit; replacement order remains unchanged */
            result->steps[i].status = PAGE_HIT;
        } else {
            /* Page Fault: load/replace at current fifo_index and advance pointer */
            result->steps[i].status = PAGE_FAULT;
            current_frames[fifo_index] = current_page;
            fifo_index = (fifo_index + 1) % frame_count;
        }

        /* Snapshot current frame state into result step */
        for (int f = 0; f < frame_count; ++f) {
            result->steps[i].frames[f] = current_frames[f];
        }
    }

    free(current_frames);

    /* Update summary statistics (total hits, faults, ratios) */
    update_simulation_statistics(result);

    return result;
}
