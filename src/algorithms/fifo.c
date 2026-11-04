/**
 * @file fifo.c
 * @brief Implementation skeleton for First-In First-Out (FIFO) algorithm.
 *
 * Owned by Person 1.
 */

#include "algorithms/fifo.h"
#include "utils/memory.h"
#include <stdio.h>

SimulationResult *run_fifo(
    const int references[],
    int reference_count,
    int frame_count
) {
    if (!references || reference_count <= 0 || frame_count <= 0) {
        return NULL;
    }

    SimulationResult *result = create_simulation_result("FIFO", reference_count, frame_count);
    if (!result) {
        return NULL;
    }

    /*
     * TODO [Person 1]: Implement FIFO page replacement logic:
     * 1. Maintain an array of size frame_count for frames, initialized to EMPTY_FRAME (-1).
     * 2. Maintain a replacement pointer / queue index (e.g., fifo_index = 0).
     * 3. For each reference (i from 0 to reference_count - 1):
     *    a. Store result->steps[i].page = references[i].
     *    b. Check if references[i] is already in one of the frames.
     *    c. If present:
     *       - Hit: result->steps[i].status = PAGE_HIT
     *       - Do NOT modify the FIFO replacement index.
     *    d. If absent:
     *       - Fault: result->steps[i].status = PAGE_FAULT
     *       - Replace frame at fifo_index: frames[fifo_index] = references[i]
     *       - Advance fifo_index: fifo_index = (fifo_index + 1) % frame_count
     *    e. Copy current frame state into result->steps[i].frames.
     * 4. Call statistics module to compute total_hits, total_faults, hit_ratio, and fault_ratio.
     */

    return result;
}

