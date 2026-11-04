/**
 * @file statistics.c
 * @brief Implementation of simulation statistics calculations.
 *
 * Owned by Person 2.
 */

#include "core/statistics.h"

int calculate_total_hits(const SimulationStep steps[], int reference_count) {
    if (!steps || reference_count <= 0) {
        return 0;
    }

    int hits = 0;
    for (int i = 0; i < reference_count; ++i) {
        if (steps[i].status == PAGE_HIT) {
            hits++;
        }
    }
    return hits;
}

int calculate_total_faults(const SimulationStep steps[], int reference_count) {
    if (!steps || reference_count <= 0) {
        return 0;
    }

    int faults = 0;
    for (int i = 0; i < reference_count; ++i) {
        if (steps[i].status == PAGE_FAULT) {
            faults++;
        }
    }
    return faults;
}

double calculate_hit_ratio(int total_hits, int total_references) {
    if (total_references <= 0 || total_hits <= 0) {
        return 0.0;
    }
    return ((double)total_hits / (double)total_references) * 100.0;
}

double calculate_fault_ratio(int total_faults, int total_references) {
    if (total_references <= 0 || total_faults <= 0) {
        return 0.0;
    }
    return ((double)total_faults / (double)total_references) * 100.0;
}

void update_simulation_statistics(SimulationResult *result) {
    if (!result) {
        return;
    }

    if (result->total_references <= 0 || !result->steps) {
        result->total_hits = 0;
        result->total_faults = 0;
        result->hit_ratio = 0.0;
        result->fault_ratio = 0.0;
        return;
    }

    result->total_hits = calculate_total_hits(result->steps, result->total_references);
    result->total_faults = calculate_total_faults(result->steps, result->total_references);
    result->hit_ratio = calculate_hit_ratio(result->total_hits, result->total_references);
    result->fault_ratio = calculate_fault_ratio(result->total_faults, result->total_references);
}

