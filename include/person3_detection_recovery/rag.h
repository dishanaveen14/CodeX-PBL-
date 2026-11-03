/**
 * @file rag.h
 * @brief Person 3: Resource Allocation Graph (RAG) data structures and interfaces.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 3 (Deadlock Detection, Recovery, and RAG)
 * 
 * Note: FUNCTION DECLARATIONS ONLY. Do not add algorithm implementations here.
 */

#ifndef RAG_H
#define RAG_H

#include "../common/types.h"
#include "../common/system_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Resource Allocation Graph Data Structures                                 */
/* ========================================================================= */

/**
 * @brief Distinguishes between Process and Resource graph nodes.
 */
typedef enum {
    RAG_NODE_PROCESS = 0,
    RAG_NODE_RESOURCE
} RAGNodeType;

/**
 * @brief Direction and semantics of an edge in the Resource Allocation Graph.
 */
typedef enum {
    RAG_EDGE_ALLOCATION = 0, /**< Resource -> Process (instances held by process) */
    RAG_EDGE_REQUEST         /**< Process -> Resource (instances currently requested/waiting) */
} RAGEdgeType;

/**
 * @brief Node representation in the Resource Allocation Graph.
 */
typedef struct {
    int id;                      /**< Identifier matching process_id or resource_id */
    RAGNodeType type;            /**< Node type: Process or Resource */
    char label[MAX_NAME_LEN];    /**< Node display name (e.g. "P0" or "R1") */
    int total_or_status;         /**< Total instances (for Resource) or ProcessStatus (for Process) */
    int pos_x;                   /**< GUI layout coordinate X */
    int pos_y;                   /**< GUI layout coordinate Y */
    bool is_in_deadlock;         /**< Highlighting flag for deadlocked nodes */
} RAGNode;

/**
 * @brief Directed edge in the Resource Allocation Graph.
 */
typedef struct {
    int from_id;                 /**< Source node ID */
    RAGNodeType from_type;       /**< Source node type */
    int to_id;                   /**< Destination node ID */
    RAGNodeType to_type;         /**< Destination node type */
    RAGEdgeType edge_type;       /**< Allocation or Request edge */
    int instance_count;          /**< Number of instances represented by this edge */
    bool is_deadlock_cycle;      /**< Highlighting flag for edges participating in deadlock */
} RAGEdge;

/**
 * @brief Complete Resource Allocation Graph representation.
 */
typedef struct {
    RAGNode nodes[MAX_PROCESSES + MAX_RESOURCES];             /**< All graph nodes */
    int node_count;                                           /**< Total count of nodes */
    RAGEdge edges[(MAX_PROCESSES * MAX_RESOURCES) * 2];       /**< All graph edges */
    int edge_count;                                           /**< Total count of edges */
} RAGGraph;

/* ========================================================================= */
/* Function Prototypes                                                       */
/* ========================================================================= */

/**
 * @brief Constructs a complete Resource Allocation Graph model from the current system state.
 * 
 * Generates:
 *  - Process nodes for each active process.
 *  - Resource nodes for each configured resource type.
 *  - Allocation edges from Resource -> Process for non-zero Allocation[i][j].
 *  - Request edges from Process -> Resource for non-zero Outstanding_Requests[i][j].
 * 
 * @param state Pointer to the current system state.
 * @param out_graph Pointer to receive the constructed RAG model.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM if pointers are NULL.
 */
ErrorCode rag_build_from_state(const SystemState *state, RAGGraph *out_graph);

/**
 * @brief Clears and resets a RAGGraph data structure.
 * 
 * @param graph Pointer to RAGGraph to reset.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM if graph is NULL.
 */
ErrorCode rag_reset(RAGGraph *graph);

#ifdef __cplusplus
}
#endif

#endif /* RAG_H */

