#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <clang-c/Index.h>

#include "analyzer/metric_calculator.h"
#include "data/ast_types.h"
#include "utils/logger.h"

int calculate_cyclomatic_complexity(void *ast_data)
{
    if (!ast_data)
    {
        LOG_ERROR("Invalid AST data for complexity calculation");
        return -1;
    }

    // TODO: Implement cyclomatic complexity calculation
    // TODO: Count decision points (if, while, for, case, etc.)
    // TODO: Add 1 for each function/method

    LOG_WARNING("Cyclomatic complexity calculation not yet implemented");
    return 1; // Default complexity
}

CQError calculate_lines_of_code(const char *filepath, int *physical_loc,
                                int *logical_loc, int *comment_loc)
{
    if (!filepath || !physical_loc || !logical_loc || !comment_loc)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        LOG_ERROR("Could not open file for LOC calculation: %s", filepath);
        return CQ_ERROR_FILE_NOT_FOUND;
    }

    char line[1024];
    int phys_lines = 0;
    int log_lines = 0;
    int comment_lines = 0;
    bool in_multiline_comment = false;

    while (fgets(line, sizeof(line), file))
    {
        phys_lines++;

        // Remove trailing whitespace
        char *end = line + strlen(line) - 1;
        while (end > line && isspace(*end))
            *end-- = '\0';

        // Skip empty lines
        if (line[0] == '\0')
        {
            continue;
        }

        // Check for comments
        char *comment_start = strstr(line, "/*");
        char *comment_end = strstr(line, "*/");
        char *line_comment = strstr(line, "//");

        if (in_multiline_comment)
        {
            comment_lines++;
            if (comment_end)
            {
                in_multiline_comment = false;
            }
            continue;
        }

        if (comment_start && (!line_comment || comment_start < line_comment))
        {
            comment_lines++;
            if (!comment_end)
            {
                in_multiline_comment = true;
            }
            continue;
        }

        if (line_comment)
        {
            comment_lines++;
            continue;
        }

        // Count as logical line if it contains actual code
        log_lines++;
    }

    fclose(file);

    *physical_loc = phys_lines;
    *logical_loc = log_lines;
    *comment_loc = comment_lines;

    LOG_INFO("LOC calculation for %s: physical=%d, logical=%d, comments=%d",
             filepath, phys_lines, log_lines, comment_lines);

    return CQ_SUCCESS;
}

CQError calculate_halstead_metrics(const char *filepath, HalsteadMetrics *metrics)
{
    if (!filepath || !metrics)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Initialize metrics to zero
    memset(metrics, 0, sizeof(HalsteadMetrics));

    // For simplicity, use a basic token counting approach
    // In a full implementation, this would use clang_tokenize

    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        LOG_ERROR("Could not open file for Halstead calculation: %s", filepath);
        return CQ_ERROR_FILE_NOT_FOUND;
    }

    // Simple token counting (operators and operands)
    // This is a simplified implementation
    char line[1024];
    while (fgets(line, sizeof(line), file))
    {
        char *token = strtok(line, " \t\n\r;(){}[]");
        while (token)
        {
            // Count operators
            if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
                strcmp(token, "*") == 0 || strcmp(token, "/") == 0 ||
                strcmp(token, "%") == 0 || strcmp(token, "=") == 0 ||
                strcmp(token, "==") == 0 || strcmp(token, "!=") == 0 ||
                strcmp(token, "<") == 0 || strcmp(token, ">") == 0 ||
                strcmp(token, "<=") == 0 || strcmp(token, ">=") == 0 ||
                strcmp(token, "&&") == 0 || strcmp(token, "||") == 0 ||
                strcmp(token, "!") == 0 || strcmp(token, "if") == 0 ||
                strcmp(token, "while") == 0 || strcmp(token, "for") == 0 ||
                strcmp(token, "return") == 0)
            {
                metrics->N1++;
                // For simplicity, assume all operators are distinct
                metrics->n1++;
            }
            // Count operands (identifiers and literals)
            else if (isalpha(token[0]) || token[0] == '_' ||
                     (isdigit(token[0]) && strlen(token) > 1))
            {
                metrics->N2++;
                // For simplicity, assume all operands are distinct
                metrics->n2++;
            }

            token = strtok(NULL, " \t\n\r;(){}[]");
        }
    }

    fclose(file);

    // Calculate derived metrics
    int N = metrics->N1 + metrics->N2;
    int n = metrics->n1 + metrics->n2;

    if (n > 0)
    {
        metrics->volume = N * log2((double)n);
        if (metrics->n2 > 0)
        {
            metrics->difficulty = ((double)metrics->n1 / 2.0) * ((double)metrics->N2 / (double)metrics->n2);
        }
        metrics->effort = metrics->difficulty * metrics->volume;
        metrics->time = metrics->effort / 18.0; // 18 seconds per unit effort
        metrics->bugs = pow(metrics->effort, 2.0/3.0) / 3000.0;
    }

    LOG_INFO("Halstead metrics for %s: n1=%d, n2=%d, N1=%d, N2=%d, volume=%.2f",
             filepath, metrics->n1, metrics->n2, metrics->N1, metrics->N2, metrics->volume);

    return CQ_SUCCESS;
}

double calculate_maintainability_index(int complexity, int loc, double comment_ratio)
{
    if (loc <= 0)
    {
        return 0.0;
    }

    // Maintainability Index formula (simplified)
    // MI = 171 - 5.2 * ln(V) - 0.23 * G - 16.2 * ln(LOC)
    // where V is volume, G is cyclomatic complexity, LOC is lines of code

    double volume = (double)loc; // Simplified
    double mi = 171.0 - 5.2 * log(volume) - 0.23 * (double)complexity - 16.2 * log((double)loc);

    // Add comment ratio bonus
    mi += comment_ratio * 20.0;

    // Clamp to 0-100 range
    if (mi < 0.0)
        mi = 0.0;
    if (mi > 100.0)
        mi = 100.0;

    return mi;
}

