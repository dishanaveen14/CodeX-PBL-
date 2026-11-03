/**
 * @file test_skeleton.c
 * @brief Integration and Verification Test Suite.
 * 
 * Verifies Person 1 (Process & Resource Management) and Person 2 (Banker's Algorithm
 * & Safe-State Analysis) implementations, plus Person 3 and 4 stubs.
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

static void test_person2_safety_check_safe_state(void) {
    printf("[TEST SUITE 6] Person 2: Banker's Safety Algorithm (Safe State Analysis)\n");
    SystemState state;
    sys_load_sample_configuration(&state, 0); /* Load textbook 5P/3R configuration */

    /* Verify baseline state */
    TEST_ASSERT(state.available[0] == 3 && state.available[1] == 3 && state.available[2] == 2, "Textbook Available is [3, 3, 2]");

    BankerSafetyResult result;
    ErrorCode err = banker_run_safety_check(&state, &result);
    TEST_ASSERT(err == ERR_SUCCESS, "banker_run_safety_check succeeds");
    TEST_ASSERT(result.is_safe == true, "System classified as SAFE");
    TEST_ASSERT(result.safe_sequence.is_valid == true, "Safe sequence is marked valid");
    TEST_ASSERT(result.safe_sequence.length == 5, "Safe sequence length is 5 (all processes finish)");
    TEST_ASSERT(result.unfinished_count == 0, "Unfinished process count is 0");

    /* Check that P1 finishes first in textbook order */
    TEST_ASSERT(result.safe_sequence.sequence[0] == 1, "First process in safe sequence is P1");

    /* Verify live state was NOT modified during safety check */
    TEST_ASSERT(state.available[0] == 3 && state.available[1] == 3 && state.available[2] == 2, "Live Available vector untouched [3, 3, 2]");
    TEST_ASSERT(state.allocation[1][0] == 2, "Live Allocation for P1 untouched [2, 0, 0]");

    /* Verify step-by-step trace was recorded */
    TEST_ASSERT(result.step_count == 5, "Recorded exactly 5 step-by-step Work updates");
    TEST_ASSERT(result.steps[0].work_after[0] == 5, "Step 1 Work[A] after P1 finishes becomes 3 + 2 = 5");

    /* Safe sequence formatting */
    char seq_str[128];
    banker_format_safe_sequence(&state, &result.safe_sequence, seq_str, sizeof(seq_str));
    TEST_ASSERT(strstr(seq_str, "P1 ->") != NULL, "Safe sequence string contains 'P1 ->'");
}

static void test_person2_safety_check_unsafe_state(void) {
    printf("[TEST SUITE 7] Person 2: Banker's Safety Algorithm (Unsafe State Analysis)\n");
    SystemState state;
    sys_load_sample_configuration(&state, 0);

    /* Allocate all available instances so no process can satisfy its Need */
    state.available[0] = 0;
    state.available[1] = 0;
    state.available[2] = 0;

    BankerSafetyResult result;
    ErrorCode err = banker_run_safety_check(&state, &result);
    TEST_ASSERT(err == ERR_SUCCESS, "banker_run_safety_check executes successfully");
    TEST_ASSERT(result.is_safe == false, "State correctly identified as UNSAFE");
    TEST_ASSERT(result.safe_sequence.is_valid == false, "Safe sequence is marked invalid");
    TEST_ASSERT(result.unfinished_count == 5, "All 5 processes unable to finish");
    TEST_ASSERT(strstr(result.explanation, "UNSAFE") != NULL, "Explanation accurately notes UNSAFE state");

    SafeSequence seq;
    err = banker_find_safe_sequence(&state, &seq);
    TEST_ASSERT(err == ERR_SYSTEM_UNSAFE, "banker_find_safe_sequence returns ERR_SYSTEM_UNSAFE when no sequence exists");
}

