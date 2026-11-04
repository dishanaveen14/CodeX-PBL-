/**
 * @file optimal.c
 * @brief Implementation skeleton for Optimal page replacement algorithm.
 *
 * Owned by Person 2.
 */

#include "algorithms/optimal.h"
#include "utils/memory.h"
#include <stdio.h>

SimulationResult *run_optimal(
    const int references[],
    int reference_count,
    int frame_count
) {
    if (!references || reference_count <= 0 || frame_count <= 0) {
        return NULL;
    }

    SimulationResult *result = create_simulation_result("Optimal", reference_count, frame_count);
    if (!result) {
        return NULL;
    }

    /*
     * TODO [Person 2]: Implement Optimal page replacement logic:
     * 1. Maintain an array of size frame_count for frames, initialized to EMPTY_FRAME (-1).
     * 2. For each reference (i from 0 to reference_count - 1):
     *    a. Store result->steps[i].page = references[i].
     *    b. Check if references[i] is already in one of the frames.
     *    c. If present:
     *       - Hit: result->steps[i].status = PAGE_HIT
     *    d. If absent:
     *       - Fault: result->steps[i].status = PAGE_FAULT
     *       - If empty frame available, allocate that frame.
     *       - Else find frame whose page will not be used for the longest time in the future:
     *         * Scan future references (from index i + 1 to reference_count - 1).
     *         * If a page in memory is never referenced again, it has infinite distance (prioritize replacing it).
     *         * Otherwise, replace the page with the maximum next-use index.
     *    e. Copy current frame state into result->steps[i].frames.
     * 3. Call statistics module to compute total_hits, total_faults, hit_ratio, and fault_ratio.
     */

    return result;
}

