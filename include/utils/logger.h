#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include "cqanalyzer.h"

/**
 * @file logger.h
 * @brief Logging system for CQAnalyzer
 *
 * Provides configurable logging functionality with different verbosity levels
 * and output destinations.
 */

// Log levels
typedef enum
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_NONE = 4
} LogLevel;

// Log output destinations
typedef enum
{
    LOG_OUTPUT_CONSOLE = 1 << 0,
    LOG_OUTPUT_FILE = 1 << 1
} LogOutput;

/**
 * @brief Initialize the logging system
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError logger_init(void);

/**
 * @brief Shutdown the logging system
 */
void logger_shutdown(void);

/**
 * @brief Set the minimum log level
 *
 * @param level Minimum log level to output
 */
void logger_set_level(LogLevel level);

/**
 * @brief Set log output destinations
 *
 * @param outputs Bitmask of LogOutput values
 */
void logger_set_outputs(int outputs);

/**
 * @brief Set log file path (for file output)
 *
 * @param filepath Path to log file
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError logger_set_file(const char *filepath);

/**
 * @brief Log a debug message
 *
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void LOG_DEBUG(const char *format, ...);

/**
 * @brief Log an info message
 *
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void LOG_INFO(const char *format, ...);

/**
 * @brief Log a warning message
 *
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void LOG_WARNING(const char *format, ...);

/**
 * @brief Log an error message
 *
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 */
void LOG_ERROR(const char *format, ...);

#endif // LOGGER_H
