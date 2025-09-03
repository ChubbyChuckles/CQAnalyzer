#ifndef METRIC_AGGREGATOR_H
#define METRIC_AGGREGATOR_H

#include "cqanalyzer.h"

/**
 * @file metric_aggregator.h
 * @brief Metric aggregation and statistics
 *
 * Provides functions to aggregate and analyze metric data
 * across multiple files and projects.
 */

/**
 * @brief Aggregate metrics for a project
 *
 * @param project_name Name of the project
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError aggregate_project_metrics(const char *project_name);

/**
 * @brief Calculate statistical measures for a metric
 *
 * @param metric_name Name of the metric
 * @param mean Output mean value
 * @param median Output median value
 * @param stddev Output standard deviation
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError calculate_metric_statistics(const char *metric_name, double *mean,
                                    double *median, double *stddev);

/**
 * @brief Get project summary
 *
 * @param total_files Output total number of files
 * @param total_loc Output total lines of code
 * @param avg_complexity Output average complexity
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError get_project_summary(int *total_files, int *total_loc, double *avg_complexity);

/**
 * @brief Calculate min and max values for a metric
 *
 * @param metric_name Name of the metric
 * @param min Output minimum value
 * @param max Output maximum value
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError calculate_metric_min_max(const char *metric_name, double *min, double *max);

/**
 * @brief Calculate percentile for a metric
 *
 * @param metric_name Name of the metric
 * @param percentile Percentile to calculate (0-100)
 * @param value Output percentile value
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError calculate_metric_percentile(const char *metric_name, double percentile, double *value);

/**
 * @brief Process metrics in batches for memory efficiency
 *
 * @param metric_name Name of the metric
 * @param batch_size Size of each processing batch
 * @param processor Callback function to process each batch
 * @param user_data User data passed to processor
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError process_metric_batches(const char *metric_name, int batch_size,
                               CQError (*processor)(double *batch, int batch_count, void *user_data),
                               void *user_data);

#endif // METRIC_AGGREGATOR_H
