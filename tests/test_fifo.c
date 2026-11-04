/**
 * @file test_fifo.c
 * @brief Unit test skeleton for First-In First-Out (FIFO) algorithm.
 *
 * Owned by Person 1.
 */

#include <stdio.h>
#include <assert.h>
#include "algorithms/fifo.h"
#include "utils/memory.h"

/**
 * @brief Test basic FIFO execution interface and structure allocation.
 */
static void test_fifo_interface(void) {
    printf("[TEST] Running test_fifo_interface...\n");

    const int sample_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    int ref_count = 10;
    int frame_count = 3;

    SimulationResult *result = run_fifo(sample_refs, ref_count, frame_count);
    assert(result != NULL);
    assert(result->total_references == ref_count);
    assert(result->frame_count == frame_count);
    assert(result->steps != NULL);

    /*
     * TODO [Person 1]: After implementing FIFO algorithm, assert:
     * - result->total_hits == 1
     * - result->total_faults == 9
     * - Check intermediate frame states for each step against specification.
     */

    free_simulation_result(result);
    printf("[PASS] test_fifo_interface passed.\n");
}

/**
 * @brief Test FIFO invalid arguments handling.
 */
static void test_fifo_invalid_inputs(void) {
    printf("[TEST] Running test_fifo_invalid_inputs...\n");

    const int sample_refs[] = {1, 2, 3};
    assert(run_fifo(NULL, 3, 3) == NULL);
    assert(run_fifo(sample_refs, 0, 3) == NULL);
    assert(run_fifo(sample_refs, -1, 3) == NULL);
    assert(run_fifo(sample_refs, 3, 0) == NULL);
    assert(run_fifo(sample_refs, 3, -1) == NULL);

    printf("[PASS] test_fifo_invalid_inputs passed.\n");
}

int main(void) {
    printf("=== FIFO Unit Tests (Person 1) ===\n");
    test_fifo_invalid_inputs();
    test_fifo_interface();
    printf("All FIFO skeleton tests passed successfully!\n\n");
    return 0;
}

