/**
 * @file rag.c
 * @brief Person 3: Resource Allocation Graph (RAG) Construction Implementation.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 3 (Deadlock Detection, Recovery, and RAG)
 */

#include "../../include/person3_detection_recovery/rag.h"
#include "../../include/person3_detection_recovery/detection.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

ErrorCode rag_reset(RAGGraph *graph) {
    if (graph == NULL) {
        return ERR_INVALID_PARAM;
    }
    memset(graph, 0, sizeof(RAGGraph));
    return ERR_SUCCESS;
}

ErrorCode rag_build_from_state(const SystemState *state, RAGGraph *out_graph) {
    if (state == NULL || out_graph == NULL) {
        return ERR_INVALID_PARAM;
    }

    rag_reset(out_graph);

    /* Run detection to identify deadlocked nodes for visual highlighting */
    DeadlockResult dl_result;
    detection_run(state, &dl_result);

    bool is_proc_deadlocked[MAX_PROCESSES] = {false};
    for (int k = 0; k < dl_result.deadlocked_count; k++) {
        int pid = dl_result.deadlocked_processes[k];
        if (pid >= 0 && pid < MAX_PROCESSES) {
            is_proc_deadlocked[pid] = true;
        }
    }

    bool is_res_deadlocked[MAX_RESOURCES] = {false};
    for (int k = 0; k < dl_result.involved_resource_count; k++) {
        int rid = dl_result.involved_resources[k];
        if (rid >= 0 && rid < MAX_RESOURCES) {
            is_res_deadlocked[rid] = true;
        }
    }

    /* 1. Build Process Nodes (Column on Left) */
    int p_count = state->process_count;
    int p_start_y = 60;
    int p_spacing = (p_count > 1) ? (380 / (p_count > 6 ? p_count : 6)) : 60;
    if (p_spacing < 40) p_spacing = 40;

    for (int i = 0; i < p_count; i++) {
        if (out_graph->node_count >= (MAX_PROCESSES + MAX_RESOURCES)) break;

        RAGNode *node = &out_graph->nodes[out_graph->node_count++];
        node->id = i;
        node->type = RAG_NODE_PROCESS;
        strncpy(node->label, state->processes[i].name, MAX_NAME_LEN - 1);
        node->total_or_status = state->processes[i].status;
        node->pos_x = 120;
        node->pos_y = p_start_y + i * p_spacing;
        node->is_in_deadlock = is_proc_deadlocked[i];
    }

    /* 2. Build Resource Nodes (Column on Right) */
    int r_count = state->resource_count;
    int r_start_y = 70;
    int r_spacing = (r_count > 1) ? (380 / (r_count > 5 ? r_count : 5)) : 70;
    if (r_spacing < 45) r_spacing = 45;

    for (int j = 0; j < r_count; j++) {
        if (out_graph->node_count >= (MAX_PROCESSES + MAX_RESOURCES)) break;

        RAGNode *node = &out_graph->nodes[out_graph->node_count++];
        node->id = j;
        node->type = RAG_NODE_RESOURCE;
        snprintf(node->label, MAX_NAME_LEN, "%.20s (%d)", state->resources[j].name, state->total[j]);
        node->total_or_status = state->total[j];
        node->pos_x = 540;
        node->pos_y = r_start_y + j * r_spacing;
        node->is_in_deadlock = is_res_deadlocked[j];
    }

    /* 3. Build Allocation Edges (Resource -> Process) */
    for (int i = 0; i < state->process_count; i++) {
        for (int j = 0; j < state->resource_count; j++) {
            if (state->allocation[i][j] > 0) {
                if (out_graph->edge_count >= (MAX_PROCESSES * MAX_RESOURCES * 2)) break;

                RAGEdge *edge = &out_graph->edges[out_graph->edge_count++];
                edge->from_id = j;
                edge->from_type = RAG_NODE_RESOURCE;
                edge->to_id = i;
                edge->to_type = RAG_NODE_PROCESS;
                edge->edge_type = RAG_EDGE_ALLOCATION;
                edge->instance_count = state->allocation[i][j];
                edge->is_deadlock_cycle = (is_proc_deadlocked[i] && is_res_deadlocked[j]);
            }
        }
    }

    /* 4. Build Request Edges (Process -> Resource) */
    for (int i = 0; i < state->process_count; i++) {
        for (int j = 0; j < state->resource_count; j++) {
            if (state->outstanding_requests[i][j] > 0) {
                if (out_graph->edge_count >= (MAX_PROCESSES * MAX_RESOURCES * 2)) break;

                RAGEdge *edge = &out_graph->edges[out_graph->edge_count++];
                edge->from_id = i;
                edge->from_type = RAG_NODE_PROCESS;
                edge->to_id = j;
                edge->to_type = RAG_NODE_RESOURCE;
                edge->edge_type = RAG_EDGE_REQUEST;
                edge->instance_count = state->outstanding_requests[i][j];
                edge->is_deadlock_cycle = (is_proc_deadlocked[i] && is_res_deadlocked[j]);
            }
        }
    }

    return ERR_SUCCESS;
}
