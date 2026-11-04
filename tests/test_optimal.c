/**
 * @file test_optimal.c
 * @brief Comprehensive unit test suite for Optimal page replacement algorithm.
 *
 * Owned by Person 2.
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "algorithms/optimal.h"
#include "utils/memory.h"

/**
 * @brief Tests the standard project specification reference string:
 * Reference string: 7 0 1 2 0 3 0 4 2 3
 * Frames: 3
 *
 * Step-by-step trace:
 * Ref 0: page 7 -> Fault -> [7, -1, -1]
 * Ref 1: page 0 -> Fault -> [7,  0, -1]
 * Ref 2: page 1 -> Fault -> [7,  0,  1]
 * Ref 3: page 2 -> Fault -> [2,  0,  1] (7 is never referenced again, replace 7)
 * Ref 4: page 0 -> Hit   -> [2,  0,  1]
 * Ref 5: page 3 -> Fault -> [2,  0,  3] (1 is never referenced again, replace 1)
 * Ref 6: page 0 -> Hit   -> [2,  0,  3]
 * Ref 7: page 4 -> Fault -> [2,  4,  3] (0 is never referenced again, replace 0)
 * Ref 8: page 2 -> Hit   -> [2,  4,  3]
 * Ref 9: page 3 -> Hit   -> [2,  4,  3]
 *
 * Expected Results:
 * Total references: 10
 * Total hits: 4
 * Total faults: 6
 * Hit ratio: 40.0%
 * Fault ratio: 60.0%
 */
static void test_optimal_specification_sample(void) {
    printf("[TEST] Running test_optimal_specification_sample...\n");

    const int sample_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    int ref_count = 10;
    int frame_count = 3;

    SimulationResult *result = run_optimal(sample_refs, ref_count, frame_count);
    assert(result != NULL);
    assert(result->total_references == 10);
    assert(result->frame_count == 3);
    assert(result->total_hits == 4);
    assert(result->total_faults == 6);
    assert(fabs(result->hit_ratio - 40.0) < 0.001);
    assert(fabs(result->fault_ratio - 60.0) < 0.001);

    /* Verify step-by-step status */
    const PageStatus expected_status[] = {
        PAGE_FAULT, /* 7 */
        PAGE_FAULT, /* 0 */
        PAGE_FAULT, /* 1 */
        PAGE_FAULT, /* 2 */
        PAGE_HIT,   /* 0 */
        PAGE_FAULT, /* 3 */
        PAGE_HIT,   /* 0 */
        PAGE_FAULT, /* 4 */
        PAGE_HIT,   /* 2 */
        PAGE_HIT    /* 3 */
    };

    for (int i = 0; i < ref_count; ++i) {
        assert(result->steps[i].page == sample_refs[i]);
        assert(result->steps[i].status == expected_status[i]);
    }

    /* Verify specific frame transitions */
    /* Step 3 (page 2): 7 was replaced by 2 at frame 0 */
    assert(result->steps[3].frames[0] == 2);
    assert(result->steps[3].frames[1] == 0);
    assert(result->steps[3].frames[2] == 1);

    /* Step 5 (page 3): 1 was replaced by 3 at frame 2 */
    assert(result->steps[5].frames[0] == 2);
    assert(result->steps[5].frames[1] == 0);
    assert(result->steps[5].frames[2] == 3);

    /* Step 7 (page 4): 0 was replaced by 4 at frame 1 */
    assert(result->steps[7].frames[0] == 2);
    assert(result->steps[7].frames[1] == 4);
    assert(result->steps[7].frames[2] == 3);

    free_simulation_result(result);
    printf("[PASS] test_optimal_specification_sample passed.\n");
}

/**
 * @brief Tests priority eviction of pages never referenced again in the future.
 */
static void test_optimal_unreferenced_future_page(void) {
    printf("[TEST] Running test_optimal_unreferenced_future_page...\n");

    /* References: 1, 2, 3, 4, 2, 3 */
    /* Frames: 3 */
    /* At ref 3 (page 4), resident pages are 1, 2, 3. Remaining future: 2, 3. */
    /* Page 1 is never used again -> must be replaced with page 4. */
    const int refs[] = {1, 2, 3, 4, 2, 3};
    SimulationResult *result = run_optimal(refs, 6, 3);
    assert(result != NULL);
    assert(result->total_references == 6);
    assert(result->total_faults == 4);
    assert(result->total_hits == 2);

    /* Check step 3 (page 4): frame 0 was 1, should now be 4 */
    assert(result->steps[3].frames[0] == 4);
    assert(result->steps[3].frames[1] == 2);
    assert(result->steps[3].frames[2] == 3);
    assert(result->steps[4].status == PAGE_HIT); /* page 2 */
    assert(result->steps[5].status == PAGE_HIT); /* page 3 */

    free_simulation_result(result);
    printf("[PASS] test_optimal_unreferenced_future_page passed.\n");
}

/**
 * @brief Tests eviction when all pages are used in future, selecting the farthest.
 */
