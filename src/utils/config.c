#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "utils/config.h"
#include "utils/logger.h"

static Config current_config;
static bool config_initialized = false;

static CQError config_set_defaults(void)
{
    // Set default values
    memset(&current_config, 0, sizeof(Config));

    strcpy(current_config.log_file, "cqanalyzer.log");
    current_config.log_level = LOG_LEVEL_INFO;
    current_config.log_outputs = LOG_OUTPUT_CONSOLE;
    current_config.enable_visualization = true;
    current_config.max_file_size_mb = 100;
    current_config.thread_count = 4;

    // Enable common metrics by default (legacy support)
    current_config.enable_metrics[0] = true; // Cyclomatic complexity
    current_config.enable_metrics[1] = true; // Lines of code
    current_config.enable_metrics[2] = true; // Maintainability index

    // Set default metric configurations
    // Cyclomatic complexity
    current_config.cyclomatic_complexity.weight = 1.0;
    current_config.cyclomatic_complexity.threshold = 10.0;
    current_config.cyclomatic_complexity.enabled = true;

    // Lines of code
    current_config.lines_of_code.weight = 0.8;
    current_config.lines_of_code.threshold = 300.0;
    current_config.lines_of_code.enabled = true;

    // Halstead metrics
    current_config.halstead_volume.weight = 0.7;
    current_config.halstead_volume.threshold = 1000.0;
    current_config.halstead_volume.enabled = true;

    current_config.halstead_difficulty.weight = 0.6;
    current_config.halstead_difficulty.threshold = 50.0;
    current_config.halstead_difficulty.enabled = true;

    current_config.halstead_effort.weight = 0.8;
    current_config.halstead_effort.threshold = 50000.0;
    current_config.halstead_effort.enabled = true;

    current_config.halstead_time.weight = 0.5;
    current_config.halstead_time.threshold = 2800.0; // 46.7 minutes
    current_config.halstead_time.enabled = true;

    current_config.halstead_bugs.weight = 0.9;
    current_config.halstead_bugs.threshold = 0.1;
    current_config.halstead_bugs.enabled = true;

    // Maintainability index
    current_config.maintainability_index.weight = 1.2;
    current_config.maintainability_index.threshold = 65.0; // Below 65 is concerning
    current_config.maintainability_index.enabled = true;

    // Comment density
    current_config.comment_density.weight = 0.6;
    current_config.comment_density.threshold = 15.0; // 15% minimum
    current_config.comment_density.enabled = true;

    // Class cohesion
    current_config.class_cohesion.weight = 0.8;
    current_config.class_cohesion.threshold = 0.5;
    current_config.class_cohesion.enabled = true;

    // Class coupling
    current_config.class_coupling.weight = 0.7;
    current_config.class_coupling.threshold = 0.3;
    current_config.class_coupling.enabled = true;

    // Overall quality thresholds
    current_config.overall_quality_threshold = 70.0;
    current_config.warning_threshold = 60.0;
    current_config.error_threshold = 40.0;

    return CQ_SUCCESS;
}

CQError config_init(void)
{
    if (config_initialized)
    {
        return CQ_SUCCESS;
    }

    CQError result = config_set_defaults();
    if (result != CQ_SUCCESS)
    {
        return result;
    }

    config_initialized = true;
    LOG_INFO("Configuration system initialized");
    return CQ_SUCCESS;
}

void config_shutdown(void)
{
    if (!config_initialized)
    {
        return;
    }

    config_initialized = false;
    LOG_INFO("Configuration system shutdown");
}

