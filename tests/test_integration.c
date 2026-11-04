/**
 * @file test_integration.c
 * @brief Integration test suite connecting input validation, simulation engine, and algorithms.
 *
 * Owned by Person 4 (Integration).
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "common.h"
#include "core/simulation.h"
#include "gui/input_validation.h"
#include "utils/memory.h"

static void test_pipeline_valid_flow(void) {
    printf("[TEST] Running test_pipeline_valid_flow...\n");

    const char *input_str = "7 0 1 2 0 3 0 4 2 3";
    int *references = NULL;
    int ref_count = 0;

    ValidationStatus ref_status = parse_and_validate_reference_string(input_str, &references, &ref_count);
    assert(ref_status == VALIDATION_SUCCESS);
    assert(ref_count == 10);
    assert(references != NULL);

    int frame_count = 3;
    ValidationStatus frame_status = validate_frame_count(frame_count);
    assert(frame_status == VALIDATION_SUCCESS);

    int result_count = 0;
    SimulationResult **results = run_simulation(references, ref_count, frame_count, ALGORITHM_ALL, &result_count);
    assert(results != NULL);
    assert(result_count == 3);

    for (int i = 0; i < result_count; ++i) {
        assert(results[i] != NULL);
        assert(results[i]->total_references == 10);
        assert(results[i]->frame_count == 3);
    }

    free_simulation_results(results, result_count);
    free_parsed_references(references);

    printf("[PASS] test_pipeline_valid_flow passed.\n");
}

static void test_pipeline_edge_cases(void) {
    printf("[TEST] Running test_pipeline_edge_cases...\n");

    int *refs = NULL;
    int count = 0;

    /* Negative numbers rejected */
    assert(parse_and_validate_reference_string("7 -1 2", &refs, &count) == VALIDATION_ERR_NEGATIVE_PAGE);

    /* Non-numeric rejected */
    assert(parse_and_validate_reference_string("7 abc 2", &refs, &count) == VALIDATION_ERR_INVALID_CHARACTER);

    /* Empty input rejected */
    assert(parse_and_validate_reference_string("   ", &refs, &count) == VALIDATION_ERR_EMPTY_STRING);

    /* Frame count zero/negative rejected */
    assert(validate_frame_count(0) == VALIDATION_ERR_FRAME_COUNT_TOO_LOW);
    assert(validate_frame_count(-5) == VALIDATION_ERR_FRAME_COUNT_TOO_LOW);
    assert(validate_frame_count(100) == VALIDATION_ERR_FRAME_COUNT_TOO_HIGH);

    printf("[PASS] test_pipeline_edge_cases passed.\n");
}

int main(void) {
    printf("=== Full Pipeline Integration Tests (Person 4) ===\n");
    test_pipeline_valid_flow();
    test_pipeline_edge_cases();
    printf("All integration tests passed successfully!\n\n");
    return 0;
}

