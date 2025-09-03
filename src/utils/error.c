#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils/error.h"
#include "utils/logger.h"
#include "utils/localization.h"

// Global error handler
static CQErrorHandler g_error_handler = NULL;

// Error code to string mappings
static const char *error_code_strings[] = {
    [CQ_ERROR_SUCCESS] = "Success",

    // General errors
    [CQ_ERROR_INVALID_ARGUMENT_CODE] = "Invalid argument provided",
    [CQ_ERROR_NULL_POINTER_CODE] = "Null pointer encountered",
    [CQ_ERROR_BUFFER_OVERFLOW_CODE] = "Buffer overflow detected",
    [CQ_ERROR_INVALID_STATE_CODE] = "Invalid system state",
    [CQ_ERROR_OPERATION_NOT_SUPPORTED_CODE] = "Operation not supported",

    // Parsing errors
    [CQ_ERROR_PARSING_FAILED_CODE] = "Code parsing failed",
    [CQ_ERROR_SYNTAX_ERROR_CODE] = "Syntax error in source code",
    [CQ_ERROR_UNSUPPORTED_LANGUAGE_CODE] = "Unsupported programming language",
    [CQ_ERROR_FILE_FORMAT_INVALID_CODE] = "Invalid file format",
    [CQ_ERROR_ENCODING_UNSUPPORTED_CODE] = "Unsupported text encoding",
    [CQ_ERROR_PARSER_INITIALIZATION_FAILED_CODE] = "Parser initialization failed",
    [CQ_ERROR_AST_GENERATION_FAILED_CODE] = "AST generation failed",
    [CQ_ERROR_TOKENIZATION_FAILED_CODE] = "Source code tokenization failed",

    // Analysis errors
    [CQ_ERROR_ANALYSIS_FAILED_CODE] = "Code analysis failed",
    [CQ_ERROR_METRIC_CALCULATION_FAILED_CODE] = "Metric calculation failed",
    [CQ_ERROR_COMPLEXITY_ANALYSIS_FAILED_CODE] = "Complexity analysis failed",
    [CQ_ERROR_DEPENDENCY_ANALYSIS_FAILED_CODE] = "Dependency analysis failed",
    [CQ_ERROR_CODE_QUALITY_ASSESSMENT_FAILED_CODE] = "Code quality assessment failed",
    [CQ_ERROR_DEAD_CODE_DETECTION_FAILED_CODE] = "Dead code detection failed",
    [CQ_ERROR_DUPLICATION_DETECTION_FAILED_CODE] = "Code duplication detection failed",

    // Visualization errors
    [CQ_ERROR_RENDERING_FAILED_CODE] = "3D rendering failed",
    [CQ_ERROR_OPENGL_INITIALIZATION_FAILED_CODE] = "OpenGL initialization failed",
    [CQ_ERROR_SHADER_COMPILATION_FAILED_CODE] = "Shader compilation failed",
    [CQ_ERROR_TEXTURE_LOADING_FAILED_CODE] = "Texture loading failed",
    [CQ_ERROR_CAMERA_SETUP_FAILED_CODE] = "Camera setup failed",
    [CQ_ERROR_DATA_TRANSFORMATION_FAILED_CODE] = "Data transformation failed",
    [CQ_ERROR_VISUALIZATION_DATA_INVALID_CODE] = "Invalid visualization data",

    // UI errors
    [CQ_ERROR_UI_INITIALIZATION_FAILED_CODE] = "UI initialization failed",
    [CQ_ERROR_WINDOW_CREATION_FAILED_CODE] = "Window creation failed",
    [CQ_ERROR_GUI_LIBRARY_ERROR_CODE] = "GUI library error",
    [CQ_ERROR_INPUT_HANDLING_FAILED_CODE] = "Input handling failed",
    [CQ_ERROR_DIALOG_CREATION_FAILED_CODE] = "Dialog creation failed",

    // Configuration errors
    [CQ_ERROR_CONFIG_INVALID_CODE] = "Invalid configuration",
    [CQ_ERROR_CONFIG_FILE_NOT_FOUND_CODE] = "Configuration file not found",
    [CQ_ERROR_CONFIG_PARSING_FAILED_CODE] = "Configuration parsing failed",
    [CQ_ERROR_CONFIG_VALUE_INVALID_CODE] = "Invalid configuration value",
    [CQ_ERROR_CONFIG_SAVE_FAILED_CODE] = "Configuration save failed",

    // I/O errors
    [CQ_ERROR_FILE_NOT_FOUND_CODE] = "File not found",
    [CQ_ERROR_FILE_ACCESS_DENIED_CODE] = "File access denied",
    [CQ_ERROR_FILE_READ_FAILED_CODE] = "File read failed",
    [CQ_ERROR_FILE_WRITE_FAILED_CODE] = "File write failed",
    [CQ_ERROR_DIRECTORY_NOT_FOUND_CODE] = "Directory not found",
    [CQ_ERROR_PATH_TOO_LONG_CODE] = "Path too long",
    [CQ_ERROR_DISK_FULL_CODE] = "Disk full",

    // Memory errors
    [CQ_ERROR_MEMORY_ALLOCATION_CODE] = "Memory allocation failed",
    [CQ_ERROR_MEMORY_CORRUPTION_CODE] = "Memory corruption detected",
    [CQ_ERROR_OUT_OF_MEMORY_CODE] = "Out of memory",
    [CQ_ERROR_MEMORY_LEAK_DETECTED_CODE] = "Memory leak detected",

    // System errors
    [CQ_ERROR_SYSTEM_CALL_FAILED_CODE] = "System call failed",
    [CQ_ERROR_LIBRARY_NOT_FOUND_CODE] = "Required library not found",
    [CQ_ERROR_DEPENDENCY_MISSING_CODE] = "Missing dependency",
    [CQ_ERROR_PERMISSION_DENIED_CODE] = "Permission denied",
    [CQ_ERROR_RESOURCE_BUSY_CODE] = "Resource busy",
    [CQ_ERROR_TIMEOUT_CODE] = "Operation timeout",

    // Unknown
    [CQ_ERROR_UNKNOWN_CODE] = "Unknown error"
};

