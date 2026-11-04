/**
 * @file test_integration.c
 * @brief Comprehensive end-to-end integration test suite.
 *
 * Owned by Person 4 (Integration).
 * Verifies end-to-end dataflow across:
 * Input Validation -> Simulation Engine -> FIFO / LRU / Optimal -> Statistics -> Memory Cleanup.
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "common.h"
#include "core/simulation.h"
#include "gui/input_validation.h"
#include "utils/memory.h"

/**
 * @brief End-to-end test of the project specification sample input with Compare All:
 * Reference string: 7 0 1 2 0 3 0 4 2 3
 * Frames: 3
 *
 * Verified Expectations:
 * - FIFO:    Hits: 1, Faults: 9, Hit Ratio: 10.0%, Fault Ratio: 90.0%
 * - LRU:     Hits: 2, Faults: 8, Hit Ratio: 20.0%, Fault Ratio: 80.0%
 * - Optimal: Hits: 4, Faults: 6, Hit Ratio: 40.0%, Fault Ratio: 60.0%
 */
static void test_e2e_specification_compare_all(void) {
    printf("[TEST] Running test_e2e_specification_compare_all...\n");

    const char *raw_input = "7 0 1 2 0 3 0 4 2 3";
    int frame_count = 3;

    /* 1. Validate frame count */
    assert(validate_frame_count(frame_count) == VALIDATION_SUCCESS);

    /* 2. Parse and validate reference string */
    int *references = NULL;
    int ref_count = 0;
    ValidationStatus status = parse_and_validate_reference_string(raw_input, &references, &ref_count);
    assert(status == VALIDATION_SUCCESS);
    assert(ref_count == 10);
    assert(references != NULL);

    /* 3. Dispatch Compare All simulation */
    int result_count = 0;
    SimulationResult **results = run_simulation(references, ref_count, frame_count, ALGORITHM_ALL, &result_count);
    assert(results != NULL);
    assert(result_count == 3);

    /* --- Verify FIFO Results (Index 0) --- */
    SimulationResult *fifo_res = results[0];
    assert(fifo_res != NULL);
    assert(strcmp(fifo_res->algorithm_name, "FIFO") == 0);
    assert(fifo_res->total_references == 10);
    assert(fifo_res->total_hits == 1);
    assert(fifo_res->total_faults == 9);
    assert(fabs(fifo_res->hit_ratio - 10.0) < 0.001);
    assert(fabs(fifo_res->fault_ratio - 90.0) < 0.001);

    /* --- Verify LRU Results (Index 1) --- */
    SimulationResult *lru_res = results[1];
    assert(lru_res != NULL);
    assert(strcmp(lru_res->algorithm_name, "LRU") == 0);
    assert(lru_res->total_references == 10);
    assert(lru_res->total_hits == 2);
    assert(lru_res->total_faults == 8);
    assert(fabs(lru_res->hit_ratio - 20.0) < 0.001);
    assert(fabs(lru_res->fault_ratio - 80.0) < 0.001);

    /* --- Verify Optimal Results (Index 2) --- */
    SimulationResult *opt_res = results[2];
    assert(opt_res != NULL);
    assert(strcmp(opt_res->algorithm_name, "Optimal") == 0);
    assert(opt_res->total_references == 10);
    assert(opt_res->total_hits == 4);
    assert(opt_res->total_faults == 6);
    assert(fabs(opt_res->hit_ratio - 40.0) < 0.001);
    assert(fabs(opt_res->fault_ratio - 60.0) < 0.001);

    /* Verify universal invariants for all algorithms */
    for (int r = 0; r < result_count; ++r) {
        assert(results[r]->total_hits + results[r]->total_faults == results[r]->total_references);
        assert(fabs((results[r]->hit_ratio + results[r]->fault_ratio) - 100.0) < 0.001);
    }

    /* 4. Release allocated memory */
    free_parsed_references(references);
    free_simulation_results(results, result_count);

    printf("[PASS] test_e2e_specification_compare_all passed.\n");
}

/**
 * @brief Tests individual algorithm selection pipelines.
 */
static void test_individual_algorithm_dispatches(void) {
    printf("[TEST] Running test_individual_algorithm_dispatches...\n");

    const char *raw_input = "7 0 1 2 0 3 0 4 2 3";
    int *refs = NULL;
    int ref_count = 0;
    assert(parse_and_validate_reference_string(raw_input, &refs, &ref_count) == VALIDATION_SUCCESS);

    int result_count = 0;

    /* FIFO individual dispatch */
    SimulationResult **fifo_out = run_simulation(refs, ref_count, 3, ALGORITHM_FIFO, &result_count);
    assert(fifo_out != NULL && result_count == 1);
    assert(strcmp(fifo_out[0]->algorithm_name, "FIFO") == 0);
    assert(fifo_out[0]->total_hits == 1 && fifo_out[0]->total_faults == 9);
    free_simulation_results(fifo_out, result_count);

    /* LRU individual dispatch */
    SimulationResult **lru_out = run_simulation(refs, ref_count, 3, ALGORITHM_LRU, &result_count);
    assert(lru_out != NULL && result_count == 1);
    assert(strcmp(lru_out[0]->algorithm_name, "LRU") == 0);
    assert(lru_out[0]->total_hits == 2 && lru_out[0]->total_faults == 8);
    free_simulation_results(lru_out, result_count);

    /* Optimal individual dispatch */
    SimulationResult **opt_out = run_simulation(refs, ref_count, 3, ALGORITHM_OPTIMAL, &result_count);
    assert(opt_out != NULL && result_count == 1);
    assert(strcmp(opt_out[0]->algorithm_name, "Optimal") == 0);
    assert(opt_out[0]->total_hits == 4 && opt_out[0]->total_faults == 6);
    free_simulation_results(opt_out, result_count);

    free_parsed_references(refs);
    printf("[PASS] test_individual_algorithm_dispatches passed.\n");
}

