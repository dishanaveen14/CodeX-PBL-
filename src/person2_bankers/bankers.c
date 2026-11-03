/**
 * @file bankers.c
 * @brief Person 2: Banker's Algorithm and Safe-State Analysis Implementation.
 * 
 * Deadlock Simulation System - PBL Project
 * Assigned to: Person 2 (Banker's Algorithm and Safe-State Analysis)
 */

#include "../../include/person2_bankers/bankers.h"
#include <string.h>
#include <stdio.h>

void banker_format_safe_sequence(const SystemState *state, const SafeSequence *sequence, char *out_str, size_t max_len) {
    if (!sequence || !out_str || max_len == 0) return;

    if (!sequence->is_valid || sequence->length == 0) {
        snprintf(out_str, max_len, "(None)");
        return;
    }

    out_str[0] = '\0';
    size_t offset = 0;

    for (int i = 0; i < sequence->length; i++) {
        int pid = sequence->sequence[i];
        const char *pname = (state && pid < state->process_count) ? state->processes[pid].name : "P?";

        if (i == 0) {
            offset += snprintf(out_str + offset, max_len - offset, "%s", pname);
        } else {
            offset += snprintf(out_str + offset, max_len - offset, " -> %s", pname);
        }

        if (offset >= max_len - 1) break;
    }
}

ErrorCode banker_run_safety_check(const SystemState *state, BankerSafetyResult *out_result) {
    if (state == NULL || out_result == NULL) {
        return ERR_INVALID_PARAM;
    }

    memset(out_result, 0, sizeof(BankerSafetyResult));

    if (state->process_count == 0) {
        out_result->is_safe = true;
        out_result->safe_sequence.is_valid = true;
        out_result->safe_sequence.length = 0;
        strncpy(out_result->explanation, "No active processes. System is trivially safe.", MAX_DESC_LEN - 1);
        return ERR_SUCCESS;
    }

    /* 1. Initialize Work vector as a copy of Available */
    int work[MAX_RESOURCES];
    for (int j = 0; j < state->resource_count; j++) {
        work[j] = state->available[j];
    }

    /* 2. Initialize Finish boolean vector */
    bool finish[MAX_PROCESSES];
    for (int i = 0; i < state->process_count; i++) {
        if (state->processes[i].status == PROCESS_STATUS_TERMINATED) {
            finish[i] = true; /* Already completed/terminated */
        } else {
            finish[i] = false;
        }
    }

    int seq_idx = 0;
    out_result->step_count = 0;

    /* 3. Search for an unfinished process whose Need <= Work */
    while (1) {
        bool progress = false;

        for (int i = 0; i < state->process_count; i++) {
            if (!finish[i]) {
                bool can_finish = true;
                for (int j = 0; j < state->resource_count; j++) {
                    if (state->need[i][j] > work[j]) {
                        can_finish = false;
                        break;
                    }
                }

                if (can_finish) {
                    /* Record educational trace step */
                    if (out_result->step_count < MAX_STEPS) {
                        BankerWorkStep *st = &out_result->steps[out_result->step_count];
                        st->process_id = i;
                        st->can_finish = true;
                        memcpy(st->work_before, work, sizeof(int) * state->resource_count);

                        /* Reclaim allocation into Work vector */
                        for (int j = 0; j < state->resource_count; j++) {
                            work[j] += state->allocation[i][j];
                        }
                        memcpy(st->work_after, work, sizeof(int) * state->resource_count);

                        char work_str[64] = "";
                        for (int j = 0; j < state->resource_count; j++) {
                            char temp[16];
                            snprintf(temp, sizeof(temp), "%d ", work[j]);
                            strcat(work_str, temp);
                        }

                        snprintf(st->step_description, MAX_DESC_LEN,
                            "Process %s finishes (Need <= Work). Reclaiming held resources. New Work = [%s].",
                            state->processes[i].name, work_str);
                        out_result->step_count++;
                    } else {
                        for (int j = 0; j < state->resource_count; j++) {
                            work[j] += state->allocation[i][j];
                        }
                    }

                    finish[i] = true;
                    out_result->safe_sequence.sequence[seq_idx++] = i;
                    progress = true;
                    break; /* Restart loop to find next finishable process in deterministic order */
                }
            }
        }

        if (!progress) {
            break;
        }
    }

    /* 4. Determine if every process was able to finish */
    bool all_finished = true;
    out_result->unfinished_count = 0;

    for (int i = 0; i < state->process_count; i++) {
        if (!finish[i]) {
            all_finished = false;
            out_result->unfinished_processes[out_result->unfinished_count++] = i;
        }
    }

    if (all_finished) {
        out_result->is_safe = true;
        out_result->safe_sequence.length = seq_idx;
        out_result->safe_sequence.is_valid = true;

        char seq_str[128];
        banker_format_safe_sequence(state, &out_result->safe_sequence, seq_str, sizeof(seq_str));
        snprintf(out_result->explanation, MAX_DESC_LEN,
            "State is SAFE. Complete safe execution sequence: <%s>. No deadlock can occur under maximum claims.",
            seq_str);
    } else {
        out_result->is_safe = false;
        out_result->safe_sequence.length = 0;
        out_result->safe_sequence.is_valid = false;

        char unfin_str[128] = "";
        for (int k = 0; k < out_result->unfinished_count; k++) {
            int pid = out_result->unfinished_processes[k];
            char temp[32];
            snprintf(temp, sizeof(temp), "%s%s", (k > 0) ? ", " : "", state->processes[pid].name);
            strcat(unfin_str, temp);
        }

        snprintf(out_result->explanation, MAX_DESC_LEN,
            "State is UNSAFE. No safe sequence exists. Unfinishable processes: [%s]. Deadlock is possible if remaining demand is requested.",
            unfin_str);
    }

    return ERR_SUCCESS;
}