// Recovery suggestions for error codes
static const char *recovery_suggestions[] = {
    [CQ_ERROR_SUCCESS] = "",

    // General errors
    [CQ_ERROR_INVALID_ARGUMENT_CODE] = "Check function parameters and ensure they are valid",
    [CQ_ERROR_NULL_POINTER_CODE] = "Ensure pointers are properly initialized before use",
    [CQ_ERROR_BUFFER_OVERFLOW_CODE] = "Increase buffer size or check data length",
    [CQ_ERROR_INVALID_STATE_CODE] = "Check system state before performing operation",
    [CQ_ERROR_OPERATION_NOT_SUPPORTED_CODE] = "Use an alternative approach or check system capabilities",

    // Parsing errors
    [CQ_ERROR_PARSING_FAILED_CODE] = "Check source code syntax and file format",
    [CQ_ERROR_SYNTAX_ERROR_CODE] = "Fix syntax errors in the source code",
    [CQ_ERROR_UNSUPPORTED_LANGUAGE_CODE] = "Use a supported programming language",
    [CQ_ERROR_FILE_FORMAT_INVALID_CODE] = "Ensure file is in correct format",
    [CQ_ERROR_ENCODING_UNSUPPORTED_CODE] = "Convert file to UTF-8 encoding",
    [CQ_ERROR_PARSER_INITIALIZATION_FAILED_CODE] = "Check parser dependencies and configuration",
    [CQ_ERROR_AST_GENERATION_FAILED_CODE] = "Verify libclang installation and source code validity",
    [CQ_ERROR_TOKENIZATION_FAILED_CODE] = "Check for unusual characters or encoding issues",

    // Analysis errors
    [CQ_ERROR_ANALYSIS_FAILED_CODE] = "Verify source code is parseable and accessible",
    [CQ_ERROR_METRIC_CALCULATION_FAILED_CODE] = "Check metric calculation parameters",
    [CQ_ERROR_COMPLEXITY_ANALYSIS_FAILED_CODE] = "Ensure source code is syntactically correct",
    [CQ_ERROR_DEPENDENCY_ANALYSIS_FAILED_CODE] = "Check include paths and dependencies",
    [CQ_ERROR_CODE_QUALITY_ASSESSMENT_FAILED_CODE] = "Verify analysis configuration",
    [CQ_ERROR_DEAD_CODE_DETECTION_FAILED_CODE] = "Ensure source code compiles successfully",
    [CQ_ERROR_DUPLICATION_DETECTION_FAILED_CODE] = "Check file permissions and access",

    // Visualization errors
    [CQ_ERROR_RENDERING_FAILED_CODE] = "Check OpenGL drivers and system requirements",
    [CQ_ERROR_OPENGL_INITIALIZATION_FAILED_CODE] = "Update graphics drivers or check OpenGL version",
    [CQ_ERROR_SHADER_COMPILATION_FAILED_CODE] = "Check shader source code for syntax errors",
    [CQ_ERROR_TEXTURE_LOADING_FAILED_CODE] = "Verify texture file exists and is valid",
    [CQ_ERROR_CAMERA_SETUP_FAILED_CODE] = "Check camera parameters and viewport settings",
    [CQ_ERROR_DATA_TRANSFORMATION_FAILED_CODE] = "Verify data format and transformation logic",
    [CQ_ERROR_VISUALIZATION_DATA_INVALID_CODE] = "Check data preprocessing and validation",

    // UI errors
    [CQ_ERROR_UI_INITIALIZATION_FAILED_CODE] = "Check GUI library installation and dependencies",
    [CQ_ERROR_WINDOW_CREATION_FAILED_CODE] = "Check display settings and window manager",
    [CQ_ERROR_GUI_LIBRARY_ERROR_CODE] = "Update GUI library or check system compatibility",
    [CQ_ERROR_INPUT_HANDLING_FAILED_CODE] = "Check input device connections and drivers",
    [CQ_ERROR_DIALOG_CREATION_FAILED_CODE] = "Verify dialog parameters and system resources",

    // Configuration errors
    [CQ_ERROR_CONFIG_INVALID_CODE] = "Check configuration file syntax and values",
    [CQ_ERROR_CONFIG_FILE_NOT_FOUND_CODE] = "Create configuration file or check path",
    [CQ_ERROR_CONFIG_PARSING_FAILED_CODE] = "Fix configuration file format",
    [CQ_ERROR_CONFIG_VALUE_INVALID_CODE] = "Correct invalid configuration values",
    [CQ_ERROR_CONFIG_SAVE_FAILED_CODE] = "Check file permissions and disk space",

    // I/O errors
    [CQ_ERROR_FILE_NOT_FOUND_CODE] = "Verify file path and existence",
    [CQ_ERROR_FILE_ACCESS_DENIED_CODE] = "Check file permissions",
    [CQ_ERROR_FILE_READ_FAILED_CODE] = "Check file permissions and disk status",
    [CQ_ERROR_FILE_WRITE_FAILED_CODE] = "Check file permissions and disk space",
    [CQ_ERROR_DIRECTORY_NOT_FOUND_CODE] = "Create directory or check path",
    [CQ_ERROR_PATH_TOO_LONG_CODE] = "Use shorter path or relative paths",
    [CQ_ERROR_DISK_FULL_CODE] = "Free up disk space",

    // Memory errors
    [CQ_ERROR_MEMORY_ALLOCATION_CODE] = "Close other applications or increase system memory",
    [CQ_ERROR_MEMORY_CORRUPTION_CODE] = "Restart application or check for memory issues",
    [CQ_ERROR_OUT_OF_MEMORY_CODE] = "Reduce project size or increase system memory",
    [CQ_ERROR_MEMORY_LEAK_DETECTED_CODE] = "Check for memory leaks in application code",

    // System errors
    [CQ_ERROR_SYSTEM_CALL_FAILED_CODE] = "Check system call parameters and permissions",
    [CQ_ERROR_LIBRARY_NOT_FOUND_CODE] = "Install missing libraries or check library path",
    [CQ_ERROR_DEPENDENCY_MISSING_CODE] = "Install required dependencies",
    [CQ_ERROR_PERMISSION_DENIED_CODE] = "Run with appropriate permissions",
    [CQ_ERROR_RESOURCE_BUSY_CODE] = "Wait for resource to become available",
    [CQ_ERROR_TIMEOUT_CODE] = "Increase timeout value or check system performance",

    // Unknown
    [CQ_ERROR_UNKNOWN_CODE] = "Contact support with error details"
};

