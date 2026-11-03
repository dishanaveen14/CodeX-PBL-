/**
 * @file test_skeleton.c
 * @brief Integration and Verification Test Suite.
 * 
 * Verifies Person 1 (Process & Resource Management) active implementation,
 * mathematical matrix invariants, and module stubs for Person 2, 3, and 4.
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../include/common/types.h"
#include "../include/common/system_state.h"
#include "../include/common/error.h"
#include "../include/person1_process_resource/process_mgmt.h"
#include "../include/person1_process_resource/resource_mgmt.h"
#include "../include/person1_process_resource/matrix_mgmt.h"
#include "../include/person2_bankers/bankers.h"
#include "../include/person3_detection_recovery/detection.h"
#include "../include/person3_detection_recovery/rag.h"
#include "../include/person3_detection_recovery/recovery.h"
#include "../include/person4_integration/coordinator.h"

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_tests_run++; \
    if (cond) { \
        g_tests_passed++; \
        printf("  [PASS] %s\n", msg); \
    } else { \
        printf("  [FAIL] %s (Line %d)\n", msg, __LINE__); \
    } \
} while(0)

static void test_system_state_lifecycle(void) {
    printf("[TEST SUITE 1] System State Lifecycle & Memory Contracts\n");
    SystemState state;
    ErrorCode err = system_state_init(&state);
    TEST_ASSERT(err == ERR_SUCCESS, "system_state_init succeeds");
    TEST_ASSERT(state.process_count == 0, "Initial process count is 0");
    TEST_ASSERT(state.resource_count == 0, "Initial resource count is 0");
    TEST_ASSERT(state.overall_status == SYSTEM_STATUS_UNCHECKED, "Initial status is UNCHECKED");

    SystemState clone;
    err = system_state_clone(&state, &clone);
    TEST_ASSERT(err == ERR_SUCCESS, "system_state_clone succeeds");
    TEST_ASSERT(clone.overall_status == SYSTEM_STATUS_UNCHECKED, "Cloned status matches original");
}

static void test_person1_process_management(void) {
    printf("[TEST SUITE 2] Person 1: Process Creation, Queries & Safe Removal\n");
    SystemState state;
    system_state_init(&state);

    /* 1. Create Process with Auto-naming */
    int p0_id = -1;
    ErrorCode err = sys_process_create(&state, NULL, &p0_id);
    TEST_ASSERT(err == ERR_SUCCESS, "sys_process_create (auto-name) succeeds");
    TEST_ASSERT(p0_id == 0, "First process assigned ID 0");
    TEST_ASSERT(strcmp(state.processes[0].name, "P0") == 0, "Auto-generated name is 'P0'");

    /* 2. Create Process with Custom Name */
    int p1_id = -1;
    err = sys_process_create(&state, "Worker_P1", &p1_id);
    TEST_ASSERT(err == ERR_SUCCESS, "sys_process_create (custom name) succeeds");
    TEST_ASSERT(p1_id == 1, "Second process assigned ID 1");
    TEST_ASSERT(strcmp(state.processes[1].name, "Worker_P1") == 0, "Custom name stored correctly");

    /* 3. Query Count and Process Details */
    int count = 0;
    err = sys_process_get_count(&state, &count);
    TEST_ASSERT(err == ERR_SUCCESS && count == 2, "sys_process_get_count returns 2");

    Process proc_query;
    err = sys_process_get(&state, 1, &proc_query);
    TEST_ASSERT(err == ERR_SUCCESS && strcmp(proc_query.name, "Worker_P1") == 0, "sys_process_get retrieves correct metadata");

    /* 4. Safe Removal with Index Re-shifting */
    int p2_id = -1;
    sys_process_create(&state, "P2", &p2_id);
    TEST_ASSERT(state.process_count == 3, "Created 3 processes total");

    err = sys_process_remove(&state, 1); /* Remove P1 */
    TEST_ASSERT(err == ERR_SUCCESS, "sys_process_remove(1) succeeds");
    TEST_ASSERT(state.process_count == 2, "Process count decremented to 2");
    TEST_ASSERT(state.processes[1].id == 1, "Shifted process has re-indexed ID 1");
    TEST_ASSERT(strcmp(state.processes[1].name, "P2") == 0, "Shifted process is 'P2'");
}

