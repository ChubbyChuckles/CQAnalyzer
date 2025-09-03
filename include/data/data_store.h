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
CQError data_store_add_file(const char *filepath, SupportedLanguage language);

/**
 * @brief Add metric data for a file
 *
 * @param filepath File path
 * @param metric_name Name of the metric
 * @param value Metric value
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError data_store_add_metric(const char *filepath, const char *metric_name, double value);

/**
 * @brief Get metric value for a file
 *
 * @param filepath File path
 * @param metric_name Name of the metric
 * @return Metric value, or -1.0 if not found
 */
double data_store_get_metric(const char *filepath, const char *metric_name);

/**
 * @brief Get all file paths in the data store
 *
 * @param filepaths Array to store file paths (caller must allocate)
 * @param max_files Maximum number of files to return
 * @return Number of files found
 */
int data_store_get_all_files(char filepaths[][MAX_PATH_LENGTH], int max_files);

/**
 * @brief Get all metric values for a specific metric across all files
 *
 * @param metric_name Name of the metric
 * @param values Array to store metric values (caller must allocate)
 * @param max_values Maximum number of values to return
 * @return Number of values found
 */
int data_store_get_all_metric_values(const char *metric_name, double *values, int max_values);

/**
 * @brief Serialize data store to binary format
 *
 * @param filepath Output file path
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError data_store_serialize_binary(const char *filepath);

/**
 * @brief Deserialize data store from binary format
 *
 * @param filepath Input file path
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError data_store_deserialize_binary(const char *filepath);

/**
 * @brief Serialize data store to JSON format
 *
 * @param filepath Output file path
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError data_store_serialize_json(const char *filepath);

/**
 * @brief Export metrics to CSV format
 *
 * @param filepath Output file path
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError data_store_export_csv(const char *filepath);

#endif // DATA_STORE_H
