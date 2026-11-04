/**
 * @file test_lru.c
 * @brief Unit test skeleton for Least Recently Used (LRU) algorithm.
 *
 * Owned by Person 1.
 */

#include <stdio.h>
#include <assert.h>
#include "algorithms/lru.h"
#include "utils/memory.h"

/**
 * @brief Test basic LRU execution interface and structure allocation.
 */
static void test_lru_interface(void) {
    printf("[TEST] Running test_lru_interface...\n");

    const int sample_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    int ref_count = 10;
    int frame_count = 3;

    SimulationResult *result = run_lru(sample_refs, ref_count, frame_count);
    assert(result != NULL);
    assert(result->total_references == ref_count);
    assert(result->frame_count == frame_count);
    assert(result->steps != NULL);

    /*
     * TODO [Person 1]: After implementing LRU algorithm, assert:
     * - result step frame contents and correct victim replacement.
     * - total_hits + total_faults == total_references.
     */

    free_simulation_result(result);
    printf("[PASS] test_lru_interface passed.\n");
}

/**
 * @brief Test LRU invalid arguments handling.
 */
static void test_lru_invalid_inputs(void) {
    printf("[TEST] Running test_lru_invalid_inputs...\n");

    const int sample_refs[] = {1, 2, 3};
    assert(run_lru(NULL, 3, 3) == NULL);
    assert(run_lru(sample_refs, 0, 3) == NULL);
    assert(run_lru(sample_refs, -1, 3) == NULL);
    assert(run_lru(sample_refs, 3, 0) == NULL);
    assert(run_lru(sample_refs, 3, -1) == NULL);

    printf("[PASS] test_lru_invalid_inputs passed.\n");
}

int main(void) {
    printf("=== LRU Unit Tests (Person 1) ===\n");
    test_lru_invalid_inputs();
    test_lru_interface();
    printf("All LRU skeleton tests passed successfully!\n\n");
    return 0;
}

