#ifndef CQANALYZER_H
#define CQANALYZER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @file cqanalyzer.h
 * @brief Main header file for CQAnalyzer
 *
 * This file contains core definitions, constants, and type declarations
 * used throughout the CQAnalyzer application.
 */

// Version information
#define CQANALYZER_VERSION "1.0.0"
#define CQANALYZER_MAJOR_VERSION 1
#define CQANALYZER_MINOR_VERSION 0
#define CQANALYZER_PATCH_VERSION 0

// Common constants
#define MAX_PATH_LENGTH 4096
#define MAX_NAME_LENGTH 256
#define MAX_VALUE_LENGTH 1024
#define MAX_ERROR_MESSAGE_LENGTH 512

// Error codes
typedef enum
{
    CQ_SUCCESS = 0,
    CQ_ERROR_INVALID_ARGUMENT = -1,
    CQ_ERROR_FILE_NOT_FOUND = -2,
    CQ_ERROR_MEMORY_ALLOCATION = -3,
    CQ_ERROR_PARSING_FAILED = -4,
    CQ_ERROR_ANALYSIS_FAILED = -5,
    CQ_ERROR_RENDERING_FAILED = -6,
    CQ_ERROR_CONFIG_INVALID = -7,
    CQ_ERROR_UNKNOWN = -99
} CQError;

// Language support enumeration
typedef enum
{
    LANG_C,
    LANG_CPP,
    LANG_JAVA,
    LANG_PYTHON,
    LANG_JAVASCRIPT,
    LANG_TYPESCRIPT,
    LANG_UNKNOWN
} SupportedLanguage;

// Core structures are defined in their respective header files

// Common data structures
typedef struct
{
    char project_path[MAX_PATH_LENGTH];
    SupportedLanguage language;
    bool enable_visualization;
    bool enable_metrics[32]; // Bitfield for different metrics
    int verbosity_level;
    char output_path[MAX_PATH_LENGTH];
    bool show_help;
    bool show_version;
} CLIArgs;

// Function declarations for core functionality
const char *cq_error_to_string(CQError error);
const char *language_to_string(SupportedLanguage lang);

#endif // CQANALYZER_H
