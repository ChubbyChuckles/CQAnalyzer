#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "cqanalyzer.h"

/**
 * @file preprocessor.h
 * @brief Preprocessing functionality for include files and macros
 *
 * Provides functions to handle preprocessing directives, include paths,
 * and macro definitions before AST parsing.
 */

/**
 * @brief Include path information
 */
typedef struct
{
    char path[MAX_PATH_LENGTH];
    struct IncludePath *next;
} IncludePath;

/**
 * @brief Macro definition information
 */
typedef struct
{
    char name[MAX_NAME_LENGTH];
    char value[MAX_VALUE_LENGTH];
    struct MacroDefinition *next;
} MacroDefinition;

/**
 * @brief Preprocessing context
 */
typedef struct
{
    IncludePath *include_paths;
    MacroDefinition *macros;
    uint32_t include_count;
    uint32_t macro_count;
} PreprocessingContext;

/**
 * @brief Initialize preprocessing context
 *
 * @return Pointer to new preprocessing context, or NULL on error
 */
PreprocessingContext *preprocessor_init(void);

/**
 * @brief Scan project directory for include paths
 *
 * @param context Preprocessing context
 * @param project_root Root directory of the project
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError preprocessor_scan_includes(PreprocessingContext *context, const char *project_root);

/**
 * @brief Extract macro definitions from source file
 *
 * @param context Preprocessing context
 * @param filepath Path to source file
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError preprocessor_extract_macros(PreprocessingContext *context, const char *filepath);

/**
 * @brief Build command line arguments for libclang
 *
 * @param context Preprocessing context
 * @param args Output array for arguments
 * @param max_args Maximum number of arguments
 * @return Number of arguments added
 */
int preprocessor_build_args(PreprocessingContext *context, const char **args, int max_args);

/**
 * @brief Free preprocessing context
 *
 * @param context Context to free
 */
void preprocessor_free(PreprocessingContext *context);

#endif // PREPROCESSOR_H