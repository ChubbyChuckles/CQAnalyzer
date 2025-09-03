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

    // Enable common metrics by default
    current_config.enable_metrics[0] = true; // Cyclomatic complexity
    current_config.enable_metrics[1] = true; // Lines of code
    current_config.enable_metrics[2] = true; // Maintainability index

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
