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
CQError aggregate_project_metrics(const char* project_name);

/**
 * @brief Calculate statistical measures for a metric
 *
 * @param metric_name Name of the metric
 * @param mean Output mean value
 * @param median Output median value
 * @param stddev Output standard deviation
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError calculate_metric_statistics(const char* metric_name, double* mean,
                                   double* median, double* stddev);

/**
 * @brief Get project summary
 *
 * @param total_files Output total number of files
 * @param total_loc Output total lines of code
 * @param avg_complexity Output average complexity
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError get_project_summary(int* total_files, int* total_loc, double* avg_complexity);

#endif // METRIC_AGGREGATOR_H
