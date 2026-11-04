/**
 * @file test_fifo.c
 * @brief Comprehensive unit test suite for First-In First-Out (FIFO) algorithm.
 *
 * Owned by Person 1.
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "algorithms/fifo.h"
#include "utils/memory.h"

/**
 * @brief Tests the project specification sample reference string:
 * Reference string: 7 0 1 2 0 3 0 4 2 3
 * Frames: 3
 *
 * Progression from specification (page_replacement_implementation_plan.md lines 1004-1025):
 * Step 0: page 7 -> [7, ---, ---] Fault
 * Step 1: page 0 -> [7,   0, ---] Fault
 * Step 2: page 1 -> [7,   0,   1] Fault
 * Step 3: page 2 -> [2,   0,   1] Fault (replaces 7)
 * Step 4: page 0 -> [2,   0,   1] Hit
 * Step 5: page 3 -> [2,   3,   1] Fault (replaces 0)
 * Step 6: page 0 -> [2,   3,   0] Fault (replaces 1)
 * Step 7: page 4 -> [4,   3,   0] Fault (replaces 2)
 * Step 8: page 2 -> [4,   2,   0] Fault (replaces 3)
 * Step 9: page 3 -> [4,   2,   3] Fault (replaces 0)
 *
 * Expected Summary:
 * Total references: 10
 * FIFO hits: 1
 * FIFO faults: 9
 * FIFO hit ratio: 10.0%
 * FIFO fault ratio: 90.0%
 */
static void test_fifo_specification_sample(void) {
    printf("[TEST] Running test_fifo_specification_sample...\n");

    const int sample_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    int ref_count = 10;
    int frame_count = 3;

    SimulationResult *result = run_fifo(sample_refs, ref_count, frame_count);
    assert(result != NULL);
    assert(result->total_references == 10);
    assert(result->frame_count == 3);
    assert(result->total_hits == 1);
    assert(result->total_faults == 9);
    assert(fabs(result->hit_ratio - 10.0) < 0.001);
    assert(fabs(result->fault_ratio - 90.0) < 0.001);

    const int expected_frames[10][3] = {
        {7, EMPTY_FRAME, EMPTY_FRAME}, /* Step 0 */
        {7, 0,           EMPTY_FRAME}, /* Step 1 */
        {7, 0,           1},           /* Step 2 */
        {2, 0,           1},           /* Step 3 */
        {2, 0,           1},           /* Step 4 */
        {2, 3,           1},           /* Step 5 */
        {2, 3,           0},           /* Step 6 */
        {4, 3,           0},           /* Step 7 */
        {4, 2,           0},           /* Step 8 */
        {4, 2,           3}            /* Step 9 */
    };

    const PageStatus expected_status[10] = {
        PAGE_FAULT, /* 7 */
        PAGE_FAULT, /* 0 */
        PAGE_FAULT, /* 1 */
        PAGE_FAULT, /* 2 */
        PAGE_HIT,   /* 0 */
        PAGE_FAULT, /* 3 */
        PAGE_FAULT, /* 0 */
        PAGE_FAULT, /* 4 */
        PAGE_FAULT, /* 2 */
        PAGE_FAULT  /* 3 */
    };

    for (int i = 0; i < ref_count; ++i) {
        assert(result->steps[i].page == sample_refs[i]);
        assert(result->steps[i].status == expected_status[i]);
        for (int f = 0; f < frame_count; ++f) {
            assert(result->steps[i].frames[f] == expected_frames[i][f]);
        }
    }

    free_simulation_result(result);
    printf("[PASS] test_fifo_specification_sample passed.\n");
}

/**
 * @brief Tests reference string with no repeated pages.
 */
