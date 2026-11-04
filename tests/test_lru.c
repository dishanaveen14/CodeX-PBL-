/**
 * @file test_lru.c
 * @brief Comprehensive unit test suite for Least Recently Used (LRU) algorithm.
 *
 * Owned by Person 1.
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "algorithms/lru.h"
#include "utils/memory.h"

/**
 * @brief Tests the project specification sample reference string under LRU:
 * Reference string: 7 0 1 2 0 3 0 4 2 3
 * Frames: 3
 *
 * Step-by-step trace:
 * Step 0: page 7 -> [7, ---, ---] Fault
 * Step 1: page 0 -> [7,   0, ---] Fault
 * Step 2: page 1 -> [7,   0,   1] Fault
 * Step 3: page 2 -> [2,   0,   1] Fault (7 is least recently used, evict frame 0)
 * Step 4: page 0 -> [2,   0,   1] Hit   (0 recency updated)
 * Step 5: page 3 -> [2,   0,   3] Fault (1 is least recently used, evict frame 2)
 * Step 6: page 0 -> [2,   0,   3] Hit   (0 recency updated)
 * Step 7: page 4 -> [4,   0,   3] Fault (2 is least recently used, evict frame 0)
 * Step 8: page 2 -> [4,   0,   2] Fault (3 is least recently used, evict frame 2)
 * Step 9: page 3 -> [4,   3,   2] Fault (0 is least recently used, evict frame 1)
 *
 * Summary:
 * Total references: 10
 * LRU hits: 2
 * LRU faults: 8
 * LRU hit ratio: 20.0%
 * LRU fault ratio: 80.0%
 */