static void test_optimal_farthest_future_page(void) {
    printf("[TEST] Running test_optimal_farthest_future_page...\n");

    /* References: 1, 2, 3, 4, 1, 2, 3 */
    /* Frames: 3 */
    /* At ref 3 (page 4), resident: 1, 2, 3. Future uses: 1 (idx 4), 2 (idx 5), 3 (idx 6). */
    /* Page 3 is farthest in the future -> replace 3 at frame 2 */
    const int refs[] = {1, 2, 3, 4, 1, 2, 3};
    SimulationResult *result = run_optimal(refs, 7, 3);
    assert(result != NULL);
    assert(result->steps[3].frames[0] == 1);
    assert(result->steps[3].frames[1] == 2);
    assert(result->steps[3].frames[2] == 4); /* Page 3 was replaced */
    assert(result->steps[4].status == PAGE_HIT); /* Page 1 hits */
    assert(result->steps[5].status == PAGE_HIT); /* Page 2 hits */

    free_simulation_result(result);
    printf("[PASS] test_optimal_farthest_future_page passed.\n");
}

/**
 * @brief Tests edge case: 1 frame only.
 */
static void test_optimal_single_frame(void) {
    printf("[TEST] Running test_optimal_single_frame...\n");

    const int refs[] = {1, 2, 3, 2, 1};
    SimulationResult *result = run_optimal(refs, 5, 1);
    assert(result != NULL);
    assert(result->total_references == 5);
    assert(result->total_faults == 5);
    assert(result->total_hits == 0);
    assert(result->hit_ratio == 0.0);
    assert(result->fault_ratio == 100.0);

    for (int i = 0; i < 5; ++i) {
        assert(result->steps[i].frames[0] == refs[i]);
        assert(result->steps[i].status == PAGE_FAULT);
    }

    free_simulation_result(result);
    printf("[PASS] test_optimal_single_frame passed.\n");
}

/**
 * @brief Tests repeated consecutive references to the same page.
 */
static void test_optimal_repeated_references(void) {
    printf("[TEST] Running test_optimal_repeated_references...\n");

    const int refs[] = {5, 5, 5, 5, 5};
    SimulationResult *result = run_optimal(refs, 5, 3);
    assert(result != NULL);
    assert(result->total_faults == 1);
    assert(result->total_hits == 4);
    assert(fabs(result->hit_ratio - 80.0) < 0.001);
    assert(fabs(result->fault_ratio - 20.0) < 0.001);

    free_simulation_result(result);
    printf("[PASS] test_optimal_repeated_references passed.\n");
}

/**
 * @brief Tests frames count greater than number of distinct pages.
 */
static void test_optimal_more_frames_than_pages(void) {
    printf("[TEST] Running test_optimal_more_frames_than_pages...\n");

    const int refs[] = {1, 2, 3, 1, 2, 3};
    SimulationResult *result = run_optimal(refs, 6, 5);
    assert(result != NULL);
    assert(result->total_faults == 3);
    assert(result->total_hits == 3);
    assert(fabs(result->hit_ratio - 50.0) < 0.001);
    assert(fabs(result->fault_ratio - 50.0) < 0.001);

    /* Verify empty frames remain (-1) */
    assert(result->steps[5].frames[0] == 1);
    assert(result->steps[5].frames[1] == 2);
    assert(result->steps[5].frames[2] == 3);
    assert(result->steps[5].frames[3] == EMPTY_FRAME);
    assert(result->steps[5].frames[4] == EMPTY_FRAME);

    free_simulation_result(result);
    printf("[PASS] test_optimal_more_frames_than_pages passed.\n");
}

/**
 * @brief Tests invalid inputs and boundary validations.
 */
static void test_optimal_invalid_inputs(void) {
    printf("[TEST] Running test_optimal_invalid_inputs...\n");

    const int valid_refs[] = {1, 2, 3};
    const int negative_refs[] = {1, -2, 3};

    assert(run_optimal(NULL, 3, 3) == NULL);
    assert(run_optimal(valid_refs, 0, 3) == NULL);
    assert(run_optimal(valid_refs, -1, 3) == NULL);
    assert(run_optimal(valid_refs, 3, 0) == NULL);
    assert(run_optimal(valid_refs, 3, -2) == NULL);
    assert(run_optimal(negative_refs, 3, 3) == NULL);

    printf("[PASS] test_optimal_invalid_inputs passed.\n");
}

int main(void) {
    printf("==========================================\n");
    printf("   Optimal Algorithm Unit Tests (Person 2)\n");
    printf("==========================================\n");

    test_optimal_invalid_inputs();
    test_optimal_specification_sample();
    test_optimal_unreferenced_future_page();
    test_optimal_farthest_future_page();
    test_optimal_single_frame();
    test_optimal_repeated_references();
    test_optimal_more_frames_than_pages();

    printf("\nAll Person 2 Optimal unit tests passed successfully!\n\n");
    return 0;
}