/**
 * @brief Calculate comment density ratio
 *
 * @param comment_loc Number of comment lines
 * @param physical_loc Total physical lines of code
 * @return Comment density as percentage (0.0 to 100.0)
 */
double calculate_comment_density(int comment_loc, int physical_loc)
{
    if (physical_loc <= 0)
    {
        return 0.0;
    }

    return (double)comment_loc / (double)physical_loc * 100.0;
}

double calculate_class_cohesion(const struct ClassInfo *class_info)
{
    if (!class_info)
    {
        LOG_ERROR("Invalid class info for cohesion calculation");
        return 0.0;
    }

    // Basic cohesion calculation based on method-to-field ratio
    // Higher method count relative to fields suggests better cohesion
    // This is a simplified metric - in practice, we'd analyze method interactions

    uint32_t method_count = class_info->method_count;
    uint32_t field_count = class_info->field_count;

    if (field_count == 0)
    {
        // No fields - cohesion depends on method count
        return method_count > 0 ? 0.5 : 0.0;
    }

    // Calculate cohesion as method-to-field ratio, capped at 1.0
    double cohesion = (double)method_count / (double)field_count;

    if (cohesion > 1.0)
        cohesion = 1.0;

    LOG_INFO("Class cohesion for %s: methods=%u, fields=%u, cohesion=%.2f",
             class_info->name, method_count, field_count, cohesion);

    return cohesion;
}

double calculate_class_coupling(const struct ClassInfo *class_info,
                               const struct ClassInfo *all_classes)
{
    if (!class_info || !all_classes)
    {
        LOG_ERROR("Invalid parameters for coupling calculation");
        return 0.0;
    }

    // Basic coupling calculation based on class relationships
    // This is a simplified implementation - in practice, we'd analyze
    // method calls, field accesses, and inheritance relationships

    uint32_t total_classes = 0;

    // Count total classes in the project
    const struct ClassInfo *current = all_classes;
    while (current)
    {
        total_classes++;
        current = current->next;
    }

    if (total_classes <= 1)
    {
        return 0.0; // No coupling possible with 0 or 1 classes
    }

    // Simplified coupling calculation
    // In a real implementation, this would analyze actual dependencies
    // For now, we use a basic heuristic based on class size and project size

    uint32_t class_size = class_info->method_count + class_info->field_count;
    double coupling_ratio = (double)class_size / (double)total_classes;

    // Normalize to 0.0-1.0 range
    double coupling = coupling_ratio > 1.0 ? 1.0 : coupling_ratio;

    LOG_INFO("Class coupling for %s: size=%u, total_classes=%u, coupling=%.2f",
             class_info->name, class_size, total_classes, coupling);

    return coupling;
}

/**
 * @brief Normalize a metric value using specified method
 */
double normalize_metric(double value, double min_val, double max_val,
                       double mean, double std_dev, NormalizationMethod method)
{
    if (method == NORMALIZATION_MIN_MAX)
    {
        // Min-max normalization: (x - min) / (max - min)
        if (max_val == min_val)
        {
            return 0.5; // Return middle value if all values are the same
        }
        double normalized = (value - min_val) / (max_val - min_val);

        // Clamp to [0, 1] range
        if (normalized < 0.0) normalized = 0.0;
        if (normalized > 1.0) normalized = 1.0;

        return normalized;
    }
    else if (method == NORMALIZATION_Z_SCORE)
    {
        // Z-score normalization: (x - mean) / std_dev
        if (std_dev == 0.0)
        {
            return 0.0; // Return 0 if no variation
        }
        return (value - mean) / std_dev;
    }
    else if (method == NORMALIZATION_ROBUST)
    {
        // For robust normalization, we need median and IQR
        // This is a simplified version - in practice, you'd calculate these
        LOG_WARNING("Robust normalization not fully implemented - using min-max fallback");
        return normalize_metric(value, min_val, max_val, mean, std_dev, NORMALIZATION_MIN_MAX);
    }

    LOG_ERROR("Unknown normalization method: %d", method);
    return 0.0;
}

/**
 * @brief Scale a normalized metric for visualization
 */
double scale_metric(double normalized_value, double target_min, double target_max)
{
    // Scale from [0, 1] to [target_min, target_max]
    return target_min + (normalized_value * (target_max - target_min));
}

/**
 * @brief Normalize an array of metric values
 */
CQError normalize_metric_array(const double *values, size_t count,
                              NormalizationMethod method, double *output)
{
    if (!values || !output || count == 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Calculate statistics needed for normalization
    double min_val = values[0];
    double max_val = values[0];
    double sum = values[0];
    double sum_sq = values[0] * values[0];

    for (size_t i = 1; i < count; i++)
    {
        if (values[i] < min_val) min_val = values[i];
        if (values[i] > max_val) max_val = values[i];
        sum += values[i];
        sum_sq += values[i] * values[i];
    }

    double mean = sum / count;
    double variance = (sum_sq / count) - (mean * mean);
    double std_dev = sqrt(variance);

    // Normalize each value
    for (size_t i = 0; i < count; i++)
    {
        output[i] = normalize_metric(values[i], min_val, max_val, mean, std_dev, method);
    }

    LOG_INFO("Normalized %zu metric values using method %d", count, method);
    return CQ_SUCCESS;
}