static void test_person2_request_evaluation_grant_and_deny(void) {
    printf("[TEST SUITE 8] Person 2: Banker's Resource-Request Avoidance Algorithm\n");
    SystemState state;
    sys_load_sample_configuration(&state, 0); /* Available: [3, 3, 2] */

    /* Case A: P1 requests (1, 0, 2) - Standard textbook grantable request */
    int req_p1[3] = {1, 0, 2};
    BankerRequestResult res_p1;
    ErrorCode err = banker_evaluate_resource_request(&state, 1, req_p1, &res_p1);
    TEST_ASSERT(err == ERR_SUCCESS, "banker_evaluate_resource_request succeeds");
    TEST_ASSERT(res_p1.is_granted == true, "Request for P1 (1, 0, 2) is APPROVED (trial state is safe)");
    TEST_ASSERT(res_p1.trial_result.is_safe == true, "Trial state confirmed SAFE");

    /* Ensure live state was NOT modified during trial */
    TEST_ASSERT(state.available[0] == 3, "Live Available[0] remains 3 (not mutated before commit)");
    TEST_ASSERT(state.allocation[1][0] == 2, "Live Allocation[P1][0] remains 2");

    /* Case B: P4 requests (3, 3, 0) - Exceeds Need (Need for P4 is [4, 3, 1], Need[B] is 3, wait, P0 requests (8, 0, 0) exceeds Need) */
    int req_over_need[3] = {8, 0, 0}; /* Need[0] is [7, 4, 3] */
    BankerRequestResult res_over_need;
    err = banker_evaluate_resource_request(&state, 0, req_over_need, &res_over_need);
    TEST_ASSERT(err == ERR_EXCEEDS_NEED, "Rejects request exceeding remaining Need");
    TEST_ASSERT(res_over_need.is_granted == false, "Request denied");

    /* Case C: P0 requests (0, 4, 0) - Exceeds Available (Available[B] is 3) */
    int req_over_avail[3] = {0, 4, 0};
    BankerRequestResult res_over_avail;
    err = banker_evaluate_resource_request(&state, 0, req_over_avail, &res_over_avail);
    TEST_ASSERT(err == ERR_INSUFFICIENT_AVAILABLE, "Rejects request exceeding Available");
    TEST_ASSERT(res_over_avail.is_granted == false, "Request must wait");

    /* Case D: Request that passes preliminary checks (Req <= Need and Req <= Avail) but results in UNSAFE state */
    /* Textbook Silberschatz avoidance example: Total = 12, P0: Alloc 5, Max 10; P1: Alloc 2, Max 4; P2: Alloc 2, Max 9.
       Available = 3. If P2 requests 1, Available becomes 2. P1 finishes and Work becomes 4, but P0 needs 5 and P2 needs 6.
       Neither can finish, so trial state is UNSAFE! */
    SystemState unsafe_demo;
    system_state_init(&unsafe_demo);
    sys_process_create(&unsafe_demo, "P0", NULL);
    sys_process_create(&unsafe_demo, "P1", NULL);
    sys_process_create(&unsafe_demo, "P2", NULL);
    sys_resource_create(&unsafe_demo, "Tape", 12, NULL);

    unsafe_demo.maximum[0][0] = 10; unsafe_demo.allocation[0][0] = 5; unsafe_demo.need[0][0] = 5;
    unsafe_demo.maximum[1][0] = 4;  unsafe_demo.allocation[1][0] = 2; unsafe_demo.need[1][0] = 2;
    unsafe_demo.maximum[2][0] = 9;  unsafe_demo.allocation[2][0] = 2; unsafe_demo.need[2][0] = 7;
    unsafe_demo.total[0] = 12;
    unsafe_demo.available[0] = 3;

    int p2_req[1] = {1};
    BankerRequestResult res_unsafe;
    err = banker_evaluate_resource_request(&unsafe_demo, 2, p2_req, &res_unsafe);
    TEST_ASSERT(err == ERR_SYSTEM_UNSAFE, "banker_evaluate_resource_request rejects request resulting in UNSAFE trial state");
    TEST_ASSERT(res_unsafe.is_granted == false, "Unsafe request correctly DENIED");
    TEST_ASSERT(res_unsafe.trial_result.is_safe == false, "Trial state confirmed UNSAFE");
    TEST_ASSERT(unsafe_demo.available[0] == 3, "Live unsafe_demo Available[0] preserved as 3");
}

static void test_person3_and_4_stubs(void) {
    printf("[TEST SUITE 9] Person 3 & 4 Stubs Pending Implementation\n");
    SystemState state;
    system_state_init(&state);

    DeadlockResult dl_res;
    TEST_ASSERT(detection_run(&state, &dl_res) == ERR_NOT_IMPLEMENTED, "Person 3 Detection stub intact");

    RAGGraph graph;
    TEST_ASSERT(rag_build_from_state(&state, &graph) == ERR_NOT_IMPLEMENTED, "Person 3 RAG stub intact");

    SystemContext ctx;
    coordinator_init(&ctx);
    TEST_ASSERT(coordinator_run_full_analysis(&ctx) == ERR_NOT_IMPLEMENTED, "Person 4 Coordinator stub intact");
}

int main(void) {
    printf("=================================================================\n");
    printf(" Deadlock Simulation System - Person 1, 2 & Skeleton Test Suite  \n");
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
    test_person2_safety_check_safe_state();
    printf("\n");
    test_person2_safety_check_unsafe_state();
    printf("\n");
    test_person2_request_evaluation_grant_and_deny();
    printf("\n");
    test_person3_and_4_stubs();

    printf("\n=================================================================\n");
    printf(" Test Results: %d / %d Passed\n", g_tests_passed, g_tests_run);
    printf("=================================================================\n");

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
