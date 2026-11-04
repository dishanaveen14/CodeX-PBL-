/**
 * @file simulation_types.h
 * @brief Core data structures representing page replacement simulation data.
 *
 * Owned primarily by Person 2 (Optimal & Simulation Engine).
 * Consumed by Person 1 (FIFO/LRU), Person 3 (GUI), and Person 4 (Integration).
 */

#ifndef SIMULATION_TYPES_H
#define SIMULATION_TYPES_H

#include "common.h"

/**
 * @brief Indicates whether a memory page request resulted in a Hit or a Fault.
 */
typedef enum {
    PAGE_HIT,
    PAGE_FAULT
} PageStatus;

/**
 * @brief Represents the state of physical memory frames after a single page reference.
 */
typedef struct {
    int page;            /**< Requested page number */
    PageStatus status;   /**< Hit or Fault status */
    int *frames;         /**< Snapshot array of frame contents of size frame_count; EMPTY_FRAME (-1) if empty */
} SimulationStep;

/**
 * @brief Complete simulation result for one page replacement algorithm execution.
 */
typedef struct {
    char algorithm_name[16]; /**< Name identifier (e.g. "FIFO", "LRU", "Optimal") */

    int total_references;    /**< Total number of page requests processed */
    int total_hits;          /**< Total number of page hits recorded */
    int total_faults;        /**< Total number of page faults recorded */

    double hit_ratio;        /**< Hit ratio percentage: (total_hits / total_references) * 100.0 */
    double fault_ratio;      /**< Fault ratio percentage: (total_faults / total_references) * 100.0 */

    int frame_count;         /**< Number of physical page frames allocated for the simulation */
    SimulationStep *steps;   /**< Array of simulation steps of size total_references */
} SimulationResult;

#endif /* SIMULATION_TYPES_H */

