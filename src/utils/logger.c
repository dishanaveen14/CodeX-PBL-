/**
 * @file logger.c
 * @brief Application logging implementation.
 * 
 * Deadlock Simulation System - PBL Project
 */

#include "../../include/utils/logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static const char* level_to_str(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        default:              return "LOG";
    }
}

void log_message(LogLevel level, const char *component, const char *format, ...) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_buf[20];
    if (t) {
        strftime(time_buf, sizeof(time_buf), "%H:%M:%S", t);
    } else {
        snprintf(time_buf, sizeof(time_buf), "00:00:00");
    }

    FILE *out = (level == LOG_LEVEL_ERROR) ? stderr : stdout;

    fprintf(out, "[%s] [%s] [%s] ", time_buf, level_to_str(level), component ? component : "APP");

    va_list args;
    va_start(args, format);
    vfprintf(out, format, args);
    va_end(args);

    fprintf(out, "\n");
    fflush(out);
}

