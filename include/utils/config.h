#ifndef CONFIG_H
#define CONFIG_H

#include "cqanalyzer.h"
#include "utils/logger.h" // For LogLevel type

/**
 * @file config.h
 * @brief Configuration management for CQAnalyzer
 *
 * Provides functionality to load, store, and manage application configuration
 * from various sources (command line, config files, environment variables).
 */

// Metric configuration structure
typedef struct
{
    double weight;      // Weight for this metric in overall score
    double threshold;   // Threshold value for warnings/errors
    bool enabled;       // Whether this metric is enabled
} MetricConfig;

// Configuration structure
typedef struct
{
    char log_file[MAX_PATH_LENGTH];
    LogLevel log_level;
    int log_outputs;
    char default_project_path[MAX_PATH_LENGTH];
    bool enable_visualization;
    bool enable_metrics[32];  // Legacy support
    int max_file_size_mb;
    int thread_count;

    // Metric-specific configurations
    MetricConfig cyclomatic_complexity;
    MetricConfig lines_of_code;
    MetricConfig halstead_volume;
    MetricConfig halstead_difficulty;
    MetricConfig halstead_effort;
    MetricConfig halstead_time;
    MetricConfig halstead_bugs;
    MetricConfig maintainability_index;
    MetricConfig comment_density;
    MetricConfig class_cohesion;
    MetricConfig class_coupling;

    // Overall quality thresholds
    double overall_quality_threshold;
    double warning_threshold;
    double error_threshold;
} Config;

/**
 * @brief Initialize the configuration system
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError config_init(void);

/**
 * @brief Shutdown the configuration system
 */
void config_shutdown(void);

/**
 * @brief Load configuration from file
 *
 * @param filepath Path to configuration file
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError config_load_from_file(const char *filepath);

/**
 * @brief Save current configuration to file
 *
 * @param filepath Path to save configuration file
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError config_save_to_file(const char *filepath);

/**
 * @brief Get the current configuration
 *
 * @return Pointer to current configuration structure
 */
const Config *config_get(void);

/**
 * @brief Set a configuration value
 *
 * @param key Configuration key
 * @param value Configuration value
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError config_set(const char *key, const char *value);

/**
 * @brief Get a configuration value as string
 *
 * @param key Configuration key
 * @return Configuration value string, or NULL if not found
 */
const char *config_get_string(const char *key);

/**
 * @brief Get a configuration value as integer
 *
 * @param key Configuration key
 * @param default_value Default value if key not found
 * @return Configuration value as integer
 */
int config_get_int(const char *key, int default_value);

/**
 * @brief Get a configuration value as boolean
 *
 * @param key Configuration key
 * @param default_value Default value if key not found
 * @return Configuration value as boolean
 */
bool config_get_bool(const char *key, bool default_value);

/**
 * @brief Get metric configuration by name
 *
 * @param metric_name Name of the metric (e.g., "cyclomatic_complexity")
 * @return Pointer to metric configuration, or NULL if not found
 */
const MetricConfig *config_get_metric_config(const char *metric_name);

/**
 * @brief Get overall quality threshold
 *
 * @return Overall quality threshold value
 */
double config_get_overall_quality_threshold(void);

/**
 * @brief Get warning threshold
 *
 * @return Warning threshold value
 */
double config_get_warning_threshold(void);

/**
 * @brief Get error threshold
 *
 * @return Error threshold value
 */
double config_get_error_threshold(void);

#endif // CONFIG_H
