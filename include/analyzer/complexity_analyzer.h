#ifndef COMPLEXITY_ANALYZER_H
#define COMPLEXITY_ANALYZER_H

#include "cqanalyzer.h"

/**
 * @file complexity_analyzer.h
 * @brief Code complexity analysis
 *
 * Provides functions to analyze code complexity using various metrics.
 */

/**
 * @brief Analyze function complexity
 *
 * @param ast_data Parsed AST data for a function
 * @param complexity Output complexity value
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError analyze_function_complexity(void* ast_data, int* complexity);

/**
 * @brief Analyze file complexity
 *
 * @param filepath Source file path
 * @param complexity Output complexity value
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError analyze_file_complexity(const char* filepath, int* complexity);

#endif // COMPLEXITY_ANALYZER_H