static void test_person1_resource_management(void) {
    printf("[TEST SUITE 3] Person 1: Resource Creation, Instances & Total Updates\n");
    SystemState state;
    system_state_init(&state);

    /* 1. Create Resource Types */
    int r0_id = -1;
    ErrorCode err = sys_resource_create(&state, "Disk", 10, &r0_id);
    TEST_ASSERT(err == ERR_SUCCESS, "sys_resource_create (Disk, 10) succeeds");
    TEST_ASSERT(r0_id == 0, "First resource assigned ID 0");
    TEST_ASSERT(state.total[0] == 10, "Total instances initialized to 10");
    TEST_ASSERT(state.available[0] == 10, "Available instances initialized to 10");

    /* 2. Auto-named Resource (A, B, C...) */
    int r1_id = -1;
    err = sys_resource_create(&state, NULL, 5, &r1_id);
    TEST_ASSERT(err == ERR_SUCCESS, "sys_resource_create (auto-name) succeeds");
    TEST_ASSERT(r1_id == 1, "Second resource assigned ID 1");
    TEST_ASSERT(strcmp(state.resources[1].name, "B") == 0, "Auto-generated name is 'B'");

    /* 3. Negative instances rejected */
    err = sys_resource_create(&state, "NegativeRes", -5, NULL);
    TEST_ASSERT(err == ERR_INVALID_PARAM, "sys_resource_create rejects negative total instances");

    /* 4. Update Resource Total */
    err = sys_resource_set_total(&state, 0, 15);
    TEST_ASSERT(err == ERR_SUCCESS, "sys_resource_set_total expands total to 15");
    TEST_ASSERT(state.total[0] == 15, "state.total[0] is now 15");
    TEST_ASSERT(state.available[0] == 15, "state.available[0] updated to 15");
}

static void test_person1_matrices_and_math_invariants(void) {
    printf("[TEST SUITE 4] Person 1: Matrices, Math Invariants (Need & Available)\n");
    SystemState state;
    system_state_init(&state);

    sys_process_create(&state, "P0", NULL);
    sys_process_create(&state, "P1", NULL);
    sys_resource_create(&state, "A", 10, NULL);
    sys_resource_create(&state, "B", 5, NULL);

    /* 1. Set Maximum Requirement */
    ErrorCode err = sys_matrix_set_maximum(&state, 0, 0, 7);
    TEST_ASSERT(err == ERR_SUCCESS, "sys_matrix_set_maximum(P0, A, 7) succeeds");
    err = sys_matrix_set_maximum(&state, 0, 1, 5);
    TEST_ASSERT(err == ERR_SUCCESS, "sys_matrix_set_maximum(P0, B, 5) succeeds");

    /* 2. Verify Need automatically equals Maximum when Allocation is 0 */
    TEST_ASSERT(state.need[0][0] == 7, "Need[0][0] auto-calculated as 7");
    TEST_ASSERT(state.need[0][1] == 5, "Need[0][1] auto-calculated as 5");

    /* 3. Set Allocation Matrix */
    err = sys_matrix_set_allocation(&state, 0, 0, 2);
    TEST_ASSERT(err == ERR_SUCCESS, "sys_matrix_set_allocation(P0, A, 2) succeeds");

    /* 4. Verify Need = Maximum - Allocation and Available = Total - Allocation */
    TEST_ASSERT(state.need[0][0] == 5, "Need[0][0] recalculated: 7 - 2 = 5");
    TEST_ASSERT(state.available[0] == 8, "Available[0] recalculated: 10 - 2 = 8");

    /* 5. Reject Allocation exceeding Maximum */
    err = sys_matrix_set_allocation(&state, 0, 0, 8);
    TEST_ASSERT(err == ERR_EXCEEDS_MAX_DEMAND, "sys_matrix_set_allocation rejects allocation > maximum");

    /* 6. Reject Allocation exceeding Total Capacity */
    err = sys_matrix_set_maximum(&state, 1, 1, 5);
    err = sys_matrix_set_allocation(&state, 0, 1, 4);
    err = sys_matrix_set_allocation(&state, 1, 1, 3); /* 4 + 3 = 7 > 5 */
    TEST_ASSERT(err == ERR_OVER_ALLOCATION, "sys_matrix_set_allocation rejects sum(allocation) > total capacity");

    /* 7. Comprehensive Matrix Validation */
    sys_matrix_set_allocation(&state, 1, 1, 1); /* 4 + 1 = 5 <= 5 */
    err = sys_matrix_validate_all(&state);
    TEST_ASSERT(err == ERR_SUCCESS, "sys_matrix_validate_all confirms consistent state");
}