ErrorCode banker_find_safe_sequence(const SystemState *state, SafeSequence *out_sequence) {
    if (state == NULL || out_sequence == NULL) {
        return ERR_INVALID_PARAM;
    }

    BankerSafetyResult res;
    ErrorCode err = banker_run_safety_check(state, &res);
    if (err != ERR_SUCCESS) {
        return err;
    }

    if (res.is_safe) {
        *out_sequence = res.safe_sequence;
        return ERR_SUCCESS;
    } else {
        out_sequence->is_valid = false;
        out_sequence->length = 0;
        return ERR_SYSTEM_UNSAFE;
    }
}

ErrorCode banker_evaluate_resource_request(const SystemState *state, 
                                          int process_id, 
                                          const int *request_vector, 
                                          BankerRequestResult *out_result) {
    if (state == NULL || request_vector == NULL || out_result == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (process_id < 0 || process_id >= state->process_count) {
        return ERR_PROCESS_NOT_FOUND;
    }

    memset(out_result, 0, sizeof(BankerRequestResult));

    /* Step 1: Validate Request <= Need */
    for (int j = 0; j < state->resource_count; j++) {
        if (request_vector[j] < 0) {
            out_result->is_granted = false;
            strncpy(out_result->reason, "Negative request quantities are invalid.", MAX_DESC_LEN - 1);
            return ERR_INVALID_PARAM;
        }

        if (request_vector[j] > state->need[process_id][j]) {
            out_result->is_granted = false;
            snprintf(out_result->reason, MAX_DESC_LEN,
                "Request exceeds declared Need for resource %s (Requested: %d, Remaining Need: %d).",
                state->resources[j].name, request_vector[j], state->need[process_id][j]);
            return ERR_EXCEEDS_NEED;
        }
    }

    /* Step 2: Validate Request <= Available */
    for (int j = 0; j < state->resource_count; j++) {
        if (request_vector[j] > state->available[j]) {
            out_result->is_granted = false;
            snprintf(out_result->reason, MAX_DESC_LEN,
                "Process must wait: Insufficient available instances of resource %s (Requested: %d, Available: %d).",
                state->resources[j].name, request_vector[j], state->available[j]);
            return ERR_INSUFFICIENT_AVAILABLE;
        }
    }

    /* Step 3: Construct trial state */
    SystemState trial_state;
    system_state_clone(state, &trial_state);

    for (int j = 0; j < state->resource_count; j++) {
        trial_state.available[j] -= request_vector[j];
        trial_state.allocation[process_id][j] += request_vector[j];
        trial_state.need[process_id][j] -= request_vector[j];
    }

    /* Step 4: Run safety algorithm on trial state */
    ErrorCode err = banker_run_safety_check(&trial_state, &out_result->trial_result);
    if (err != ERR_SUCCESS) {
        out_result->is_granted = false;
        strncpy(out_result->reason, "Error executing safety algorithm on trial state.", MAX_DESC_LEN - 1);
        return err;
    }

    if (out_result->trial_result.is_safe) {
        out_result->is_granted = true;
        char seq_str[128];
        banker_format_safe_sequence(&trial_state, &out_result->trial_result.safe_sequence, seq_str, sizeof(seq_str));
        snprintf(out_result->reason, MAX_DESC_LEN,
            "Request APPROVED. Trial state is SAFE with execution sequence: <%s>.", seq_str);
        return ERR_SUCCESS;
    } else {
        out_result->is_granted = false;
        snprintf(out_result->reason, MAX_DESC_LEN,
            "Request DENIED. Granting request would transition system into an UNSAFE state. Live state preserved.");
        return ERR_SYSTEM_UNSAFE;
    }
}

const char* banker_get_safety_explanation(const BankerSafetyResult *result) {
    if (result == NULL) {
        return "No safety result provided.";
    }
    return result->explanation;
}