CQError cq_error_init(void)
{
    // Initialize error system - currently no special initialization needed
    return CQ_SUCCESS;
}

void cq_error_shutdown(void)
{
    // Clean up error system
    g_error_handler = NULL;
}

void cq_error_set_handler(CQErrorHandler handler)
{
    g_error_handler = handler;
}

CQErrorContext *cq_error_create(CQErrorCode code, CQErrorSeverity severity,
                                const char *message, const char *file,
                                int line, const char *function)
{
    CQErrorContext *error = (CQErrorContext *)malloc(sizeof(CQErrorContext));
    if (!error)
    {
        return NULL;
    }

    error->code = code;
    error->category = cq_error_get_category(code);
    error->severity = severity;
    error->line = line;
    error->timestamp = (uint64_t)time(NULL);
    error->user_data = NULL;

    // Copy strings with bounds checking
    if (message)
    {
        strncpy(error->message, message, MAX_ERROR_MESSAGE_LENGTH - 1);
        error->message[MAX_ERROR_MESSAGE_LENGTH - 1] = '\0';
    }
    else
    {
        error->message[0] = '\0';
    }

    if (file)
    {
        strncpy(error->file, file, MAX_PATH_LENGTH - 1);
        error->file[MAX_PATH_LENGTH - 1] = '\0';
    }
    else
    {
        error->file[0] = '\0';
    }

    if (function)
    {
        strncpy(error->function, function, MAX_NAME_LENGTH - 1);
        error->function[MAX_NAME_LENGTH - 1] = '\0';
    }
    else
    {
        error->function[0] = '\0';
    }

    // Initialize optional fields
    error->context_info[0] = '\0';
    error->recovery_suggestion[0] = '\0';

    return error;
}

