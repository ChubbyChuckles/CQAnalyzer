#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "data/metric_aggregator.h"
#include "data/data_store.h"
#include "utils/logger.h"

CQError aggregate_project_metrics(const char *project_name)
{
    if (!project_name)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Project metrics are aggregated on-demand when requested
    // This function serves as a placeholder for any pre-aggregation setup

    LOG_INFO("Project metric aggregation initialized for: %s", project_name);
    return CQ_SUCCESS;
}

CQError calculate_metric_statistics(const char *metric_name, double *mean,
                                     double *median, double *stddev)
{
    if (!metric_name || !mean || !median || !stddev)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Get all metric values
    double values[1024]; // Reasonable maximum
    int count = data_store_get_all_metric_values(metric_name, values, 1024);

    if (count == 0)
    {
        LOG_WARNING("No values found for metric: %s", metric_name);
        *mean = 0.0;
        *median = 0.0;
        *stddev = 0.0;
        return CQ_SUCCESS;
    }

    // Calculate mean
    double sum = 0.0;
    for (int i = 0; i < count; i++)
    {
        sum += values[i];
    }
    *mean = sum / count;

    // Sort values for median
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            if (values[i] > values[j])
            {
                double temp = values[i];
                values[i] = values[j];
                values[j] = temp;
            }
        }
    }

    // Calculate median
    if (count % 2 == 0)
    {
        *median = (values[count / 2 - 1] + values[count / 2]) / 2.0;
    }
    else
    {
        *median = values[count / 2];
    }

    // Calculate standard deviation
    double variance = 0.0;
    for (int i = 0; i < count; i++)
    {
        double diff = values[i] - *mean;
        variance += diff * diff;
    }
    variance /= count;
    *stddev = sqrt(variance);

    LOG_INFO("Calculated statistics for %s: mean=%.2f, median=%.2f, stddev=%.2f (n=%d)",
             metric_name, *mean, *median, *stddev, count);

    return CQ_SUCCESS;
}

CQError get_project_summary(int *total_files, int *total_loc, double *avg_complexity)
{
    if (!total_files || !total_loc || !avg_complexity)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Get all files
    char filepaths[1024][MAX_PATH_LENGTH];
    int file_count = data_store_get_all_files(filepaths, 1024);
    *total_files = file_count;

    if (file_count == 0)
    {
        LOG_WARNING("No files found in project");
        *total_loc = 0;
        *avg_complexity = 0.0;
        return CQ_SUCCESS;
    }

    // Calculate totals
    int total_lines = 0;
    double total_complexity = 0.0;
    int complexity_count = 0;

    for (int i = 0; i < file_count; i++)
    {
        // Get LOC for this file
        double loc = data_store_get_metric(filepaths[i], "loc");
        if (loc >= 0)
        {
            total_lines += (int)loc;
        }

        // Get complexity for this file
        double complexity = data_store_get_metric(filepaths[i], "complexity");
        if (complexity >= 0)
        {
            total_complexity += complexity;
            complexity_count++;
        }
    }

    *total_loc = total_lines;

    if (complexity_count > 0)
    {
        *avg_complexity = total_complexity / complexity_count;
    }
    else
    {
        *avg_complexity = 0.0;
    }

    LOG_INFO("Project summary: %d files, %d total LOC, avg complexity %.2f",
             *total_files, *total_loc, *avg_complexity);

    return CQ_SUCCESS;
}
