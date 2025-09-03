#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "cqanalyzer.h"

/**
 * @file data_store.h
 * @brief Data storage and management
 *
 * Provides data structures and functions to store and manage
 * parsed code information and computed metrics.
 */

/**
 * @brief Initialize data store
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError data_store_init(void);

/**
 * @brief Shutdown data store
 */
void data_store_shutdown(void);

/**
 * @brief Add file information to store
 *
 * @param filepath File path
 * @param language Programming language
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError data_store_add_file(const char* filepath, SupportedLanguage language);

/**
 * @brief Add metric data for a file
 *
 * @param filepath File path
 * @param metric_name Name of the metric
 * @param value Metric value
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError data_store_add_metric(const char* filepath, const char* metric_name, double value);

/**
 * @brief Get metric value for a file
 *
 * @param filepath File path
 * @param metric_name Name of the metric
 * @return Metric value, or -1.0 if not found
 */
double data_store_get_metric(const char* filepath, const char* metric_name);

#endif // DATA_STORE_H
