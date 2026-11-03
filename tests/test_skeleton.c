/**
 * @file test_skeleton.c
 * @brief Integration and Verification Test Suite.
 * 
 * Verifies Person 1 (Process & Resource Management), Person 2 (Banker's Algorithm),
 * and Person 3 (Deadlock Detection, Recovery, and RAG), plus Person 4 integration stubs.
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

    TEST_ASSERT(state.available[0] == 3 && state.available[1] == 3 && state.available[2] == 2, "Textbook Available is [3, 3, 2]");

    BankerSafetyResult result;
    ErrorCode err = banker_run_safety_check(&state, &result);
    TEST_ASSERT(err == ERR_SUCCESS, "banker_run_safety_check succeeds");
    TEST_ASSERT(result.is_safe == true, "System classified as SAFE");
    TEST_ASSERT(result.safe_sequence.is_valid == true, "Safe sequence is marked valid");
    TEST_ASSERT(result.safe_sequence.length == 5, "Safe sequence length is 5 (all processes finish)");
    TEST_ASSERT(result.unfinished_count == 0, "Unfinished process count is 0");
    TEST_ASSERT(result.safe_sequence.sequence[0] == 1, "First process in safe sequence is P1");

    /* Live state unchanged */
    TEST_ASSERT(state.available[0] == 3 && state.available[1] == 3 && state.available[2] == 2, "Live Available vector untouched [3, 3, 2]");
    TEST_ASSERT(state.allocation[1][0] == 2, "Live Allocation for P1 untouched [2, 0, 0]");

    /* Step trace */
    TEST_ASSERT(result.step_count == 5, "Recorded exactly 5 step-by-step Work updates");
    TEST_ASSERT(result.steps[0].work_after[0] == 5, "Step 1 Work[A] after P1 finishes becomes 3 + 2 = 5");

    char seq_str[128];
    banker_format_safe_sequence(&state, &result.safe_sequence, seq_str, sizeof(seq_str));
    TEST_ASSERT(strstr(seq_str, "P1 ->") != NULL, "Safe sequence string contains 'P1 ->'");
}

