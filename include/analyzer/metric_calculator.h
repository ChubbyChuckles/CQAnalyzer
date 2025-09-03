#ifndef METRIC_CALCULATOR_H
#define METRIC_CALCULATOR_H

#include "cqanalyzer.h"
#include "data/ast_types.h"

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
 * @brief Halstead complexity metrics
 */
typedef struct {
    int n1;  // Number of distinct operators
    int n2;  // Number of distinct operands
    int N1;  // Total number of operators
    int N2;  // Total number of operands
    double volume;      // Program volume
    double difficulty;  // Program difficulty
    double effort;      // Program effort
    double time;        // Time to program (seconds)
    double bugs;        // Estimated number of bugs
} HalsteadMetrics;

/**
 * @brief Calculate Halstead complexity metrics
 *
 * @param filepath Source file path
 * @param metrics Output Halstead metrics
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError calculate_halstead_metrics(const char *filepath, HalsteadMetrics *metrics);

/**
 * @brief Calculate maintainability index
 *
 * @param complexity Cyclomatic complexity
 * @param loc Lines of code
 * @param comment_ratio Comment ratio (0.0 to 1.0)
 * @return Maintainability index (0-100)
 */
double calculate_maintainability_index(int complexity, int loc, double comment_ratio);

/**
 * @brief Calculate comment density ratio
 *
 * @param comment_loc Number of comment lines
 * @param physical_loc Total physical lines of code
 * @return Comment density as percentage (0.0 to 100.0)
 */
double calculate_comment_density(int comment_loc, int physical_loc);

/**
 * @brief Calculate class cohesion metric
 *
 * Measures how well the methods of a class work together.
 * Higher values indicate better cohesion.
 *
 * @param class_info Pointer to class information
 * @return Cohesion value (0.0 to 1.0)
 */
double calculate_class_cohesion(const ClassInfo *class_info, const Project *project);

/**
 * @brief Calculate class coupling metric
 *
 * Measures the degree of interdependence between classes.
 * Lower values indicate better decoupling.
 *
 * @param class_info Pointer to class information
 * @param all_classes Pointer to list of all classes in the project
 * @return Coupling value (0.0 to 1.0)
 */
double calculate_class_coupling(const ClassInfo *class_info,
                               const Project *project);

/**
 * @brief Normalization method enumeration
 */
typedef enum {
    NORMALIZATION_MIN_MAX,    // Min-max normalization: (x - min) / (max - min)
    NORMALIZATION_Z_SCORE,    // Z-score normalization: (x - mean) / std_dev
    NORMALIZATION_ROBUST      // Robust normalization: (x - median) / IQR
} NormalizationMethod;

/**
 * @brief Normalize a metric value using specified method
 *
 * @param value The metric value to normalize
 * @param min_val Minimum value in the dataset
 * @param max_val Maximum value in the dataset
 * @param mean Mean value of the dataset (for z-score)
 * @param std_dev Standard deviation of the dataset (for z-score)
 * @param method Normalization method to use
 * @return Normalized value (typically 0.0 to 1.0 for min-max, or standardized for z-score)
 */
double normalize_metric(double value, double min_val, double max_val,
                       double mean, double std_dev, NormalizationMethod method);

/**
 * @brief Scale a normalized metric for visualization
 *
 * @param normalized_value Normalized metric value (0.0 to 1.0)
 * @param target_min Target minimum value for scaling
 * @param target_max Target maximum value for scaling
 * @return Scaled value in the target range
 */
double scale_metric(double normalized_value, double target_min, double target_max);

/**
 * @brief Normalize an array of metric values
 *
 * @param values Array of metric values to normalize
 * @param count Number of values in the array
 * @param method Normalization method to use
 * @param output Array to store normalized values (must be pre-allocated)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError normalize_metric_array(const double *values, size_t count,
                              NormalizationMethod method, double *output);

#endif // METRIC_CALCULATOR_H
