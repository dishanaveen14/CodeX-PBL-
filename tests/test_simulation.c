/**
 * @file test_simulation.c
 * @brief Unit test skeleton for common simulation engine and statistics.
 *
 * Owned by Person 2.
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "core/simulation.h"
#include "core/statistics.h"
#include "utils/memory.h"

static void test_statistics_formulas(void) {
    printf("[TEST] Running test_statistics_formulas...\n");

    /* Division by zero handling */
    assert(calculate_hit_ratio(0, 0) == 0.0);
    assert(calculate_fault_ratio(0, 0) == 0.0);

    /* Normal calculations */
    double hit_ratio = calculate_hit_ratio(1, 10);
    double fault_ratio = calculate_fault_ratio(9, 10);
    assert(fabs(hit_ratio - 10.0) < 0.0001);
    assert(fabs(fault_ratio - 90.0) < 0.0001);
    assert(fabs((hit_ratio + fault_ratio) - 100.0) < 0.0001);

    printf("[PASS] test_statistics_formulas passed.\n");
}

static void test_simulation_engine_dispatch(void) {
    printf("[TEST] Running test_simulation_engine_dispatch...\n");

    const int sample_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3};
    int ref_count = 10;
    int frame_count = 3;
    int result_count = 0;

    /* Test single algorithm dispatch */
    SimulationResult **fifo_results = run_simulation(sample_refs, ref_count, frame_count, ALGORITHM_FIFO, &result_count);
    assert(fifo_results != NULL);
    assert(result_count == 1);
    assert(fifo_results[0] != NULL);
    free_simulation_results(fifo_results, result_count);

    /* Test ALL algorithms dispatch */
    SimulationResult **all_results = run_simulation(sample_refs, ref_count, frame_count, ALGORITHM_ALL, &result_count);
    assert(all_results != NULL);
    assert(result_count == 3);
    assert(all_results[0] != NULL);
    assert(all_results[1] != NULL);
    assert(all_results[2] != NULL);
    free_simulation_results(all_results, result_count);

    /* Test invalid inputs */
    result_count = 99;
    assert(run_simulation(NULL, 10, 3, ALGORITHM_FIFO, &result_count) == NULL);
    assert(result_count == 0);

    printf("[PASS] test_simulation_engine_dispatch passed.\n");
}

int main(void) {
    printf("=== Simulation Engine & Statistics Tests (Person 2) ===\n");
    test_statistics_formulas();
    test_simulation_engine_dispatch();
    printf("All simulation engine skeleton tests passed successfully!\n\n");
    return 0;
}