CQError config_load_from_file(const char *filepath)
{
    if (!config_initialized)
    {
        return CQ_ERROR_UNKNOWN;
    }

    if (!filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        LOG_WARNING("Could not open config file: %s", filepath);
        return CQ_ERROR_FILE_NOT_FOUND;
    }

    char line[512];
    int line_number = 0;

    while (fgets(line, sizeof(line), file))
    {
        line_number++;

        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\0' || isspace(line[0]))
        {
            continue;
        }

        // Parse key=value pairs
        char *equals_pos = strchr(line, '=');
        if (!equals_pos)
        {
            LOG_WARNING("Invalid config line %d: %s", line_number, line);
            continue;
        }

        *equals_pos = '\0';
        char *key = line;
        char *value = equals_pos + 1;

        // Trim whitespace
        while (isspace(*key))
            key++;
        char *end = key + strlen(key) - 1;
        while (end > key && isspace(*end))
            *end-- = '\0';

        while (isspace(*value))
            value++;
        end = value + strlen(value) - 1;
        while (end > value && isspace(*end))
            *end-- = '\0';

        // Set configuration value
        if (config_set(key, value) != CQ_SUCCESS)
        {
            LOG_WARNING("Failed to set config key '%s' at line %d", key, line_number);
        }
    }

    fclose(file);
    LOG_INFO("Configuration loaded from: %s", filepath);
    return CQ_SUCCESS;
}

