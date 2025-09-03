#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include "cqanalyzer.h"

/**
 * @file error.h
 * @brief Comprehensive error handling system for CQAnalyzer
 *
 * This module provides a robust error handling framework with:
 * - Categorized error codes
 * - Severity levels
 * - Error context and stack trace collection
 * - Formatted error messages
 * - Recovery suggestions
 * - Integration with logging system
 */

// Error severity levels
typedef enum
{
    ERROR_SEVERITY_INFO = 0,
    ERROR_SEVERITY_WARNING = 1,
    ERROR_SEVERITY_ERROR = 2,
    ERROR_SEVERITY_CRITICAL = 3
} CQErrorSeverity;

// Error categories
typedef enum
{
    ERROR_CATEGORY_GENERAL = 0,
    ERROR_CATEGORY_PARSING = 1,
    ERROR_CATEGORY_ANALYSIS = 2,
    ERROR_CATEGORY_VISUALIZATION = 3,
    ERROR_CATEGORY_UI = 4,
    ERROR_CATEGORY_CONFIG = 5,
    ERROR_CATEGORY_IO = 6,
    ERROR_CATEGORY_MEMORY = 7,
    ERROR_CATEGORY_SYSTEM = 8
} CQErrorCategory;

// Comprehensive error codes (using different names to avoid conflicts)
typedef enum
{
    // Success
    CQ_ERROR_SUCCESS = 0,

    // General errors (1000-1999)
    CQ_ERROR_INVALID_ARGUMENT_CODE = 1001,
    CQ_ERROR_NULL_POINTER_CODE = 1002,
    CQ_ERROR_BUFFER_OVERFLOW_CODE = 1003,
    CQ_ERROR_INVALID_STATE_CODE = 1004,
    CQ_ERROR_OPERATION_NOT_SUPPORTED_CODE = 1005,

    // Parsing errors (2000-2999)
    CQ_ERROR_PARSING_FAILED_CODE = 2001,
    CQ_ERROR_SYNTAX_ERROR_CODE = 2002,
    CQ_ERROR_UNSUPPORTED_LANGUAGE_CODE = 2003,
    CQ_ERROR_FILE_FORMAT_INVALID_CODE = 2004,
    CQ_ERROR_ENCODING_UNSUPPORTED_CODE = 2005,
    CQ_ERROR_PARSER_INITIALIZATION_FAILED_CODE = 2006,
    CQ_ERROR_AST_GENERATION_FAILED_CODE = 2007,
    CQ_ERROR_TOKENIZATION_FAILED_CODE = 2008,

    // Analysis errors (3000-3999)
    CQ_ERROR_ANALYSIS_FAILED_CODE = 3001,
    CQ_ERROR_METRIC_CALCULATION_FAILED_CODE = 3002,
    CQ_ERROR_COMPLEXITY_ANALYSIS_FAILED_CODE = 3003,
    CQ_ERROR_DEPENDENCY_ANALYSIS_FAILED_CODE = 3004,
    CQ_ERROR_CODE_QUALITY_ASSESSMENT_FAILED_CODE = 3005,
    CQ_ERROR_DEAD_CODE_DETECTION_FAILED_CODE = 3006,
    CQ_ERROR_DUPLICATION_DETECTION_FAILED_CODE = 3007,

    // Visualization errors (4000-4999)
    CQ_ERROR_RENDERING_FAILED_CODE = 4001,
    CQ_ERROR_OPENGL_INITIALIZATION_FAILED_CODE = 4002,
    CQ_ERROR_SHADER_COMPILATION_FAILED_CODE = 4003,
    CQ_ERROR_TEXTURE_LOADING_FAILED_CODE = 4004,
    CQ_ERROR_CAMERA_SETUP_FAILED_CODE = 4005,
    CQ_ERROR_DATA_TRANSFORMATION_FAILED_CODE = 4006,
    CQ_ERROR_VISUALIZATION_DATA_INVALID_CODE = 4007,

    // UI errors (5000-5999)
    CQ_ERROR_UI_INITIALIZATION_FAILED_CODE = 5001,
    CQ_ERROR_WINDOW_CREATION_FAILED_CODE = 5002,
    CQ_ERROR_GUI_LIBRARY_ERROR_CODE = 5003,
    CQ_ERROR_INPUT_HANDLING_FAILED_CODE = 5004,
    CQ_ERROR_DIALOG_CREATION_FAILED_CODE = 5005,

    // Configuration errors (6000-6999)
    CQ_ERROR_CONFIG_INVALID_CODE = 6001,
    CQ_ERROR_CONFIG_FILE_NOT_FOUND_CODE = 6002,
    CQ_ERROR_CONFIG_PARSING_FAILED_CODE = 6003,
    CQ_ERROR_CONFIG_VALUE_INVALID_CODE = 6004,
    CQ_ERROR_CONFIG_SAVE_FAILED_CODE = 6005,

    // I/O errors (7000-7999)
    CQ_ERROR_FILE_NOT_FOUND_CODE = 7001,
    CQ_ERROR_FILE_ACCESS_DENIED_CODE = 7002,
    CQ_ERROR_FILE_READ_FAILED_CODE = 7003,
    CQ_ERROR_FILE_WRITE_FAILED_CODE = 7004,
    CQ_ERROR_DIRECTORY_NOT_FOUND_CODE = 7005,
    CQ_ERROR_PATH_TOO_LONG_CODE = 7006,
    CQ_ERROR_DISK_FULL_CODE = 7007,

    // Memory errors (8000-8999)
    CQ_ERROR_MEMORY_ALLOCATION_CODE = 8001,
    CQ_ERROR_MEMORY_CORRUPTION_CODE = 8002,
    CQ_ERROR_OUT_OF_MEMORY_CODE = 8003,
    CQ_ERROR_MEMORY_LEAK_DETECTED_CODE = 8004,

    // System errors (9000-9999)
    CQ_ERROR_SYSTEM_CALL_FAILED_CODE = 9001,
    CQ_ERROR_LIBRARY_NOT_FOUND_CODE = 9002,
    CQ_ERROR_DEPENDENCY_MISSING_CODE = 9003,
    CQ_ERROR_PERMISSION_DENIED_CODE = 9004,
    CQ_ERROR_RESOURCE_BUSY_CODE = 9005,
    CQ_ERROR_TIMEOUT_CODE = 9006,

    // Unknown error
    CQ_ERROR_UNKNOWN_CODE = 9999
} CQErrorCode;