/**
 * @brief Tests comma-separated and mixed whitespace formatting in input string.
 */
static void test_comma_delimited_input_pipeline(void) {
    printf("[TEST] Running test_comma_delimited_input_pipeline...\n");

    const char *comma_input = "  7, 0, 1,2,  0, 3, 0, 4, 2, 3  ";
    int *refs = NULL;
    int count = 0;

    assert(parse_and_validate_reference_string(comma_input, &refs, &count) == VALIDATION_SUCCESS);
    assert(count == 10);

    int res_count = 0;
    SimulationResult **results = run_simulation(refs, count, 3, ALGORITHM_OPTIMAL, &res_count);
    assert(results != NULL && res_count == 1);
    assert(results[0]->total_hits == 4);
    assert(results[0]->total_faults == 6);

    free_parsed_references(refs);
    free_simulation_results(results, res_count);

    printf("[PASS] test_comma_delimited_input_pipeline passed.\n");
}

/**
 * @brief Tests boundary condition: single reference.
 */
static void test_single_reference_pipeline(void) {
    printf("[TEST] Running test_single_reference_pipeline...\n");

    int *refs = NULL;
    int count = 0;
    assert(parse_and_validate_reference_string("42", &refs, &count) == VALIDATION_SUCCESS);
    assert(count == 1);

    int res_count = 0;
    SimulationResult **results = run_simulation(refs, count, 3, ALGORITHM_ALL, &res_count);
    assert(results != NULL && res_count == 3);

    for (int r = 0; r < 3; ++r) {
        assert(results[r]->total_references == 1);
        assert(results[r]->total_hits == 0);
        assert(results[r]->total_faults == 1);
        assert(results[r]->hit_ratio == 0.0);
        assert(results[r]->fault_ratio == 100.0);
        assert(results[r]->steps[0].frames[0] == 42);
        assert(results[r]->steps[0].frames[1] == EMPTY_FRAME);
        assert(results[r]->steps[0].frames[2] == EMPTY_FRAME);
    }

    free_parsed_references(refs);
    free_simulation_results(results, res_count);

    printf("[PASS] test_single_reference_pipeline passed.\n");
}

/**
 * @brief Tests input validation error handling across edge cases.
 */
static void test_validation_rejections(void) {
    printf("[TEST] Running test_validation_rejections...\n");

    int *refs = NULL;
    int count = 0;

    /* Negative numbers rejected (-1 is reserved for empty frames) */
    assert(parse_and_validate_reference_string("7 -1 2", &refs, &count) == VALIDATION_ERR_NEGATIVE_PAGE);

    /* Non-numeric characters rejected */
    assert(parse_and_validate_reference_string("7 abc 2", &refs, &count) == VALIDATION_ERR_INVALID_CHARACTER);

    /* Empty and whitespace string rejected */
    assert(parse_and_validate_reference_string("", &refs, &count) == VALIDATION_ERR_EMPTY_STRING);
    assert(parse_and_validate_reference_string("     ", &refs, &count) == VALIDATION_ERR_EMPTY_STRING);

    /* Frame count range checks */
    assert(validate_frame_count(0) == VALIDATION_ERR_FRAME_COUNT_TOO_LOW);
    assert(validate_frame_count(-1) == VALIDATION_ERR_FRAME_COUNT_TOO_LOW);
    assert(validate_frame_count(65) == VALIDATION_ERR_FRAME_COUNT_TOO_HIGH);
    assert(validate_frame_count(1) == VALIDATION_SUCCESS);
    assert(validate_frame_count(64) == VALIDATION_SUCCESS);

    printf("[PASS] test_validation_rejections passed.\n");
}

/**
 * @brief Tests consecutive simulation execution without memory leaks or stale state.
 */
static void test_consecutive_simulation_lifecycle(void) {
    printf("[TEST] Running test_consecutive_simulation_lifecycle...\n");

    /* Simulation 1 */
    int *refs1 = NULL;
    int count1 = 0;
    assert(parse_and_validate_reference_string("1 2 3", &refs1, &count1) == VALIDATION_SUCCESS);
    int res_count1 = 0;
    SimulationResult **res1 = run_simulation(refs1, count1, 2, ALGORITHM_FIFO, &res_count1);
    assert(res1 != NULL);
    assert(res1[0]->total_references == 3);
    assert(res1[0]->frame_count == 2);
    free_parsed_references(refs1);
    free_simulation_results(res1, res_count1);

    /* Simulation 2 */
    int *refs2 = NULL;
    int count2 = 0;
    assert(parse_and_validate_reference_string("9 9 9 9", &refs2, &count2) == VALIDATION_SUCCESS);
    int res_count2 = 0;
    SimulationResult **res2 = run_simulation(refs2, count2, 3, ALGORITHM_OPTIMAL, &res_count2);
    assert(res2 != NULL);
    assert(res2[0]->total_references == 4);
    assert(res2[0]->total_hits == 3);
    assert(res2[0]->total_faults == 1);
    free_parsed_references(refs2);
    free_simulation_results(res2, res_count2);

    printf("[PASS] test_consecutive_simulation_lifecycle passed.\n");
}

int main(void) {
    printf("====================================================\n");
    printf("    Full Pipeline Integration Tests (Person 4)      \n");
    printf("====================================================\n");

    test_e2e_specification_compare_all();
    test_individual_algorithm_dispatches();
    test_comma_delimited_input_pipeline();
    test_single_reference_pipeline();
    test_validation_rejections();
    test_consecutive_simulation_lifecycle();

    printf("\nAll Person 4 integration tests passed successfully!\n\n");
    return 0;
}
