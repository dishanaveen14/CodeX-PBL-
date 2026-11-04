/**
 * @file test_optimal.c
 * @brief Unit test skeleton for Optimal page replacement algorithm.
 *
 * Owned by Person 2.
 */

#include <stdio.h>
#include <assert.h>
#include "algorithms/optimal.h"
#include "utils/memory.h"

/**
 * @brief Test basic Optimal execution interface and structure allocation.
 */
static void test_optimal_interface(void) {
    printf("[TEST] Running test_optimal_interface...\n");

    const int sample_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    int ref_count = 10;
    int frame_count = 3;

    SimulationResult *result = run_optimal(sample_refs, ref_count, frame_count);
    assert(result != NULL);
    assert(result->total_references == ref_count);
    assert(result->frame_count == frame_count);
    assert(result->steps != NULL);

    /*
     * TODO [Person 2]: After implementing Optimal algorithm, assert:
     * - Victim selection looks strictly forward in the reference string.
     * - Unreferenced pages in memory are preferred for replacement.
     * - total_hits + total_faults == total_references.
     */

    free_simulation_result(result);
    printf("[PASS] test_optimal_interface passed.\n");
}

/**
 * @brief Test Optimal invalid arguments handling.
 */
static void test_optimal_invalid_inputs(void) {
    printf("[TEST] Running test_optimal_invalid_inputs...\n");

    const int sample_refs[] = {1, 2, 3};
    assert(run_optimal(NULL, 3, 3) == NULL);
    assert(run_optimal(sample_refs, 0, 3) == NULL);
    assert(run_optimal(sample_refs, -1, 3) == NULL);
    assert(run_optimal(sample_refs, 3, 0) == NULL);
    assert(run_optimal(sample_refs, 3, -1) == NULL);

    printf("[PASS] test_optimal_invalid_inputs passed.\n");
}

int main(void) {
    printf("=== Optimal Unit Tests (Person 2) ===\n");
    test_optimal_invalid_inputs();
    test_optimal_interface();
    printf("All Optimal skeleton tests passed successfully!\n\n");
    return 0;
}

