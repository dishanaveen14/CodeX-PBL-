/**
 * @file logger.h
 * @brief Application logging and diagnostics utility.
 * 
 * Deadlock Simulation System - PBL Project
 */

#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} LogLevel;

/**
 * @brief Logs a formatted message with a severity level and component tag.
 */
void log_message(LogLevel level, const char *component, const char *format, ...);

#define LOG_DEBUG(comp, fmt, ...) log_message(LOG_LEVEL_DEBUG, comp, fmt, ##__VA_ARGS__)
#define LOG_INFO(comp, fmt, ...)  log_message(LOG_LEVEL_INFO,  comp, fmt, ##__VA_ARGS__)
#define LOG_WARN(comp, fmt, ...)  log_message(LOG_LEVEL_WARN,  comp, fmt, ##__VA_ARGS__)
#define LOG_ERROR(comp, fmt, ...) log_message(LOG_LEVEL_ERROR, comp, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_H */