static void test_lru_specification_sample(void) {
    printf("[TEST] Running test_lru_specification_sample...\n");

    const int sample_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    int ref_count = 10;
    int frame_count = 3;

    SimulationResult *result = run_lru(sample_refs, ref_count, frame_count);
    assert(result != NULL);
    assert(result->total_references == 10);
    assert(result->frame_count == 3);
    assert(result->total_hits == 2);
    assert(result->total_faults == 8);
    assert(fabs(result->hit_ratio - 20.0) < 0.001);
    assert(fabs(result->fault_ratio - 80.0) < 0.001);

    const int expected_frames[10][3] = {
        {7, EMPTY_FRAME, EMPTY_FRAME}, /* Step 0 */
        {7, 0,           EMPTY_FRAME}, /* Step 1 */
        {7, 0,           1},           /* Step 2 */
        {2, 0,           1},           /* Step 3 */
        {2, 0,           1},           /* Step 4: Hit */
        {2, 0,           3},           /* Step 5 */
        {2, 0,           3},           /* Step 6: Hit */
        {4, 0,           3},           /* Step 7 */
        {4, 0,           2},           /* Step 8 */
        {4, 3,           2}            /* Step 9 */
    };

    const PageStatus expected_status[10] = {
        PAGE_FAULT, /* 7 */
        PAGE_FAULT, /* 0 */
        PAGE_FAULT, /* 1 */
        PAGE_FAULT, /* 2 */
        PAGE_HIT,   /* 0 */
        PAGE_FAULT, /* 3 */
        PAGE_HIT,   /* 0 */
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
    printf("[PASS] test_lru_specification_sample passed.\n");
}

/**
 * @brief Tests recency update on hits (contrasting LRU behavior with FIFO).
 */
static void test_lru_recency_update_on_hit(void) {
    printf("[TEST] Running test_lru_recency_update_on_hit...\n");

    /* References: 1, 2, 3, 1, 4 */
    /* Step 0: 1 -> [1, -, -] */
    /* Step 1: 2 -> [1, 2, -] */
    /* Step 2: 3 -> [1, 2, 3] */
    /* Step 3: 1 -> [1, 2, 3] Hit (recency of 1 updated) */
    /* Step 4: 4 -> LRU is 2 (frame 1), NOT 1 (frame 0) */
    /* Resulting frames: [1, 4, 3] */
    const int refs[] = {1, 2, 3, 1, 4};
    SimulationResult *result = run_lru(refs, 5, 3);
    assert(result != NULL);
    assert(result->total_hits == 1);
    assert(result->total_faults == 4);

    assert(result->steps[3].status == PAGE_HIT);
    assert(result->steps[4].status == PAGE_FAULT);

    assert(result->steps[4].frames[0] == 1);
    assert(result->steps[4].frames[1] == 4); /* Page 2 was evicted */
    assert(result->steps[4].frames[2] == 3);

    free_simulation_result(result);
    printf("[PASS] test_lru_recency_update_on_hit passed.\n");
}

/**
 * @brief Tests reference string with no repeated pages.
 */
static void test_lru_no_repeated_pages(void) {
    printf("[TEST] Running test_lru_no_repeated_pages...\n");

    const int refs[] = {1, 2, 3, 4, 5};
    SimulationResult *result = run_lru(refs, 5, 3);
    assert(result != NULL);
    assert(result->total_faults == 5);
    assert(result->total_hits == 0);

    /* Frame transitions: [1,2,3] -> [4,2,3] -> [4,5,3] */
    assert(result->steps[4].frames[0] == 4);
    assert(result->steps[4].frames[1] == 5);
    assert(result->steps[4].frames[2] == 3);

    free_simulation_result(result);
    printf("[PASS] test_lru_no_repeated_pages passed.\n");
}

/**
 * @brief Tests repeated pages and consecutive hits.
 */
static void test_lru_repeated_pages_hits(void) {
    printf("[TEST] Running test_lru_repeated_pages_hits...\n");

    const int refs[] = {2, 2, 2, 2, 2};
    SimulationResult *result = run_lru(refs, 5, 3);
    assert(result != NULL);
    assert(result->total_faults == 1);
    assert(result->total_hits == 4);
    assert(fabs(result->hit_ratio - 80.0) < 0.001);
    assert(fabs(result->fault_ratio - 20.0) < 0.001);

    free_simulation_result(result);
    printf("[PASS] test_lru_repeated_pages_hits passed.\n");
}

/**
 * @brief Tests single frame environment (frame_count = 1).
 */
static void test_lru_single_frame(void) {
    printf("[TEST] Running test_lru_single_frame...\n");

    const int refs[] = {1, 2, 3, 2, 1};
    SimulationResult *result = run_lru(refs, 5, 1);
    assert(result != NULL);
    assert(result->total_faults == 5);
    assert(result->total_hits == 0);

    for (int i = 0; i < 5; ++i) {
        assert(result->steps[i].frames[0] == refs[i]);
    }

    free_simulation_result(result);
    printf("[PASS] test_lru_single_frame passed.\n");
}

/**
 * @brief Tests more frames than distinct pages.
 */
static void test_lru_more_frames_than_pages(void) {
    printf("[TEST] Running test_lru_more_frames_than_pages...\n");

    const int refs[] = {1, 2, 3, 1, 2, 3};
    SimulationResult *result = run_lru(refs, 6, 5);
    assert(result != NULL);
    assert(result->total_faults == 3);
    assert(result->total_hits == 3);
    assert(fabs(result->hit_ratio - 50.0) < 0.001);
    assert(fabs(result->fault_ratio - 50.0) < 0.001);

    free_simulation_result(result);
    printf("[PASS] test_lru_more_frames_than_pages passed.\n");
}

/**
 * @brief Tests invalid inputs and boundary conditions.
 */
static void test_lru_invalid_inputs(void) {
    printf("[TEST] Running test_lru_invalid_inputs...\n");

    const int valid_refs[] = {1, 2, 3};
    const int negative_refs[] = {1, -1, 3};

    assert(run_lru(NULL, 3, 3) == NULL);
    assert(run_lru(valid_refs, 0, 3) == NULL);
    assert(run_lru(valid_refs, -1, 3) == NULL);
    assert(run_lru(valid_refs, 3, 0) == NULL);
    assert(run_lru(valid_refs, 3, -1) == NULL);
    assert(run_lru(negative_refs, 3, 3) == NULL);

    printf("[PASS] test_lru_invalid_inputs passed.\n");
}

int main(void) {
    printf("==========================================\n");
    printf("       LRU Algorithm Unit Tests (Person 1)\n");
    printf("==========================================\n");

    test_lru_invalid_inputs();
    test_lru_specification_sample();
    test_lru_recency_update_on_hit();
    test_lru_no_repeated_pages();
    test_lru_repeated_pages_hits();
    test_lru_single_frame();
    test_lru_more_frames_than_pages();

    printf("\nAll Person 1 LRU unit tests passed successfully!\n\n");
    return 0;
}
