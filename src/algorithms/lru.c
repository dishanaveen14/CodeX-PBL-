/**
 * @file lru.c
 * @brief Implementation skeleton for Least Recently Used (LRU) algorithm.
 *
 * Owned by Person 1.
 */

#include "algorithms/lru.h"
#include "utils/memory.h"
#include <stdio.h>

SimulationResult *run_lru(
    const int references[],
    int reference_count,
    int frame_count
) {
    if (!references || reference_count <= 0 || frame_count <= 0) {
        return NULL;
    }

    SimulationResult *result = create_simulation_result("LRU", reference_count, frame_count);
    if (!result) {
        return NULL;
    }

    /*
     * TODO [Person 1]: Implement LRU page replacement logic:
     * 1. Maintain an array of size frame_count for frames, initialized to EMPTY_FRAME (-1).
     * 2. Maintain an array of size frame_count for last_used timestamps/indices.
     * 3. For each reference (i from 0 to reference_count - 1):
     *    a. Store result->steps[i].page = references[i].
     *    b. Check if references[i] is already in one of the frames.
     *    c. If present:
     *       - Hit: result->steps[i].status = PAGE_HIT
     *       - Update last_used timestamp for that frame (e.g. last_used[frame_idx] = i).
     *    d. If absent:
     *       - Fault: result->steps[i].status = PAGE_FAULT
     *       - If empty frame available, allocate that frame.
     *       - Else find frame with the minimum last_used timestamp (least recently used) and replace.
     *       - Update last_used timestamp for the replaced frame to i.
     *    e. Copy current frame state into result->steps[i].frames.
     * 4. Call statistics module to compute total_hits, total_faults, hit_ratio, and fault_ratio.
     */

    return result;
}