CQError config_save_to_file(const char *filepath)
{
    if (!config_initialized)
    {
        return CQ_ERROR_UNKNOWN;
    }

    if (!filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    FILE *file = fopen(filepath, "w");
    if (!file)
    {
        LOG_ERROR("Could not open config file for writing: %s", filepath);
        return CQ_ERROR_FILE_NOT_FOUND;
    }

    fprintf(file, "# CQAnalyzer Configuration File\n");
    fprintf(file, "# Generated automatically\n\n");

    fprintf(file, "# Logging configuration\n");
    fprintf(file, "log_file=%s\n", current_config.log_file);
    fprintf(file, "log_level=%d\n", current_config.log_level);
    fprintf(file, "log_outputs=%d\n", current_config.log_outputs);

    fprintf(file, "\n# Analysis configuration\n");
    fprintf(file, "enable_visualization=%s\n", current_config.enable_visualization ? "true" : "false");
    fprintf(file, "max_file_size_mb=%d\n", current_config.max_file_size_mb);
    fprintf(file, "thread_count=%d\n", current_config.thread_count);

    fprintf(file, "\n# Enabled metrics (bitfield)\n");
    fprintf(file, "enable_metrics=");
    for (int i = 0; i < 32; i++)
    {
        fprintf(file, "%d", current_config.enable_metrics[i] ? 1 : 0);
    }
    fprintf(file, "\n");

    fprintf(file, "\n# Metric configurations\n");

    // Cyclomatic complexity
    fprintf(file, "metric_cyclomatic_complexity_enabled=%s\n",
             current_config.cyclomatic_complexity.enabled ? "true" : "false");
    fprintf(file, "metric_cyclomatic_complexity_weight=%.2f\n",
             current_config.cyclomatic_complexity.weight);
    fprintf(file, "metric_cyclomatic_complexity_threshold=%.2f\n",
             current_config.cyclomatic_complexity.threshold);

    // Lines of code
    fprintf(file, "metric_lines_of_code_enabled=%s\n",
             current_config.lines_of_code.enabled ? "true" : "false");
    fprintf(file, "metric_lines_of_code_weight=%.2f\n",
             current_config.lines_of_code.weight);
    fprintf(file, "metric_lines_of_code_threshold=%.2f\n",
             current_config.lines_of_code.threshold);

    // Halstead metrics
    fprintf(file, "metric_halstead_volume_enabled=%s\n",
             current_config.halstead_volume.enabled ? "true" : "false");
    fprintf(file, "metric_halstead_volume_weight=%.2f\n",
             current_config.halstead_volume.weight);
    fprintf(file, "metric_halstead_volume_threshold=%.2f\n",
             current_config.halstead_volume.threshold);

    fprintf(file, "metric_halstead_difficulty_enabled=%s\n",
             current_config.halstead_difficulty.enabled ? "true" : "false");
    fprintf(file, "metric_halstead_difficulty_weight=%.2f\n",
             current_config.halstead_difficulty.weight);
    fprintf(file, "metric_halstead_difficulty_threshold=%.2f\n",
             current_config.halstead_difficulty.threshold);

    fprintf(file, "metric_halstead_effort_enabled=%s\n",
             current_config.halstead_effort.enabled ? "true" : "false");
    fprintf(file, "metric_halstead_effort_weight=%.2f\n",
             current_config.halstead_effort.weight);
    fprintf(file, "metric_halstead_effort_threshold=%.2f\n",
             current_config.halstead_effort.threshold);

    fprintf(file, "metric_halstead_time_enabled=%s\n",
             current_config.halstead_time.enabled ? "true" : "false");
    fprintf(file, "metric_halstead_time_weight=%.2f\n",
             current_config.halstead_time.weight);
    fprintf(file, "metric_halstead_time_threshold=%.2f\n",
             current_config.halstead_time.threshold);

    fprintf(file, "metric_halstead_bugs_enabled=%s\n",
             current_config.halstead_bugs.enabled ? "true" : "false");
    fprintf(file, "metric_halstead_bugs_weight=%.2f\n",
             current_config.halstead_bugs.weight);
    fprintf(file, "metric_halstead_bugs_threshold=%.2f\n",
             current_config.halstead_bugs.threshold);

    // Maintainability index
    fprintf(file, "metric_maintainability_index_enabled=%s\n",
             current_config.maintainability_index.enabled ? "true" : "false");
    fprintf(file, "metric_maintainability_index_weight=%.2f\n",
             current_config.maintainability_index.weight);
    fprintf(file, "metric_maintainability_index_threshold=%.2f\n",
             current_config.maintainability_index.threshold);

    // Comment density
    fprintf(file, "metric_comment_density_enabled=%s\n",
             current_config.comment_density.enabled ? "true" : "false");
    fprintf(file, "metric_comment_density_weight=%.2f\n",
             current_config.comment_density.weight);
    fprintf(file, "metric_comment_density_threshold=%.2f\n",
             current_config.comment_density.threshold);

    // Class cohesion
    fprintf(file, "metric_class_cohesion_enabled=%s\n",
             current_config.class_cohesion.enabled ? "true" : "false");
    fprintf(file, "metric_class_cohesion_weight=%.2f\n",
             current_config.class_cohesion.weight);
    fprintf(file, "metric_class_cohesion_threshold=%.2f\n",
             current_config.class_cohesion.threshold);

    // Class coupling
    fprintf(file, "metric_class_coupling_enabled=%s\n",
             current_config.class_coupling.enabled ? "true" : "false");
    fprintf(file, "metric_class_coupling_weight=%.2f\n",
             current_config.class_coupling.weight);
    fprintf(file, "metric_class_coupling_threshold=%.2f\n",
             current_config.class_coupling.threshold);

    // Overall thresholds
    fprintf(file, "overall_quality_threshold=%.2f\n",
             current_config.overall_quality_threshold);
    fprintf(file, "warning_threshold=%.2f\n",
             current_config.warning_threshold);
    fprintf(file, "error_threshold=%.2f\n",
             current_config.error_threshold);

    fclose(file);
    LOG_INFO("Configuration saved to: %s", filepath);
    return CQ_SUCCESS;
}

const Config *config_get(void)
{
    return config_initialized ? &current_config : NULL;
}

CQError config_set(const char *key, const char *value)
{
    if (!config_initialized || !key || !value)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (strcmp(key, "log_file") == 0)
    {
        strncpy(current_config.log_file, value, sizeof(current_config.log_file) - 1);
    }
    else if (strcmp(key, "log_level") == 0)
    {
        current_config.log_level = atoi(value);
    }
    else if (strcmp(key, "log_outputs") == 0)
    {
        current_config.log_outputs = atoi(value);
    }
    else if (strcmp(key, "enable_visualization") == 0)
    {
        current_config.enable_visualization = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "max_file_size_mb") == 0)
    {
        current_config.max_file_size_mb = atoi(value);
    }
    else if (strcmp(key, "thread_count") == 0)
    {
        current_config.thread_count = atoi(value);
    }
    else if (strcmp(key, "enable_metrics") == 0)
    {
        // Parse bitfield string
        size_t len = strlen(value);
        for (size_t i = 0; i < len && i < 32; i++)
        {
            current_config.enable_metrics[i] = (value[i] == '1');
        }
    }
    // Metric configuration keys
    else if (strcmp(key, "metric_cyclomatic_complexity_enabled") == 0)
    {
        current_config.cyclomatic_complexity.enabled = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "metric_cyclomatic_complexity_weight") == 0)
    {
        current_config.cyclomatic_complexity.weight = atof(value);
    }
    else if (strcmp(key, "metric_cyclomatic_complexity_threshold") == 0)
    {
        current_config.cyclomatic_complexity.threshold = atof(value);
    }
    else if (strcmp(key, "metric_lines_of_code_enabled") == 0)
    {
        current_config.lines_of_code.enabled = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "metric_lines_of_code_weight") == 0)
    {
        current_config.lines_of_code.weight = atof(value);
    }
    else if (strcmp(key, "metric_lines_of_code_threshold") == 0)
    {
        current_config.lines_of_code.threshold = atof(value);
    }
    else if (strcmp(key, "metric_halstead_volume_enabled") == 0)
    {
        current_config.halstead_volume.enabled = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "metric_halstead_volume_weight") == 0)
    {
        current_config.halstead_volume.weight = atof(value);
    }
    else if (strcmp(key, "metric_halstead_volume_threshold") == 0)
    {
        current_config.halstead_volume.threshold = atof(value);
    }
    else if (strcmp(key, "metric_halstead_difficulty_enabled") == 0)
    {
        current_config.halstead_difficulty.enabled = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "metric_halstead_difficulty_weight") == 0)
    {
        current_config.halstead_difficulty.weight = atof(value);
    }
    else if (strcmp(key, "metric_halstead_difficulty_threshold") == 0)
    {
        current_config.halstead_difficulty.threshold = atof(value);
    }
    else if (strcmp(key, "metric_halstead_effort_enabled") == 0)
    {
        current_config.halstead_effort.enabled = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "metric_halstead_effort_weight") == 0)
    {
        current_config.halstead_effort.weight = atof(value);
    }
    else if (strcmp(key, "metric_halstead_effort_threshold") == 0)
    {
        current_config.halstead_effort.threshold = atof(value);
    }
    else if (strcmp(key, "metric_halstead_time_enabled") == 0)
    {
        current_config.halstead_time.enabled = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "metric_halstead_time_weight") == 0)
    {
        current_config.halstead_time.weight = atof(value);
    }
    else if (strcmp(key, "metric_halstead_time_threshold") == 0)
    {
        current_config.halstead_time.threshold = atof(value);
    }
    else if (strcmp(key, "metric_halstead_bugs_enabled") == 0)
    {
        current_config.halstead_bugs.enabled = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "metric_halstead_bugs_weight") == 0)
    {
        current_config.halstead_bugs.weight = atof(value);
    }
    else if (strcmp(key, "metric_halstead_bugs_threshold") == 0)
    {
        current_config.halstead_bugs.threshold = atof(value);
    }
    else if (strcmp(key, "metric_maintainability_index_enabled") == 0)
    {
        current_config.maintainability_index.enabled = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "metric_maintainability_index_weight") == 0)
    {
        current_config.maintainability_index.weight = atof(value);
    }
    else if (strcmp(key, "metric_maintainability_index_threshold") == 0)
    {
        current_config.maintainability_index.threshold = atof(value);
    }
    else if (strcmp(key, "metric_comment_density_enabled") == 0)
    {
        current_config.comment_density.enabled = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "metric_comment_density_weight") == 0)
    {
        current_config.comment_density.weight = atof(value);
    }
    else if (strcmp(key, "metric_comment_density_threshold") == 0)
    {
        current_config.comment_density.threshold = atof(value);
    }
    else if (strcmp(key, "metric_class_cohesion_enabled") == 0)
    {
        current_config.class_cohesion.enabled = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "metric_class_cohesion_weight") == 0)
    {
        current_config.class_cohesion.weight = atof(value);
    }
    else if (strcmp(key, "metric_class_cohesion_threshold") == 0)
    {
        current_config.class_cohesion.threshold = atof(value);
    }
    else if (strcmp(key, "metric_class_coupling_enabled") == 0)
    {
        current_config.class_coupling.enabled = (strcmp(value, "true") == 0);
    }
    else if (strcmp(key, "metric_class_coupling_weight") == 0)
    {
        current_config.class_coupling.weight = atof(value);
    }
    else if (strcmp(key, "metric_class_coupling_threshold") == 0)
    {
        current_config.class_coupling.threshold = atof(value);
    }
    else if (strcmp(key, "overall_quality_threshold") == 0)
    {
        current_config.overall_quality_threshold = atof(value);
    }
    else if (strcmp(key, "warning_threshold") == 0)
    {
        current_config.warning_threshold = atof(value);
    }
    else if (strcmp(key, "error_threshold") == 0)
    {
        current_config.error_threshold = atof(value);
    }
    else
    {
        LOG_WARNING("Unknown configuration key: %s", key);
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    return CQ_SUCCESS;
}

