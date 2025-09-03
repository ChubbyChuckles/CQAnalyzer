#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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
