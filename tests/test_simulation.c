/**
 * @file test_simulation.c
 * @brief Unit tests for Common Simulation Engine and Statistics modules.
 *
 * Owned by Person 2.
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "core/simulation.h"
#include "core/statistics.h"
#include "utils/memory.h"

static void test_statistics_edge_cases(void) {
    printf("[TEST] Running test_statistics_edge_cases...\n");

    /* 1. Zero references: avoid division by zero */
    assert(calculate_hit_ratio(0, 0) == 0.0);
    assert(calculate_fault_ratio(0, 0) == 0.0);
    assert(calculate_hit_ratio(-5, 0) == 0.0);
    assert(calculate_fault_ratio(-5, 0) == 0.0);
    assert(calculate_total_hits(NULL, 0) == 0);
    assert(calculate_total_faults(NULL, 0) == 0);

    /* 2. Normal metrics */
    double hit_ratio = calculate_hit_ratio(4, 10);
    double fault_ratio = calculate_fault_ratio(6, 10);
    assert(fabs(hit_ratio - 40.0) < 0.0001);
    assert(fabs(fault_ratio - 60.0) < 0.0001);
    assert(fabs((hit_ratio + fault_ratio) - 100.0) < 0.0001);

    /* 3. 100% hits */
    assert(fabs(calculate_hit_ratio(10, 10) - 100.0) < 0.0001);
    assert(fabs(calculate_fault_ratio(0, 10) - 0.0) < 0.0001);

    /* 4. 100% faults */
    assert(fabs(calculate_hit_ratio(0, 10) - 0.0) < 0.0001);
    assert(fabs(calculate_fault_ratio(10, 10) - 100.0) < 0.0001);

    printf("[PASS] test_statistics_edge_cases passed.\n");
}

static void test_simulation_engine_optimal_dispatch(void) {
    printf("[TEST] Running test_simulation_engine_optimal_dispatch...\n");

    const int sample_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    int ref_count = 10;
    int frame_count = 3;
    int result_count = 0;

    /* Execute Optimal algorithm through simulation engine */
    SimulationResult **results = run_simulation(sample_refs, ref_count, frame_count, ALGORITHM_OPTIMAL, &result_count);
    assert(results != NULL);
    assert(result_count == 1);
    assert(results[0] != NULL);
    assert(strcmp(results[0]->algorithm_name, "Optimal") == 0);
    assert(results[0]->total_references == 10);
    assert(results[0]->total_hits == 4);
    assert(results[0]->total_faults == 6);
    assert(fabs(results[0]->hit_ratio - 40.0) < 0.0001);
    assert(fabs(results[0]->fault_ratio - 60.0) < 0.0001);

    free_simulation_results(results, result_count);
    printf("[PASS] test_simulation_engine_optimal_dispatch passed.\n");
}

static void test_simulation_engine_all_dispatch(void) {
    printf("[TEST] Running test_simulation_engine_all_dispatch...\n");

    const int sample_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    int ref_count = 10;
    int frame_count = 3;
    int result_count = 0;

    /* Execute ALL algorithms through simulation engine */
    SimulationResult **results = run_simulation(sample_refs, ref_count, frame_count, ALGORITHM_ALL, &result_count);
    assert(results != NULL);
    assert(result_count == 3);
    assert(results[0] != NULL); /* FIFO */
    assert(results[1] != NULL); /* LRU */
    assert(results[2] != NULL); /* Optimal */

    assert(strcmp(results[0]->algorithm_name, "FIFO") == 0);
    assert(strcmp(results[1]->algorithm_name, "LRU") == 0);
    assert(strcmp(results[2]->algorithm_name, "Optimal") == 0);

    /* Optimal has been fully implemented */
    assert(results[2]->total_hits == 4);
    assert(results[2]->total_faults == 6);

    free_simulation_results(results, result_count);
    printf("[PASS] test_simulation_engine_all_dispatch passed.\n");
}

static void test_simulation_engine_invalid_inputs(void) {
    printf("[TEST] Running test_simulation_engine_invalid_inputs...\n");

    const int sample_refs[] = {1, 2, 3};
    int result_count = 42;

    assert(run_simulation(NULL, 3, 3, ALGORITHM_OPTIMAL, &result_count) == NULL);
    assert(result_count == 0);

    assert(run_simulation(sample_refs, 0, 3, ALGORITHM_OPTIMAL, &result_count) == NULL);
    assert(result_count == 0);

    assert(run_simulation(sample_refs, -1, 3, ALGORITHM_OPTIMAL, &result_count) == NULL);
    assert(result_count == 0);

    assert(run_simulation(sample_refs, 3, 0, ALGORITHM_OPTIMAL, &result_count) == NULL);
    assert(result_count == 0);

    assert(run_simulation(sample_refs, 3, -1, ALGORITHM_OPTIMAL, &result_count) == NULL);
    assert(result_count == 0);

    assert(run_simulation(sample_refs, 3, 3, (AlgorithmChoice)99, &result_count) == NULL);
    assert(result_count == 0);

    assert(run_simulation(sample_refs, 3, 3, ALGORITHM_OPTIMAL, NULL) == NULL);

    printf("[PASS] test_simulation_engine_invalid_inputs passed.\n");
}

static void test_algorithm_choice_to_string(void) {
    printf("[TEST] Running test_algorithm_choice_to_string...\n");

    assert(strcmp(algorithm_choice_to_string(ALGORITHM_FIFO), "FIFO") == 0);
    assert(strcmp(algorithm_choice_to_string(ALGORITHM_LRU), "LRU") == 0);
    assert(strcmp(algorithm_choice_to_string(ALGORITHM_OPTIMAL), "Optimal") == 0);
    assert(strcmp(algorithm_choice_to_string(ALGORITHM_ALL), "Compare All") == 0);
    assert(strcmp(algorithm_choice_to_string((AlgorithmChoice)99), "Unknown") == 0);

    printf("[PASS] test_algorithm_choice_to_string passed.\n");
}

int main(void) {
    printf("====================================================\n");
    printf("   Simulation Engine & Statistics Tests (Person 2)  \n");
    printf("====================================================\n");

    test_statistics_edge_cases();
    test_simulation_engine_optimal_dispatch();
    test_simulation_engine_all_dispatch();
    test_simulation_engine_invalid_inputs();
    test_algorithm_choice_to_string();

    printf("\nAll Person 2 Simulation Engine & Statistics tests passed successfully!\n\n");
    return 0;
}