// Error context structure
typedef struct
{
    CQErrorCode code;
    CQErrorCategory category;
    CQErrorSeverity severity;
    char message[MAX_ERROR_MESSAGE_LENGTH];
    char file[MAX_PATH_LENGTH];
    int line;
    char function[MAX_NAME_LENGTH];
    char context_info[MAX_VALUE_LENGTH];
    char recovery_suggestion[MAX_ERROR_MESSAGE_LENGTH];
    uint64_t timestamp;
    void *user_data;
} CQErrorContext;

// Error handler function type
typedef void (*CQErrorHandler)(const CQErrorContext *error);

// Recovery action types
typedef enum
{
    RECOVERY_RETRY = 0,
    RECOVERY_SKIP = 1,
    RECOVERY_ABORT = 2,
    RECOVERY_FALLBACK = 3,
    RECOVERY_IGNORE = 4
} CQRecoveryAction;

/**
 * @brief Initialize the error handling system
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError cq_error_init(void);

/**
 * @brief Shutdown the error handling system
 */
void cq_error_shutdown(void);

/**
 * @brief Set the global error handler
 *
 * @param handler Error handler function
 */
void cq_error_set_handler(CQErrorHandler handler);

/**
 * @brief Create an error context
 *
 * @param code Error code
 * @param severity Error severity
 * @param message Error message
 * @param file Source file where error occurred
 * @param line Line number where error occurred
 * @param function Function where error occurred
 * @return Pointer to error context, or NULL on failure
 */
CQErrorContext *cq_error_create(CQErrorCode code, CQErrorSeverity severity,
                                const char *message, const char *file,
                                int line, const char *function);

/**
 * @brief Create an error context with formatted message
 *
 * @param code Error code
 * @param severity Error severity
 * @param file Source file where error occurred
 * @param line Line number where error occurred
 * @param function Function where error occurred
 * @param format Format string (printf-style)
 * @param ... Variable arguments
 * @return Pointer to error context, or NULL on failure
 */
CQErrorContext *cq_error_create_formatted(CQErrorCode code, CQErrorSeverity severity,
                                         const char *file, int line, const char *function,
                                         const char *format, ...);

/**
 * @brief Set additional context information for an error
 *
 * @param error Error context
 * @param context_info Additional context information
 */
void cq_error_set_context(CQErrorContext *error, const char *context_info);

/**
 * @brief Set recovery suggestion for an error
 *
 * @param error Error context
 * @param suggestion Recovery suggestion
 */
void cq_error_set_recovery_suggestion(CQErrorContext *error, const char *suggestion);

/**
 * @brief Report an error (logs and calls handler)
 *
 * @param error Error context
 */
void cq_error_report(const CQErrorContext *error);

/**
 * @brief Get error category from error code
 *
 * @param code Error code
 * @return Error category
 */
CQErrorCategory cq_error_get_category(CQErrorCode code);

/**
 * @brief Get error severity from error code
 *
 * @param code Error code
 * @return Error severity
 */
CQErrorSeverity cq_error_get_severity(CQErrorCode code);

/**
 * @brief Convert error code to string
 *
 * @param code Error code
 * @return Error string description
 */
const char *cq_error_code_to_string(CQErrorCode code);

/**
 * @brief Convert error category to string
 *
 * @param category Error category
 * @return Category string
 */
const char *cq_error_category_to_string(CQErrorCategory category);

/**
 * @brief Convert error severity to string
 *
 * @param severity Error severity
 * @return Severity string
 */
const char *cq_error_severity_to_string(CQErrorSeverity severity);

/**
 * @brief Format error message with context
 *
 * @param error Error context
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Number of characters written, or -1 on error
 */
int cq_error_format_message(const CQErrorContext *error, char *buffer, size_t buffer_size);

/**
 * @brief Get recovery suggestion for error code
 *
 * @param code Error code
 * @return Recovery suggestion string
 */
const char *cq_error_get_recovery_suggestion(CQErrorCode code);

/**
 * @brief Check if error is recoverable
 *
 * @param code Error code
 * @return true if recoverable, false otherwise
 */
bool cq_error_is_recoverable(CQErrorCode code);

/**
 * @brief Free error context
 *
 * @param error Error context to free
 */
void cq_error_free(CQErrorContext *error);

// Convenience macros for error creation
#define CQ_ERROR_CREATE(code, severity, message) \
    cq_error_create(code, severity, message, __FILE__, __LINE__, __func__)

#define CQ_ERROR_CREATEF(code, severity, format, ...) \
    cq_error_create_formatted(code, severity, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)

// Legacy compatibility - handled in cqanalyzer.c

#endif // ERROR_H