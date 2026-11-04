/**
 * @file statistics.h
 * @brief Performance statistics calculation interface for simulation results.
 *
 * Owned by Person 2.
 */

#ifndef STATISTICS_H
#define STATISTICS_H

#include "core/simulation_types.h"

/**
 * @brief Counts the total number of page hits in an array of simulation steps.
 *
 * @param steps Array of SimulationStep structures.
 * @param reference_count Number of steps in the array.
 * @return Total number of hits.
 */
int calculate_total_hits(const SimulationStep steps[], int reference_count);

/**
 * @brief Counts the total number of page faults in an array of simulation steps.
 *
 * @param steps Array of SimulationStep structures.
 * @param reference_count Number of steps in the array.
 * @return Total number of faults.
 */
int calculate_total_faults(const SimulationStep steps[], int reference_count);

/**
 * @brief Calculates the page hit ratio percentage.
 * Formula: (Total Hits / Total References) * 100.0
 *
 * @param total_hits Number of hits.
 * @param total_references Total number of page requests.
 * @return Hit ratio as a percentage (0.0 to 100.0). Returns 0.0 if total_references <= 0.
 */
double calculate_hit_ratio(int total_hits, int total_references);

/**
 * @brief Calculates the page fault ratio percentage.
 * Formula: (Total Faults / Total References) * 100.0
 *
 * @param total_faults Number of faults.
 * @param total_references Total number of page requests.
 * @return Fault ratio as a percentage (0.0 to 100.0). Returns 0.0 if total_references <= 0.
 */
double calculate_fault_ratio(int total_faults, int total_references);

/**
 * @brief Computes and sets total_hits, total_faults, hit_ratio, and fault_ratio on a result.
 *
 * @param result Pointer to SimulationResult to update.
 */
void update_simulation_statistics(SimulationResult *result);

#endif /* STATISTICS_H */

