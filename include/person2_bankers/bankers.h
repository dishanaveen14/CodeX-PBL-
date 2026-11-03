/**
 * @file bankers.h
 * @brief Person 2: Banker's Algorithm and Safe-State Analysis interface declarations.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 2 (Banker's Algorithm and Safe-State Analysis)
 */

#ifndef BANKERS_H
#define BANKERS_H

#include "../common/types.h"
#include "../common/system_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Banker's Algorithm Data Structures                                        */
/* ========================================================================= */

/**
 * @brief Representation of an ordered safe sequence of process IDs.
 */
typedef struct {
    int sequence[MAX_PROCESSES];         /**< Process IDs in execution order */
    int length;                          /**< Number of processes in the sequence */
    bool is_valid;                       /**< True if a complete safe sequence was found */
} SafeSequence;

/**
 * @brief Trace step recording an intermediate state of the safety algorithm.
 * 
 * Useful for the GUI step-by-step educational breakdown.
 */
typedef struct {
    int process_id;                      /**< Process evaluated in this step */
    bool can_finish;                     /**< Whether Need <= Work */
    int work_before[MAX_RESOURCES];      /**< Work vector before process completed */
    int work_after[MAX_RESOURCES];       /**< Work vector after process allocation reclaimed */
    char step_description[MAX_DESC_LEN]; /**< Human-readable explanation of this step */
} BankerWorkStep;

/**
 * @brief Structured outcome of a safety check run.
 */
typedef struct {
    bool is_safe;                        /**< True if state is SAFE, false if UNSAFE */
    SafeSequence safe_sequence;          /**< The safe sequence if is_safe is true */
    int unfinished_processes[MAX_PROCESSES]; /**< IDs of processes unable to complete */
    int unfinished_count;                /**< Count of unfinished processes */
    BankerWorkStep steps[MAX_STEPS];     /**< Step-by-step trace of Work vector */
    int step_count;                      /**< Number of recorded steps */
    char explanation[MAX_DESC_LEN];      /**< Summary explanation of safe/unsafe result */
} BankerSafetyResult;

/**
 * @brief Structured outcome of evaluating a proposed resource request.
 */
typedef struct {
    bool is_granted;                     /**< True if request is safe and can be granted */
    char reason[MAX_DESC_LEN];           /**< Explanation if rejected, or grant message */
    BankerSafetyResult trial_result;     /**< Safety result of the trial state */
} BankerRequestResult;

/* ========================================================================= */
/* Function Prototypes                                                       */
/* ========================================================================= */

/**
 * @brief Executes the Banker's safety check on the provided system state.
 * 
 * Invariants:
 *  - Does NOT mutate the live system state.
 *  - Operates on a working copy of Available and Finish vectors.
 * 
 * @param state Pointer to the system state to evaluate.
 * @param out_result Pointer to receive structured safety analysis results.
 * @return ERR_SUCCESS on success, ERR_INVALID_PARAM if pointers are NULL.
 */
ErrorCode banker_run_safety_check(const SystemState *state, BankerSafetyResult *out_result);

/**
 * @brief Computes a safe execution sequence if one exists.
 * 
 * @param state Pointer to the system state.
 * @param out_sequence Pointer to receive the computed safe sequence.
 * @return ERR_SUCCESS if safe sequence found, ERR_SYSTEM_UNSAFE if none exists,
 *         or ERR_INVALID_PARAM.
 */
ErrorCode banker_find_safe_sequence(const SystemState *state, SafeSequence *out_sequence);

/**
 * @brief Evaluates whether a proposed resource request by a process can be granted safely.
 * 
 * Steps:
 *  1. Validates Request <= Need.
 *  2. Validates Request <= Available.
 *  3. Constructs a trial state (Available -= Req, Alloc += Req, Need -= Req).
 *  4. Evaluates safety of trial state.
 *  5. Reverts trial state (live state is NEVER modified).
 * 
 * @param state Pointer to the current system state.
 * @param process_id Requesting process ID index.
 * @param request_vector Array of requested instances for each resource type.
 * @param out_result Pointer to receive the evaluation outcome.
 * @return ERR_SUCCESS on success, or error code on failure.
 */
ErrorCode banker_evaluate_resource_request(const SystemState *state, 
                                          int process_id, 
                                          const int *request_vector, 
                                          BankerRequestResult *out_result);

/**
 * @brief Retrieves a human-readable explanation string from a safety result.
 * 
 * @param result Pointer to BankerSafetyResult.
 * @return String explanation pointer.
 */
const char* banker_get_safety_explanation(const BankerSafetyResult *result);

/**
 * @brief Formats a safe sequence into a readable string (e.g. "P1 -> P3 -> P4 -> P0 -> P2").
 * 
 * @param state Pointer to the system state (for process names).
 * @param sequence Pointer to SafeSequence.
 * @param out_str Buffer to write formatted text into.
 * @param max_len Size of buffer.
 */
void banker_format_safe_sequence(const SystemState *state, const SafeSequence *sequence, char *out_str, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* BANKERS_H */