const char *config_get_string(const char *key)
{
    if (!config_initialized || !key)
    {
        return NULL;
    }

    if (strcmp(key, "log_file") == 0)
    {
        return current_config.log_file;
    }

    return NULL;
}

int config_get_int(const char *key, int default_value)
{
    if (!config_initialized || !key)
    {
        return default_value;
    }

    if (strcmp(key, "log_level") == 0)
    {
        return current_config.log_level;
    }
    else if (strcmp(key, "log_outputs") == 0)
    {
        return current_config.log_outputs;
    }
    else if (strcmp(key, "max_file_size_mb") == 0)
    {
        return current_config.max_file_size_mb;
    }
    else if (strcmp(key, "thread_count") == 0)
    {
        return current_config.thread_count;
    }

    return default_value;
}

bool config_get_bool(const char *key, bool default_value)
{
    if (!config_initialized || !key)
    {
        return default_value;
    }

    if (strcmp(key, "enable_visualization") == 0)
    {
        return current_config.enable_visualization;
    }

    return default_value;
}

const MetricConfig *config_get_metric_config(const char *metric_name)
{
    if (!config_initialized || !metric_name)
    {
        return NULL;
    }

    if (strcmp(metric_name, "cyclomatic_complexity") == 0)
    {
        return &current_config.cyclomatic_complexity;
    }
    else if (strcmp(metric_name, "lines_of_code") == 0)
    {
        return &current_config.lines_of_code;
    }
    else if (strcmp(metric_name, "halstead_volume") == 0)
    {
        return &current_config.halstead_volume;
    }
    else if (strcmp(metric_name, "halstead_difficulty") == 0)
    {
        return &current_config.halstead_difficulty;
    }
    else if (strcmp(metric_name, "halstead_effort") == 0)
    {
        return &current_config.halstead_effort;
    }
    else if (strcmp(metric_name, "halstead_time") == 0)
    {
        return &current_config.halstead_time;
    }
    else if (strcmp(metric_name, "halstead_bugs") == 0)
    {
        return &current_config.halstead_bugs;
    }
    else if (strcmp(metric_name, "maintainability_index") == 0)
    {
        return &current_config.maintainability_index;
    }
    else if (strcmp(metric_name, "comment_density") == 0)
    {
        return &current_config.comment_density;
    }
    else if (strcmp(metric_name, "class_cohesion") == 0)
    {
        return &current_config.class_cohesion;
    }
    else if (strcmp(metric_name, "class_coupling") == 0)
    {
        return &current_config.class_coupling;
    }

    return NULL;
}

double config_get_overall_quality_threshold(void)
{
    return config_initialized ? current_config.overall_quality_threshold : 70.0;
}

double config_get_warning_threshold(void)
{
    return config_initialized ? current_config.warning_threshold : 60.0;
}

double config_get_error_threshold(void)
{
    return config_initialized ? current_config.error_threshold : 40.0;
}
