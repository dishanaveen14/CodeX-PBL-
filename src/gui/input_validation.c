/**
 * @file input_validation.c
 * @brief Implementation of input validation and parsing.
 *
 * Owned by Person 3.
 */

#include "gui/input_validation.h"
#include "utils/memory.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *validation_status_message(ValidationStatus status) {
    switch (status) {
        case VALIDATION_SUCCESS:
            return "Input is valid.";
        case VALIDATION_ERR_EMPTY_STRING:
            return "Reference string is empty. Please enter positive page numbers.";
        case VALIDATION_ERR_INVALID_CHARACTER:
            return "Reference string contains invalid characters. Use spaces or commas between digits.";
        case VALIDATION_ERR_NEGATIVE_PAGE:
            return "Negative page numbers are not allowed (-1 is reserved for empty frames).";
        case VALIDATION_ERR_PAGE_OUT_OF_RANGE:
            return "Page number value is too large.";
        case VALIDATION_ERR_FRAME_COUNT_TOO_LOW:
            return "Frame count must be at least 1.";
        case VALIDATION_ERR_FRAME_COUNT_TOO_HIGH:
            return "Frame count exceeds maximum supported limit (64).";
        case VALIDATION_ERR_TOO_MANY_REFERENCES:
            return "Reference string exceeds maximum allowed page count (1024).";
        case VALIDATION_ERR_NO_MEMORY:
            return "Memory allocation error during input parsing.";
        default:
            return "Unknown validation error.";
    }
}

ValidationStatus validate_frame_count(int frame_count) {
    if (frame_count < MIN_FRAMES) {
        return VALIDATION_ERR_FRAME_COUNT_TOO_LOW;
    }
    if (frame_count > MAX_FRAMES) {
        return VALIDATION_ERR_FRAME_COUNT_TOO_HIGH;
    }
    return VALIDATION_SUCCESS;
}

void free_parsed_references(int *references) {
    if (references) {
        free(references);
    }
}

ValidationStatus parse_and_validate_reference_string(
    const char *input_str,
    int **out_references,
    int *out_count
) {
    if (!out_references || !out_count) {
        return VALIDATION_ERR_EMPTY_STRING;
    }
    *out_references = NULL;
    *out_count = 0;

    if (!input_str) {
        return VALIDATION_ERR_EMPTY_STRING;
    }

    /* Check for all-whitespace string */
    const char *p = input_str;
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    if (*p == '\0') {
        return VALIDATION_ERR_EMPTY_STRING;
    }

    int capacity = 32;
    int count = 0;
    int *refs = (int *)safe_malloc((size_t)capacity * sizeof(int));
    if (!refs) {
        return VALIDATION_ERR_NO_MEMORY;
    }

    p = input_str;
    while (*p) {
        /* Skip leading whitespace or comma separators */
        while (*p && (isspace((unsigned char)*p) || *p == ',')) {
            p++;
        }
        if (*p == '\0') {
            break;
        }

        /* Check for negative sign */
        if (*p == '-') {
            free(refs);
            return VALIDATION_ERR_NEGATIVE_PAGE;
        }

        /* Must be a digit */
        if (!isdigit((unsigned char)*p)) {
            free(refs);
            return VALIDATION_ERR_INVALID_CHARACTER;
        }

        char *endptr = NULL;
        errno = 0;
        long val = strtol(p, &endptr, 10);
        if (errno == ERANGE || val < 0 || val > INT_MAX) {
            free(refs);
            return VALIDATION_ERR_PAGE_OUT_OF_RANGE;
        }

        if (count >= MAX_REFERENCES) {
            free(refs);
            return VALIDATION_ERR_TOO_MANY_REFERENCES;
        }

        if (count >= capacity) {
            capacity *= 2;
            int *new_refs = (int *)realloc(refs, (size_t)capacity * sizeof(int));
            if (!new_refs) {
                free(refs);
                return VALIDATION_ERR_NO_MEMORY;
            }
            refs = new_refs;
        }

        refs[count++] = (int)val;
        p = endptr;
    }

    if (count == 0) {
        free(refs);
        return VALIDATION_ERR_EMPTY_STRING;
    }

    *out_references = refs;
    *out_count = count;
    return VALIDATION_SUCCESS;
}

