#ifndef DEAD_CODE_DETECTOR_H
#define DEAD_CODE_DETECTOR_H

#include "cqanalyzer.h"
#include "data/ast_types.h"

/**
 * @file dead_code_detector.h
 * @brief Dead code detection functionality
 *
 * Provides functions to detect unused functions and variables in source code.
 */

/**
 * @brief Dead code detection result
 */
typedef struct {
    char symbol_name[MAX_NAME_LENGTH];
    char symbol_type[MAX_NAME_LENGTH]; // "function", "variable", etc.
    SourceLocation location;
} DeadCodeResult;

/**
 * @brief List of dead code results
 */
typedef struct {
    DeadCodeResult *results;
    uint32_t count;
    uint32_t capacity;
} DeadCodeList;

/**
 * @brief Detect dead code in a source file
 *
 * @param filepath Source file path
 * @param dead_code_list Output list of dead code results
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError detect_dead_code_in_file(const char *filepath, DeadCodeList *dead_code_list);

/**
 * @brief Detect dead code in a project
 *
 * @param project_root Project root directory
 * @param dead_code_list Output list of dead code results
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError detect_dead_code_in_project(const char *project_root, DeadCodeList *dead_code_list);

/**
 * @brief Free dead code list memory
 *
 * @param dead_code_list Dead code list to free
 */
void free_dead_code_list(DeadCodeList *dead_code_list);

#endif // DEAD_CODE_DETECTOR_H