CQErrorContext *cq_error_create_formatted(CQErrorCode code, CQErrorSeverity severity,
                                         const char *file, int line, const char *function,
                                         const char *format, ...)
{
    char message[MAX_ERROR_MESSAGE_LENGTH];
    va_list args;

    va_start(args, format);
    vsnprintf(message, MAX_ERROR_MESSAGE_LENGTH, format, args);
    va_end(args);

    return cq_error_create(code, severity, message, file, line, function);
}

void cq_error_set_context(CQErrorContext *error, const char *context_info)
{
    if (error && context_info)
    {
        strncpy(error->context_info, context_info, MAX_VALUE_LENGTH - 1);
        error->context_info[MAX_VALUE_LENGTH - 1] = '\0';
    }
}

void cq_error_set_recovery_suggestion(CQErrorContext *error, const char *suggestion)
{
    if (error && suggestion)
    {
        strncpy(error->recovery_suggestion, suggestion, MAX_ERROR_MESSAGE_LENGTH - 1);
        error->recovery_suggestion[MAX_ERROR_MESSAGE_LENGTH - 1] = '\0';
    }
}

void cq_error_report(const CQErrorContext *error)
{
    if (!error)
    {
        return;
    }

    // Log the error based on severity
    switch (error->severity)
    {
    case ERROR_SEVERITY_INFO:
        LOG_INFO("Error [%d]: %s (%s:%d in %s)",
                 error->code, error->message, error->file, error->line, error->function);
        break;
    case ERROR_SEVERITY_WARNING:
        LOG_WARNING("Error [%d]: %s (%s:%d in %s)",
                    error->code, error->message, error->file, error->line, error->function);
        break;
    case ERROR_SEVERITY_ERROR:
    case ERROR_SEVERITY_CRITICAL:
        LOG_ERROR("Error [%d]: %s (%s:%d in %s)",
                  error->code, error->message, error->file, error->line, error->function);
        break;
    }

    // Call custom error handler if set
    if (g_error_handler)
    {
        g_error_handler(error);
    }
}

CQErrorCategory cq_error_get_category(CQErrorCode code)
{
    if (code >= 2000 && code < 3000)
        return ERROR_CATEGORY_PARSING;
    else if (code >= 3000 && code < 4000)
        return ERROR_CATEGORY_ANALYSIS;
    else if (code >= 4000 && code < 5000)
        return ERROR_CATEGORY_VISUALIZATION;
    else if (code >= 5000 && code < 6000)
        return ERROR_CATEGORY_UI;
    else if (code >= 6000 && code < 7000)
        return ERROR_CATEGORY_CONFIG;
    else if (code >= 7000 && code < 8000)
        return ERROR_CATEGORY_IO;
    else if (code >= 8000 && code < 9000)
        return ERROR_CATEGORY_MEMORY;
    else if (code >= 9000 && code < 10000)
        return ERROR_CATEGORY_SYSTEM;
    else
        return ERROR_CATEGORY_GENERAL;
}