static void test_fifo_no_repeated_pages(void) {
    printf("[TEST] Running test_fifo_no_repeated_pages...\n");

    const int refs[] = {1, 2, 3, 4, 5};
    SimulationResult *result = run_fifo(refs, 5, 3);
    assert(result != NULL);
    assert(result->total_references == 5);
    assert(result->total_faults == 5);
    assert(result->total_hits == 0);
    assert(result->hit_ratio == 0.0);
    assert(result->fault_ratio == 100.0);

    /* Check final frames: 1 was replaced by 4, 2 was replaced by 5 -> [4, 5, 3] */
    assert(result->steps[4].frames[0] == 4);
    assert(result->steps[4].frames[1] == 5);
    assert(result->steps[4].frames[2] == 3);

    free_simulation_result(result);
    printf("[PASS] test_fifo_no_repeated_pages passed.\n");
}

/**
 * @brief Tests repeated pages and consecutive hits.
 */
static void test_fifo_repeated_pages_hits(void) {
    printf("[TEST] Running test_fifo_repeated_pages_hits...\n");

    const int refs[] = {1, 1, 1, 1, 1};
    SimulationResult *result = run_fifo(refs, 5, 3);
    assert(result != NULL);
    assert(result->total_faults == 1);
    assert(result->total_hits == 4);
    assert(fabs(result->hit_ratio - 80.0) < 0.001);
    assert(fabs(result->fault_ratio - 20.0) < 0.001);

    free_simulation_result(result);
    printf("[PASS] test_fifo_repeated_pages_hits passed.\n");
}

/**
 * @brief Tests single frame environment (frame_count = 1).
 */
static void test_fifo_single_frame(void) {
    printf("[TEST] Running test_fifo_single_frame...\n");

    const int refs[] = {1, 2, 3, 2, 1};
    SimulationResult *result = run_fifo(refs, 5, 1);
    assert(result != NULL);
    assert(result->total_faults == 5);
    assert(result->total_hits == 0);

    for (int i = 0; i < 5; ++i) {
        assert(result->steps[i].frames[0] == refs[i]);
    }

    free_simulation_result(result);
    printf("[PASS] test_fifo_single_frame passed.\n");
}

/**
 * @brief Tests more frames than distinct pages.
 */
static void test_fifo_more_frames_than_pages(void) {
    printf("[TEST] Running test_fifo_more_frames_than_pages...\n");

    const int refs[] = {1, 2, 3, 1, 2, 3};
    SimulationResult *result = run_fifo(refs, 6, 5);
    assert(result != NULL);
    assert(result->total_faults == 3);
    assert(result->total_hits == 3);
    assert(fabs(result->hit_ratio - 50.0) < 0.001);
    assert(fabs(result->fault_ratio - 50.0) < 0.001);

    free_simulation_result(result);
    printf("[PASS] test_fifo_more_frames_than_pages passed.\n");
}

/**
 * @brief Tests invalid inputs and boundary conditions.
 */
static void test_fifo_invalid_inputs(void) {
    printf("[TEST] Running test_fifo_invalid_inputs...\n");

    const int valid_refs[] = {1, 2, 3};
    const int negative_refs[] = {1, -1, 3};

    assert(run_fifo(NULL, 3, 3) == NULL);
    assert(run_fifo(valid_refs, 0, 3) == NULL);
    assert(run_fifo(valid_refs, -1, 3) == NULL);
    assert(run_fifo(valid_refs, 3, 0) == NULL);
    assert(run_fifo(valid_refs, 3, -1) == NULL);
    assert(run_fifo(negative_refs, 3, 3) == NULL);

    printf("[PASS] test_fifo_invalid_inputs passed.\n");
}

int main(void) {
    printf("==========================================\n");
    printf("      FIFO Algorithm Unit Tests (Person 1)\n");
    printf("==========================================\n");

    test_fifo_invalid_inputs();
    test_fifo_specification_sample();
    test_fifo_no_repeated_pages();
    test_fifo_repeated_pages_hits();
    test_fifo_single_frame();
    test_fifo_more_frames_than_pages();

    printf("\nAll Person 1 FIFO unit tests passed successfully!\n\n");
    return 0;
}
