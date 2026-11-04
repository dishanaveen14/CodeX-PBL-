/**
 * @file common.h
 * @brief Global definitions and constants for Page Replacement Algorithm Simulator.
 *
 * Project: Simulation and Comparative Analysis of Page Replacement Algorithms
 * Shared header across all modules.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/**
 * @brief Representation of an empty frame slot.
 * Page numbers must be non-negative (>= 0), so -1 unambiguously denotes an empty frame.
 */
#define EMPTY_FRAME (-1)

/**
 * @brief Maximum supported reference string length for sanity checks.
 */
#define MAX_REFERENCES 1024

/**
 * @brief Minimum and maximum allowed physical frame counts.
 */
#define MIN_FRAMES 1
#define MAX_FRAMES 64

/**
 * @brief Standard return status codes across modules.
 */
typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR_INVALID_PARAM = -1,
    STATUS_ERROR_NO_MEMORY = -2,
    STATUS_ERROR_OUT_OF_BOUNDS = -3
} StatusCode;

#endif /* COMMON_H */

