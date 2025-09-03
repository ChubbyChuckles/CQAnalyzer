#ifndef METRIC_CALCULATOR_H
#define METRIC_CALCULATOR_H

#include "cqanalyzer.h"

/**
 * @file metric_calculator.h
 * @brief Code quality metric calculation
 *
 * Provides functions to calculate various code quality metrics
 * from parsed source code.
 */

/**
 * @brief Calculate cyclomatic complexity
 *
 * @param ast_data Parsed AST data
 * @return Cyclomatic complexity value
 */
int calculate_cyclomatic_complexity(void *ast_data);

/**
 * @brief Calculate lines of code metrics
 *
 * @param filepath Source file path
 * @param physical_loc Physical lines of code
 * @param logical_loc Logical lines of code
 * @param comment_loc Comment lines of code
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError calculate_lines_of_code(const char *filepath, int *physical_loc,
                                int *logical_loc, int *comment_loc);

/**
 * @brief Calculate maintainability index
 *
 * @param complexity Cyclomatic complexity
 * @param loc Lines of code
 * @param comment_ratio Comment ratio (0.0 to 1.0)
 * @return Maintainability index (0-100)
 */
double calculate_maintainability_index(int complexity, int loc, double comment_ratio);

#endif // METRIC_CALCULATOR_H