CQErrorSeverity cq_error_get_severity(CQErrorCode code)
{
    // Critical errors
    if (code == CQ_ERROR_OUT_OF_MEMORY_CODE || code == CQ_ERROR_MEMORY_CORRUPTION_CODE ||
        code == CQ_ERROR_SYSTEM_CALL_FAILED_CODE || code == CQ_ERROR_PERMISSION_DENIED_CODE)
    {
        return ERROR_SEVERITY_CRITICAL;
    }
    // Error level
    else if (code >= 2000 && code < 10000)
    {
        return ERROR_SEVERITY_ERROR;
    }
    // Warning level for some specific cases
    else if (code == CQ_ERROR_CONFIG_VALUE_INVALID_CODE || code == CQ_ERROR_TIMEOUT_CODE)
    {
        return ERROR_SEVERITY_WARNING;
    }
    // Info level for general issues
    else
    {
        return ERROR_SEVERITY_INFO;
    }
}

const char *cq_error_code_to_string(CQErrorCode code)
{
    // Use localization system for error messages
    return localization_get_error_message(code);
}

const char *cq_error_category_to_string(CQErrorCategory category)
{
    switch (category)
    {
    case ERROR_CATEGORY_GENERAL:
        return "General";
    case ERROR_CATEGORY_PARSING:
        return "Parsing";
    case ERROR_CATEGORY_ANALYSIS:
        return "Analysis";
    case ERROR_CATEGORY_VISUALIZATION:
        return "Visualization";
    case ERROR_CATEGORY_UI:
        return "User Interface";
    case ERROR_CATEGORY_CONFIG:
        return "Configuration";
    case ERROR_CATEGORY_IO:
        return "Input/Output";
    case ERROR_CATEGORY_MEMORY:
        return "Memory";
    case ERROR_CATEGORY_SYSTEM:
        return "System";
    default:
        return "Unknown";
    }
}

const char *cq_error_severity_to_string(CQErrorSeverity severity)
{
    switch (severity)
    {
    case ERROR_SEVERITY_INFO:
        return "Info";
    case ERROR_SEVERITY_WARNING:
        return "Warning";
    case ERROR_SEVERITY_ERROR:
        return "Error";
    case ERROR_SEVERITY_CRITICAL:
        return "Critical";
    default:
        return "Unknown";
    }
}

int cq_error_format_message(const CQErrorContext *error, char *buffer, size_t buffer_size)
{
    if (!error || !buffer || buffer_size == 0)
    {
        return -1;
    }

    int written = snprintf(buffer, buffer_size,
                          "[%s] %s: %s\n"
                          "Location: %s:%d in %s\n"
                          "Category: %s | Severity: %s\n"
                          "Time: %lu",
                          cq_error_category_to_string(error->category),
                          cq_error_code_to_string(error->code),
                          error->message,
                          error->file, error->line, error->function,
                          cq_error_category_to_string(error->category),
                          cq_error_severity_to_string(error->severity),
                          error->timestamp);

    if (error->context_info[0] != '\0')
    {
        written += snprintf(buffer + written, buffer_size - written,
                           "\nContext: %s", error->context_info);
    }

    if (error->recovery_suggestion[0] != '\0')
    {
        written += snprintf(buffer + written, buffer_size - written,
                           "\nSuggestion: %s", error->recovery_suggestion);
    }

    return written;
}

const char *cq_error_get_recovery_suggestion(CQErrorCode code)
{
    // For now, return English recovery suggestions
    // TODO: Add localized recovery suggestions to message catalogs
    if (code >= 0 && code < sizeof(recovery_suggestions) / sizeof(recovery_suggestions[0]))
    {
        return recovery_suggestions[code];
    }
    return recovery_suggestions[CQ_ERROR_UNKNOWN];
}

bool cq_error_is_recoverable(CQErrorCode code)
{
    // Define which errors are recoverable
    switch (code)
    {
    case CQ_ERROR_TIMEOUT_CODE:
    case CQ_ERROR_RESOURCE_BUSY_CODE:
    case CQ_ERROR_CONFIG_VALUE_INVALID_CODE:
    case CQ_ERROR_FILE_ACCESS_DENIED_CODE:
    case CQ_ERROR_DIRECTORY_NOT_FOUND_CODE:
        return true;
    default:
        return false;
    }
}

void cq_error_free(CQErrorContext *error)
{
    if (error)
    {
        free(error);
    }
}