static void test_person2_safety_check_unsafe_state(void) {
    printf("[TEST SUITE 7] Person 2: Banker's Safety Algorithm (Unsafe State Analysis)\n");
    SystemState state;
    sys_load_sample_configuration(&state, 0);

    /* Zero out available so no process can satisfy Need */
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

    /* Case B: Rejects request exceeding remaining Need */
    int req_over_need[3] = {8, 0, 0};
    BankerRequestResult res_over_need;
    err = banker_evaluate_resource_request(&state, 0, req_over_need, &res_over_need);
    TEST_ASSERT(err == ERR_EXCEEDS_NEED, "Rejects request exceeding remaining Need");
    TEST_ASSERT(res_over_need.is_granted == false, "Request denied");

    /* Case C: Rejects request exceeding Available */
    int req_over_avail[3] = {0, 4, 0};
    BankerRequestResult res_over_avail;
    err = banker_evaluate_resource_request(&state, 0, req_over_avail, &res_over_avail);
    TEST_ASSERT(err == ERR_INSUFFICIENT_AVAILABLE, "Rejects request exceeding Available");
    TEST_ASSERT(res_over_avail.is_granted == false, "Request must wait");

    /* Case D: Request passing preliminary checks but resulting in UNSAFE state */
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

static void test_person3_deadlock_detection(void) {
    printf("[TEST SUITE 9] Person 3: Deadlock Detection & Outstanding Requests\n");
    SystemState state;

    /* 1. Test No Deadlock scenario (Sample 0 preset: no outstanding requests) */
    sys_load_sample_configuration(&state, 0);
    DeadlockResult no_dl_res;
    ErrorCode err = detection_run(&state, &no_dl_res);
    TEST_ASSERT(err == ERR_SUCCESS, "detection_run succeeds on safe state");
    TEST_ASSERT(no_dl_res.deadlock_detected == false, "State with 0 outstanding requests is NOT deadlocked");
    TEST_ASSERT(no_dl_res.deadlocked_count == 0, "Deadlocked process count is 0");

    /* 2. Record Outstanding Requests (Tracking) */
    int wait_req[3] = {1, 0, 1};
    err = detection_record_outstanding_request(&state, 0, wait_req);
    TEST_ASSERT(err == ERR_SUCCESS, "detection_record_outstanding_request succeeds");
    TEST_ASSERT(state.outstanding_requests[0][0] == 1, "Outstanding request recorded");
    TEST_ASSERT(state.processes[0].status == PROCESS_STATUS_WAITING, "Process status updated to WAITING");

    bool is_waiting = false;
    detection_is_process_waiting(&state, 0, &is_waiting);
    TEST_ASSERT(is_waiting == true, "detection_is_process_waiting confirms waiting status");

    detection_clear_outstanding_request(&state, 0);
    detection_is_process_waiting(&state, 0, &is_waiting);
    TEST_ASSERT(is_waiting == false, "detection_clear_outstanding_request resets waiting status");

    /* 3. Test Deadlock Detection on circular wait state (Sample 1 preset) */
    sys_load_sample_configuration(&state, 1);
    DeadlockResult dl_res;
    err = detection_run(&state, &dl_res);
    TEST_ASSERT(err == ERR_SUCCESS, "detection_run succeeds on circular wait state");
    TEST_ASSERT(dl_res.deadlock_detected == true, "Deadlock correctly DETECTED");
    TEST_ASSERT(dl_res.deadlocked_count == 3, "All 3 processes (P0, P1, P2) in circular wait are deadlocked");
    TEST_ASSERT(dl_res.involved_resource_count == 3, "All 3 resource types (R0, R1, R2) are involved");
    TEST_ASSERT(strstr(dl_res.explanation, "DEADLOCK DETECTED") != NULL, "Explanation accurately reports DEADLOCK");
}

static void test_person3_resource_allocation_graph(void) {
    printf("[TEST SUITE 10] Person 3: Resource Allocation Graph (RAG) Construction\n");
    SystemState state;
    sys_load_sample_configuration(&state, 1); /* 3 processes, 3 resources in circular wait */

    RAGGraph graph;
    ErrorCode err = rag_build_from_state(&state, &graph);
    TEST_ASSERT(err == ERR_SUCCESS, "rag_build_from_state succeeds");
    TEST_ASSERT(graph.node_count == 6, "Graph contains exactly 6 nodes (3 processes + 3 resources)");

    /* Verify edge counts: 3 allocation edges and 3 request edges */
    TEST_ASSERT(graph.edge_count == 6, "Graph contains 6 edges total (3 allocations + 3 requests)");

    int alloc_edge_count = 0;
    int req_edge_count = 0;
    int cycle_edge_count = 0;

    for (int e = 0; e < graph.edge_count; e++) {
        if (graph.edges[e].edge_type == RAG_EDGE_ALLOCATION) {
            alloc_edge_count++;
            TEST_ASSERT(graph.edges[e].from_type == RAG_NODE_RESOURCE && graph.edges[e].to_type == RAG_NODE_PROCESS,
                        "Allocation edge directed from Resource -> Process");
        } else if (graph.edges[e].edge_type == RAG_EDGE_REQUEST) {
            req_edge_count++;
            TEST_ASSERT(graph.edges[e].from_type == RAG_NODE_PROCESS && graph.edges[e].to_type == RAG_NODE_RESOURCE,
                        "Request edge directed from Process -> Resource");
        }
        if (graph.edges[e].is_deadlock_cycle) {
            cycle_edge_count++;
        }
    }

    TEST_ASSERT(alloc_edge_count == 3, "Found 3 allocation edges");
    TEST_ASSERT(req_edge_count == 3, "Found 3 request edges");
    TEST_ASSERT(cycle_edge_count == 6, "All 6 edges participating in the deadlock cycle are flagged");

    /* Reset RAG */
    err = rag_reset(&graph);
    TEST_ASSERT(err == ERR_SUCCESS && graph.node_count == 0 && graph.edge_count == 0, "rag_reset clears graph");
}

static void test_person3_deadlock_recovery(void) {
    printf("[TEST SUITE 11] Person 3: Deadlock Recovery & Victim Termination\n");
    SystemState state;
    sys_load_sample_configuration(&state, 1); /* Deadlocked state */

    DeadlockResult dl_res;
    detection_run(&state, &dl_res);
    TEST_ASSERT(dl_res.deadlock_detected == true, "Confirmed starting with deadlocked state");

    /* 1. Victim Selection Heuristic */
    int victim_id = -1;
    ErrorCode err = recovery_select_victim(&state, &dl_res, &victim_id);
    TEST_ASSERT(err == ERR_SUCCESS, "recovery_select_victim succeeds");
    TEST_ASSERT(victim_id >= 0 && victim_id < 3, "Victim selected is one of the deadlocked processes");

    /* 2. Execute Victim Termination & Recovery */
    RecoveryResult rec_res;
    err = recovery_execute_victim_termination(&state, victim_id, &rec_res);
    TEST_ASSERT(err == ERR_SUCCESS, "recovery_execute_victim_termination succeeds");
    TEST_ASSERT(rec_res.success == true, "Recovery outcome marked as success");
    TEST_ASSERT(state.processes[victim_id].status == PROCESS_STATUS_TERMINATED, "Victim process status set to TERMINATED");

    /* Verify victim allocations reclaimed */
    int victim_held_after = 0;
    for (int j = 0; j < state.resource_count; j++) {
        victim_held_after += state.allocation[victim_id][j];
    }
    TEST_ASSERT(victim_held_after == 0, "Victim allocation cleared to 0");
    TEST_ASSERT(state.available[victim_id] == 1, "Victim's held resource released back to Available vector");

    /* Verify deadlock was broken */
    TEST_ASSERT(rec_res.post_recovery_detection.deadlock_detected == false, "Post-recovery detection confirms deadlock ELIMINATED!");
}

static void test_person4_coordinator_integration(void) {
    printf("[TEST SUITE 12] Person 4: Coordinator Workflows, Multi-Module Integration & Recovery\n");
    SystemContext ctx;

    /* 1. Context Initialization */
    ErrorCode err = coordinator_init(&ctx);
    TEST_ASSERT(err == ERR_SUCCESS, "coordinator_init succeeds");
    TEST_ASSERT(ctx.live_state.process_count == 0, "Initial process count is 0");
    TEST_ASSERT(ctx.latest_rag.node_count == 0, "Initial RAG node count is 0");

    /* 2. Full System Analysis on Safe State (Preset 0) */
    sys_load_sample_configuration(&ctx.live_state, 0);
    err = coordinator_run_full_analysis(&ctx);
    TEST_ASSERT(err == ERR_SUCCESS, "coordinator_run_full_analysis succeeds on Preset 0");
    TEST_ASSERT(ctx.live_state.overall_status == SYSTEM_STATUS_SAFE, "System status evaluated as SAFE");
    TEST_ASSERT(ctx.latest_safety_result.is_safe == true, "Banker's result cached as SAFE");
    TEST_ASSERT(ctx.latest_deadlock_result.deadlock_detected == false, "Deadlock detection cached as NO DEADLOCK");
    TEST_ASSERT(ctx.latest_rag.node_count == 8, "RAG graph populated with 8 nodes (5 processes + 3 resources)");
    TEST_ASSERT(ctx.latest_rag.edge_count > 0, "RAG graph has active allocation edges");

    /* 3. Handle Resource Request: Safe Request (P1 requests [1, 0, 2]) */
    int req_p1[3] = { 1, 0, 2 };
    err = coordinator_handle_resource_request(&ctx, 1, req_p1);
    TEST_ASSERT(err == ERR_SUCCESS, "coordinator_handle_resource_request grants safe request for P1");
    TEST_ASSERT(ctx.live_state.allocation[1][0] == 3, "P1 allocation for A increased from 2 to 3");
    TEST_ASSERT(ctx.live_state.allocation[1][2] == 2, "P1 allocation for C increased from 0 to 2");
    TEST_ASSERT(ctx.live_state.available[0] == 2 && ctx.live_state.available[2] == 0, "Available vector decreased accordingly");
    TEST_ASSERT(ctx.live_state.overall_status == SYSTEM_STATUS_SAFE, "System remains SAFE after granted request");

    /* 4. Handle Resource Request: Unsafe Request (P0 requests [0, 2, 0]) */
    int req_p0[3] = { 0, 2, 0 };
    err = coordinator_handle_resource_request(&ctx, 0, req_p0);
    TEST_ASSERT(err == ERR_SYSTEM_UNSAFE, "coordinator_handle_resource_request denies unsafe request for P0");
    TEST_ASSERT(ctx.live_state.allocation[0][1] == 1, "P0 live allocation preserved unchanged (not granted)");
    TEST_ASSERT(ctx.live_state.outstanding_requests[0][1] == 2, "Unsafe request converted to outstanding request for P0");

    /* 5. Full System Analysis on Deadlocked State (Preset 1) */
    sys_load_sample_configuration(&ctx.live_state, 1);
    err = coordinator_run_full_analysis(&ctx);
    TEST_ASSERT(err == ERR_SUCCESS, "coordinator_run_full_analysis succeeds on Preset 1");
    TEST_ASSERT(ctx.live_state.overall_status == SYSTEM_STATUS_DEADLOCKED, "System status evaluated as DEADLOCKED");
    TEST_ASSERT(ctx.latest_deadlock_result.deadlock_detected == true, "Deadlock detection cached as DEADLOCKED");
    TEST_ASSERT(ctx.latest_deadlock_result.deadlocked_count == 3, "All 3 processes detected in circular wait");

    /* 6. Coordinated Recovery Workflow */
    int victim_id = -1;
    recovery_select_victim(&ctx.live_state, &ctx.latest_deadlock_result, &victim_id);
    RecoveryResult rec_res;
    err = coordinator_execute_recovery(&ctx, victim_id, &rec_res);
    TEST_ASSERT(err == ERR_SUCCESS, "coordinator_execute_recovery succeeds");
    TEST_ASSERT(rec_res.success == true, "Recovery execution succeeded");
    TEST_ASSERT(ctx.latest_deadlock_result.deadlock_detected == false, "Deadlock resolved and auto-synchronized across system context");
    TEST_ASSERT(ctx.live_state.overall_status != SYSTEM_STATUS_DEADLOCKED, "System status updated away from DEADLOCKED");

    /* 7. Parameter Validation & Boundary Checks */
    TEST_ASSERT(coordinator_init(NULL) == ERR_INVALID_PARAM, "coordinator_init rejects NULL");
    TEST_ASSERT(coordinator_run_full_analysis(NULL) == ERR_INVALID_PARAM, "coordinator_run_full_analysis rejects NULL");
    TEST_ASSERT(coordinator_handle_resource_request(NULL, 0, req_p1) == ERR_INVALID_PARAM, "coordinator_handle_resource_request rejects NULL ctx");
    TEST_ASSERT(coordinator_handle_resource_request(&ctx, 999, req_p1) == ERR_INVALID_PARAM, "coordinator_handle_resource_request rejects out-of-bounds PID");
    TEST_ASSERT(coordinator_execute_recovery(NULL, 0, &rec_res) == ERR_INVALID_PARAM, "coordinator_execute_recovery rejects NULL ctx");
    TEST_ASSERT(coordinator_execute_recovery(&ctx, -1, &rec_res) == ERR_INVALID_PARAM, "coordinator_execute_recovery rejects invalid victim PID");
}

int main(void) {
    printf("=================================================================\n");
    printf(" Deadlock Simulation System - Person 1, 2, 3 & 4 Test Suite      \n");
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
    test_person3_deadlock_detection();
    printf("\n");
    test_person3_resource_allocation_graph();
    printf("\n");
    test_person3_deadlock_recovery();
    printf("\n");
    test_person4_coordinator_integration();

    printf("\n=================================================================\n");
    printf(" Test Results: %d / %d Passed\n", g_tests_passed, g_tests_run);
    printf("=================================================================\n");

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
