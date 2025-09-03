#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "data/metric_aggregator.h"
#include "data/data_store.h"
#include "utils/logger.h"
#include "utils/memory.h"

CQError aggregate_project_metrics(const char *project_name)
{
    if (!project_name)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Get all files in the project
    char filepaths[1024][MAX_PATH_LENGTH];
    int file_count = data_store_get_all_files(filepaths, 1024);

    if (file_count == 0)
    {
        LOG_WARNING("No files found for project: %s", project_name);
        return CQ_SUCCESS;
    }

    // Aggregate key metrics across all files
    double total_complexity = 0.0;
    double total_loc = 0.0;
    int complexity_count = 0;
    int loc_count = 0;

    for (int i = 0; i < file_count; i++)
    {
        // Aggregate complexity
        double complexity = data_store_get_metric(filepaths[i], "complexity");
        if (complexity >= 0)
        {
            total_complexity += complexity;
            complexity_count++;
        }

        // Aggregate lines of code
        double loc = data_store_get_metric(filepaths[i], "loc");
        if (loc >= 0)
        {
            total_loc += loc;
            loc_count++;
        }
    }

    // Store aggregated metrics with project prefix
    char metric_name[128];

    if (complexity_count > 0)
    {
        double avg_complexity = total_complexity / complexity_count;
        sprintf(metric_name, "project_%s_avg_complexity", project_name);
        // Note: In a real implementation, we might want to store these in a separate project metrics store
        LOG_INFO("Project %s average complexity: %.2f (from %d files)", project_name, avg_complexity, complexity_count);
    }

    if (loc_count > 0)
    {
        sprintf(metric_name, "project_%s_total_loc", project_name);
        LOG_INFO("Project %s total LOC: %.0f (from %d files)", project_name, total_loc, loc_count);
    }

    LOG_INFO("Project metric aggregation completed for: %s (%d files processed)", project_name, file_count);
    return CQ_SUCCESS;
}

CQError calculate_metric_statistics(const char *metric_name, double *mean,
                                      double *median, double *stddev)
{
    if (!metric_name || !mean || !median || !stddev)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // First, try to get values with a reasonable initial buffer
    const int INITIAL_BUFFER_SIZE = 1024;
    double *values = cq_malloc(sizeof(double) * INITIAL_BUFFER_SIZE);
    if (!values)
    {
        LOG_ERROR("Failed to allocate memory for metric values");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    int count = data_store_get_all_metric_values(metric_name, values, INITIAL_BUFFER_SIZE);

    // If we got all values, proceed
    if (count < INITIAL_BUFFER_SIZE)
    {
        // We have all values
    }
    else
    {
        // We might have more values, try with a larger buffer
        const int LARGE_BUFFER_SIZE = 10000;
        double *large_values = cq_malloc(sizeof(double) * LARGE_BUFFER_SIZE);
        if (!large_values)
        {
            LOG_ERROR("Failed to allocate larger buffer for metric values");
            cq_free(values);
            return CQ_ERROR_MEMORY_ALLOCATION;
        }

        int large_count = data_store_get_all_metric_values(metric_name, large_values, LARGE_BUFFER_SIZE);
        cq_free(values);
        values = large_values;
        count = large_count;

        if (count >= LARGE_BUFFER_SIZE)
        {
            LOG_WARNING("Large dataset detected (%d values), performance may be impacted", count);
        }
    }

    if (count == 0)
    {
        LOG_WARNING("No values found for metric: %s", metric_name);
        cq_free(values);
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

    // Sort values for median (using bubble sort for simplicity)
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

    cq_free(values);
    return CQ_SUCCESS;
}

CQError process_metric_batches(const char *metric_name, int batch_size,
                               CQError (*processor)(double *batch, int batch_count, void *user_data),
                               void *user_data)
{
    if (!metric_name || !processor || batch_size <= 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // For now, we'll get all values and process in batches
    // In a more advanced implementation, this would stream from data_store
    const int MAX_VALUES = 50000; // Reasonable limit for demonstration
    double *all_values = cq_malloc(sizeof(double) * MAX_VALUES);
    if (!all_values)
    {
        LOG_ERROR("Failed to allocate memory for batch processing");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    int total_count = data_store_get_all_metric_values(metric_name, all_values, MAX_VALUES);

    if (total_count == 0)
    {
        LOG_WARNING("No values found for metric: %s", metric_name);
        cq_free(all_values);
        return CQ_SUCCESS;
    }

    // Process in batches
    int processed = 0;
    while (processed < total_count)
    {
        int current_batch_size = (total_count - processed < batch_size) ?
                                (total_count - processed) : batch_size;

        CQError result = processor(&all_values[processed], current_batch_size, user_data);
        if (result != CQ_SUCCESS)
        {
            LOG_ERROR("Batch processing failed at batch starting at index %d", processed);
            cq_free(all_values);
            return result;
        }

        processed += current_batch_size;
        LOG_DEBUG("Processed batch of %d values (%d/%d total)", current_batch_size, processed, total_count);
    }

    LOG_INFO("Batch processing completed for %s: %d values in %d batches",
              metric_name, total_count, (total_count + batch_size - 1) / batch_size);

    cq_free(all_values);
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

CQError calculate_metric_min_max(const char *metric_name, double *min, double *max)
{
    if (!metric_name || !min || !max)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Get all metric values
    const int BUFFER_SIZE = 10000;
    double *values = cq_malloc(sizeof(double) * BUFFER_SIZE);
    if (!values)
    {
        LOG_ERROR("Failed to allocate memory for metric values");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    int count = data_store_get_all_metric_values(metric_name, values, BUFFER_SIZE);

    if (count == 0)
    {
        LOG_WARNING("No values found for metric: %s", metric_name);
        cq_free(values);
        *min = 0.0;
        *max = 0.0;
        return CQ_SUCCESS;
    }

    // Find min and max
    *min = values[0];
    *max = values[0];

    for (int i = 1; i < count; i++)
    {
        if (values[i] < *min)
        {
            *min = values[i];
        }
        if (values[i] > *max)
        {
            *max = values[i];
        }
    }

    LOG_INFO("Calculated min/max for %s: min=%.2f, max=%.2f (n=%d)",
              metric_name, *min, *max, count);

    cq_free(values);
    return CQ_SUCCESS;
}

CQError calculate_metric_percentile(const char *metric_name, double percentile, double *value)
{
    if (!metric_name || !value || percentile < 0.0 || percentile > 100.0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Get all metric values
    const int BUFFER_SIZE = 10000;
    double *values = cq_malloc(sizeof(double) * BUFFER_SIZE);
    if (!values)
    {
        LOG_ERROR("Failed to allocate memory for metric values");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    int count = data_store_get_all_metric_values(metric_name, values, BUFFER_SIZE);

    if (count == 0)
    {
        LOG_WARNING("No values found for metric: %s", metric_name);
        cq_free(values);
        *value = 0.0;
        return CQ_SUCCESS;
    }

    // Sort values
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

    // Calculate percentile index
    double index = (percentile / 100.0) * (count - 1);
    int lower_index = (int)index;
    int upper_index = lower_index + 1;

    if (upper_index >= count)
    {
        *value = values[lower_index];
    }
    else
    {
        // Linear interpolation
        double fraction = index - lower_index;
        *value = values[lower_index] + fraction * (values[upper_index] - values[lower_index]);
    }

    LOG_INFO("Calculated %.1fth percentile for %s: %.2f (n=%d)",
              percentile, metric_name, *value, count);

    cq_free(values);
    return CQ_SUCCESS;
}
