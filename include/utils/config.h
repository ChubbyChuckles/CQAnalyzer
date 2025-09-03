#ifndef CONFIG_H
#define CONFIG_H

#include "cqanalyzer.h"
#include "utils/logger.h"  // For LogLevel type

/**
 * @file config.h
 * @brief Configuration management for CQAnalyzer
 *
 * Provides functionality to load, store, and manage application configuration
 * from various sources (command line, config files, environment variables).
 */

// Configuration structure
typedef struct {
    char log_file[MAX_PATH_LENGTH];
    LogLevel log_level;
    int log_outputs;
    char default_project_path[MAX_PATH_LENGTH];
    bool enable_visualization;
    bool enable_metrics[32];
    int max_file_size_mb;
    int thread_count;
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
CQError config_load_from_file(const char* filepath);

/**
 * @brief Save current configuration to file
 *
 * @param filepath Path to save configuration file
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError config_save_to_file(const char* filepath);

/**
 * @brief Get the current configuration
 *
 * @return Pointer to current configuration structure
 */
const Config* config_get(void);

/**
 * @brief Set a configuration value
 *
 * @param key Configuration key
 * @param value Configuration value
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError config_set(const char* key, const char* value);

/**
 * @brief Get a configuration value as string
 *
 * @param key Configuration key
 * @return Configuration value string, or NULL if not found
 */
const char* config_get_string(const char* key);

/**
 * @brief Get a configuration value as integer
 *
 * @param key Configuration key
 * @param default_value Default value if key not found
 * @return Configuration value as integer
 */
int config_get_int(const char* key, int default_value);

/**
 * @brief Get a configuration value as boolean
 *
 * @param key Configuration key
 * @param default_value Default value if key not found
 * @return Configuration value as boolean
 */
bool config_get_bool(const char* key, bool default_value);

#endif // CONFIG_H