static void test_person1_request_and_release(void) {
    printf("[TEST SUITE 5] Person 1: Preliminary Request Checks & Resource Release\n");
    SystemState state;
    system_state_init(&state);

    sys_process_create(&state, "P0", NULL);
    sys_resource_create(&state, "A", 10, NULL);
    sys_matrix_set_maximum(&state, 0, 0, 6);
    sys_matrix_set_allocation(&state, 0, 0, 2);
    /* Need = 4, Available = 8 */

    /* 1. Request exceeding remaining Need */
    int req_over_need[1] = {5};
    ErrorCode err = sys_request_validate_preliminary(&state, 0, req_over_need);
    TEST_ASSERT(err == ERR_EXCEEDS_NEED, "sys_request_validate_preliminary rejects request > Need");

    /* 2. Request exceeding Available */
    state.available[0] = 1;
    int req_over_avail[1] = {2};
    err = sys_request_validate_preliminary(&state, 0, req_over_avail);
    TEST_ASSERT(err == ERR_INSUFFICIENT_AVAILABLE, "sys_request_validate_preliminary rejects request > Available");

    /* 3. Valid Request Application */
    state.available[0] = 8;
    int valid_req[1] = {2};
    err = sys_allocation_apply(&state, 0, valid_req);
    TEST_ASSERT(err == ERR_SUCCESS, "sys_allocation_apply commits valid request");
    TEST_ASSERT(state.allocation[0][0] == 4, "Allocation increased to 4");
    TEST_ASSERT(state.need[0][0] == 2, "Need decreased to 2");
    TEST_ASSERT(state.available[0] == 6, "Available decreased to 6");

    /* 4. Release Resources */
    err = sys_resources_release(&state, 0, NULL); /* Release all */
    TEST_ASSERT(err == ERR_SUCCESS, "sys_resources_release(NULL) releases all resources");
    TEST_ASSERT(state.allocation[0][0] == 0, "Allocation reset to 0");
    TEST_ASSERT(state.available[0] == 10, "Available restored to full total 10");
}

static void test_person1_preset_loader(void) {
    printf("[TEST SUITE 6] Person 1: Preset Configuration Loader\n");
    SystemState state;
    system_state_init(&state);

    /* Load Textbook Safe State (Silberschatz) */
    ErrorCode err = sys_load_sample_configuration(&state, 0);
    TEST_ASSERT(err == ERR_SUCCESS, "sys_load_sample_configuration(0) succeeds");
    TEST_ASSERT(state.process_count == 5, "Loaded 5 processes (P0..P4)");
    TEST_ASSERT(state.resource_count == 3, "Loaded 3 resources (A, B, C)");
    TEST_ASSERT(state.total[0] == 10 && state.total[1] == 5 && state.total[2] == 7, "Total instances [10, 5, 7]");
    TEST_ASSERT(state.available[0] == 3 && state.available[1] == 3 && state.available[2] == 2, "Available vector matches textbook [3, 3, 2]");
    TEST_ASSERT(state.need[0][0] == 7 && state.need[0][1] == 4 && state.need[0][2] == 3, "Need[0] matches textbook [7, 4, 3]");

    /* Validate All */
    err = sys_matrix_validate_all(&state);
    TEST_ASSERT(err == ERR_SUCCESS, "Textbook configuration passes sys_matrix_validate_all");
}

static void test_unimplemented_stubs(void) {
    printf("[TEST SUITE 7] Person 2, 3, 4 Stubs Pending Implementation\n");
    SystemState state;
    system_state_init(&state);

    BankerSafetyResult safety_res;
    TEST_ASSERT(banker_run_safety_check(&state, &safety_res) == ERR_NOT_IMPLEMENTED, "Person 2 Banker stub intact");

    DeadlockResult dl_res;
    TEST_ASSERT(detection_run(&state, &dl_res) == ERR_NOT_IMPLEMENTED, "Person 3 Detection stub intact");

    SystemContext ctx;
    coordinator_init(&ctx);
    TEST_ASSERT(coordinator_run_full_analysis(&ctx) == ERR_NOT_IMPLEMENTED, "Person 4 Coordinator stub intact");
}

int main(void) {
    printf("=================================================================\n");
    printf(" Deadlock Simulation System - Person 1 & Skeleton Test Suite     \n");
    printf("=================================================================\n\n");

    test_system_state_lifecycle();
    printf("\n");
    test_person1_process_management();
    printf("\n");
    test_person1_resource_management();
    printf("\n");
    test_person1_matrices_and_math_invariants();
    printf("\n");
    test_person1_request_and_release();
    printf("\n");
    test_person1_preset_loader();
    printf("\n");
    test_unimplemented_stubs();

    printf("\n=================================================================\n");
    printf(" Test Results: %d / %d Passed\n", g_tests_passed, g_tests_run);
    printf("=================================================================\n");

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
