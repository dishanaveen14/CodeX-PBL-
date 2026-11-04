/**
 * @file input_validation.h
 * @brief Input parsing and validation interface for page references and frame counts.
 *
 * Owned by Person 3.
 */

#ifndef INPUT_VALIDATION_H
#define INPUT_VALIDATION_H

#include "common.h"

/**
 * @brief Status codes representing validation outcomes.
 */
typedef enum {
    VALIDATION_SUCCESS = 0,
    VALIDATION_ERR_EMPTY_STRING,
    VALIDATION_ERR_INVALID_CHARACTER,
    VALIDATION_ERR_NEGATIVE_PAGE,
    VALIDATION_ERR_PAGE_OUT_OF_RANGE,
    VALIDATION_ERR_FRAME_COUNT_TOO_LOW,
    VALIDATION_ERR_FRAME_COUNT_TOO_HIGH,
    VALIDATION_ERR_TOO_MANY_REFERENCES,
    VALIDATION_ERR_NO_MEMORY
} ValidationStatus;

/**
 * @brief Returns a descriptive user-facing message for a validation status code.
 *
 * @param status Validation status code.
 * @return Human-readable explanation.
 */
const char *validation_status_message(ValidationStatus status);

/**
 * @brief Parses and validates a space- or comma-separated page reference string.
 *
 * Allocates dynamic memory for out_references if parsing succeeds.
 * Negative page numbers are rejected because -1 denotes EMPTY_FRAME.
 *
 * @param input_str Raw text entered by the user.
 * @param out_references Output pointer receiving dynamically allocated array of parsed pages.
 * @param out_count Output pointer receiving the count of parsed pages.
 * @return VALIDATION_SUCCESS or specific error code.
 */
ValidationStatus parse_and_validate_reference_string(
    const char *input_str,
    int **out_references,
    int *out_count
);

/**
 * @brief Validates the specified frame count.
 *
 * @param frame_count Requested number of page frames.
 * @return VALIDATION_SUCCESS, or specific frame count error code.
 */
ValidationStatus validate_frame_count(int frame_count);

/**
 * @brief Frees the parsed references array returned by parse_and_validate_reference_string().
 *
 * @param references Dynamically allocated array of integers.
 */
void free_parsed_references(int *references);

#endif /* INPUT_VALIDATION_H */

