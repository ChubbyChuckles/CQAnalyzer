#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <clang-c/Index.h>

#include "analyzer/metric_calculator.h"
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
