#include "metric_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#define PRESETS_DIR "presets"
#define CONFIG_FILE "metric_config.json"

// Create presets directory if it doesn't exist
static bool ensure_presets_directory(void)
{
    struct stat st = {0};
    if (stat(PRESETS_DIR, &st) == -1) {
        if (mkdir(PRESETS_DIR, 0755) == -1) {
            fprintf(stderr, "Failed to create presets directory: %s\n", strerror(errno));
            return false;
        }
    }
    return true;
}

// Save preset to file
bool metric_config_save_preset(const char* preset_name, const MetricConfig* config)
{
    if (!preset_name || !config) return false;

    if (!ensure_presets_directory()) return false;

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s.json", PRESETS_DIR, preset_name);

    return metric_config_save_to_file(filename, config);
}

// Load preset from file
bool metric_config_load_preset(const char* preset_name, MetricConfig* config)
{
    if (!preset_name || !config) return false;

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s.json", PRESETS_DIR, preset_name);

    return metric_config_load_from_file(filename, config);
}

// Delete preset file
bool metric_config_delete_preset(const char* preset_name)
{
    if (!preset_name) return false;

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s.json", PRESETS_DIR, preset_name);

    if (remove(filename) == 0) {
        printf("Preset '%s' deleted successfully\n", preset_name);
        return true;
    } else {
        fprintf(stderr, "Failed to delete preset '%s': %s\n", preset_name, strerror(errno));
        return false;
    }
}

// List available presets
bool metric_config_list_presets(char presets[][64], int* count, int max_count)
{
    if (!presets || !count) return false;

    *count = 0;

    DIR* dir = opendir(PRESETS_DIR);
    if (!dir) {
        // If directory doesn't exist, return empty list
        return true;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL && *count < max_count) {
        if (strstr(entry->d_name, ".json")) {
            // Remove .json extension
            char* dot = strrchr(entry->d_name, '.');
            if (dot) *dot = '\0';

            strncpy(presets[*count], entry->d_name, 63);
            presets[*count][63] = '\0';
            (*count)++;
        }
    }

    closedir(dir);
    return true;
}

// Save configuration to JSON file (simplified implementation)
bool metric_config_save_to_file(const char* filename, const MetricConfig* config)
{
    if (!filename || !config) return false;

    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Failed to open file for writing: %s\n", filename);
        return false;
    }

    fprintf(file, "{\n");
    fprintf(file, "  \"enable_cyclomatic_complexity\": %s,\n", config->enable_cyclomatic_complexity ? "true" : "false");
    fprintf(file, "  \"enable_lines_of_code\": %s,\n", config->enable_lines_of_code ? "true" : "false");
    fprintf(file, "  \"enable_halstead_metrics\": %s,\n", config->enable_halstead_metrics ? "true" : "false");
    fprintf(file, "  \"enable_maintainability_index\": %s,\n", config->enable_maintainability_index ? "true" : "false");
    fprintf(file, "  \"enable_comment_density\": %s,\n", config->enable_comment_density ? "true" : "false");
    fprintf(file, "  \"enable_class_cohesion\": %s,\n", config->enable_class_cohesion ? "true" : "false");
    fprintf(file, "  \"enable_class_coupling\": %s,\n", config->enable_class_coupling ? "true" : "false");
    fprintf(file, "  \"enable_dead_code_detection\": %s,\n", config->enable_dead_code_detection ? "true" : "false");
    fprintf(file, "  \"enable_duplication_detection\": %s,\n", config->enable_duplication_detection ? "true" : "false");

    fprintf(file, "  \"cyclomatic_complexity_threshold\": %.2f,\n", config->cyclomatic_complexity_threshold);
    fprintf(file, "  \"halstead_volume_threshold\": %.2f,\n", config->halstead_volume_threshold);
    fprintf(file, "  \"halstead_difficulty_threshold\": %.2f,\n", config->halstead_difficulty_threshold);
    fprintf(file, "  \"halstead_effort_threshold\": %.2f,\n", config->halstead_effort_threshold);
    fprintf(file, "  \"maintainability_index_threshold\": %.2f,\n", config->maintainability_index_threshold);
    fprintf(file, "  \"comment_density_threshold\": %.2f,\n", config->comment_density_threshold);
    fprintf(file, "  \"class_cohesion_threshold\": %.2f,\n", config->class_cohesion_threshold);
    fprintf(file, "  \"class_coupling_threshold\": %.2f,\n", config->class_coupling_threshold);
    fprintf(file, "  \"dead_code_percentage_threshold\": %.2f,\n", config->dead_code_percentage_threshold);
    fprintf(file, "  \"duplication_percentage_threshold\": %.2f,\n", config->duplication_percentage_threshold);

    fprintf(file, "  \"cyclomatic_complexity_weight\": %.3f,\n", config->cyclomatic_complexity_weight);
    fprintf(file, "  \"halstead_metrics_weight\": %.3f,\n", config->halstead_metrics_weight);
    fprintf(file, "  \"maintainability_index_weight\": %.3f,\n", config->maintainability_index_weight);
    fprintf(file, "  \"comment_density_weight\": %.3f,\n", config->comment_density_weight);
    fprintf(file, "  \"class_cohesion_weight\": %.3f,\n", config->class_cohesion_weight);
    fprintf(file, "  \"class_coupling_weight\": %.3f,\n", config->class_coupling_weight);
    fprintf(file, "  \"dead_code_weight\": %.3f,\n", config->dead_code_weight);
    fprintf(file, "  \"duplication_weight\": %.3f,\n", config->duplication_weight);

    fprintf(file, "  \"normalization_method\": %d,\n", config->normalization_method);
    fprintf(file, "  \"auto_normalize\": %s\n", config->auto_normalize ? "true" : "false");
    fprintf(file, "}\n");

    fclose(file);
    printf("Configuration saved to: %s\n", filename);
    return true;
}

// Load configuration from JSON file (simplified implementation)
bool metric_config_load_from_file(const char* filename, MetricConfig* config)
{
    if (!filename || !config) return false;

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file for reading: %s\n", filename);
        return false;
    }

    // For simplicity, we'll use a basic parsing approach
    // In a real implementation, you'd use a proper JSON parser
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Parse boolean values
        if (strstr(line, "\"enable_cyclomatic_complexity\"")) {
            config->enable_cyclomatic_complexity = strstr(line, "true") != NULL;
        } else if (strstr(line, "\"enable_lines_of_code\"")) {
            config->enable_lines_of_code = strstr(line, "true") != NULL;
        } else if (strstr(line, "\"enable_halstead_metrics\"")) {
            config->enable_halstead_metrics = strstr(line, "true") != NULL;
        } else if (strstr(line, "\"enable_maintainability_index\"")) {
            config->enable_maintainability_index = strstr(line, "true") != NULL;
        } else if (strstr(line, "\"enable_comment_density\"")) {
            config->enable_comment_density = strstr(line, "true") != NULL;
        } else if (strstr(line, "\"enable_class_cohesion\"")) {
            config->enable_class_cohesion = strstr(line, "true") != NULL;
        } else if (strstr(line, "\"enable_class_coupling\"")) {
            config->enable_class_coupling = strstr(line, "true") != NULL;
        } else if (strstr(line, "\"enable_dead_code_detection\"")) {
            config->enable_dead_code_detection = strstr(line, "true") != NULL;
        } else if (strstr(line, "\"enable_duplication_detection\"")) {
            config->enable_duplication_detection = strstr(line, "true") != NULL;
        } else if (strstr(line, "\"auto_normalize\"")) {
            config->auto_normalize = strstr(line, "true") != NULL;
        }
        // Parse numeric values (simplified - would need better parsing in real implementation)
        else if (strstr(line, "\"cyclomatic_complexity_threshold\"")) {
            sscanf(line, "  \"cyclomatic_complexity_threshold\": %f,", &config->cyclomatic_complexity_threshold);
        } else if (strstr(line, "\"halstead_volume_threshold\"")) {
            sscanf(line, "  \"halstead_volume_threshold\": %f,", &config->halstead_volume_threshold);
        } else if (strstr(line, "\"halstead_difficulty_threshold\"")) {
            sscanf(line, "  \"halstead_difficulty_threshold\": %f,", &config->halstead_difficulty_threshold);
        } else if (strstr(line, "\"halstead_effort_threshold\"")) {
            sscanf(line, "  \"halstead_effort_threshold\": %f,", &config->halstead_effort_threshold);
        } else if (strstr(line, "\"maintainability_index_threshold\"")) {
            sscanf(line, "  \"maintainability_index_threshold\": %f,", &config->maintainability_index_threshold);
        } else if (strstr(line, "\"comment_density_threshold\"")) {
            sscanf(line, "  \"comment_density_threshold\": %f,", &config->comment_density_threshold);
        } else if (strstr(line, "\"class_cohesion_threshold\"")) {
            sscanf(line, "  \"class_cohesion_threshold\": %f,", &config->class_cohesion_threshold);
        } else if (strstr(line, "\"class_coupling_threshold\"")) {
            sscanf(line, "  \"class_coupling_threshold\": %f,", &config->class_coupling_threshold);
        } else if (strstr(line, "\"dead_code_percentage_threshold\"")) {
            sscanf(line, "  \"dead_code_percentage_threshold\": %f,", &config->dead_code_percentage_threshold);
        } else if (strstr(line, "\"duplication_percentage_threshold\"")) {
            sscanf(line, "  \"duplication_percentage_threshold\": %f,", &config->duplication_percentage_threshold);
        } else if (strstr(line, "\"cyclomatic_complexity_weight\"")) {
            sscanf(line, "  \"cyclomatic_complexity_weight\": %f,", &config->cyclomatic_complexity_weight);
        } else if (strstr(line, "\"halstead_metrics_weight\"")) {
            sscanf(line, "  \"halstead_metrics_weight\": %f,", &config->halstead_metrics_weight);
        } else if (strstr(line, "\"maintainability_index_weight\"")) {
            sscanf(line, "  \"maintainability_index_weight\": %f,", &config->maintainability_index_weight);
        } else if (strstr(line, "\"comment_density_weight\"")) {
            sscanf(line, "  \"comment_density_weight\": %f,", &config->comment_density_weight);
        } else if (strstr(line, "\"class_cohesion_weight\"")) {
            sscanf(line, "  \"class_cohesion_weight\": %f,", &config->class_cohesion_weight);
        } else if (strstr(line, "\"class_coupling_weight\"")) {
            sscanf(line, "  \"class_coupling_weight\": %f,", &config->class_coupling_weight);
        } else if (strstr(line, "\"dead_code_weight\"")) {
            sscanf(line, "  \"dead_code_weight\": %f,", &config->dead_code_weight);
        } else if (strstr(line, "\"duplication_weight\"")) {
            sscanf(line, "  \"duplication_weight\": %f,", &config->duplication_weight);
        } else if (strstr(line, "\"normalization_method\"")) {
            sscanf(line, "  \"normalization_method\": %d,", &config->normalization_method);
        }
    }

    fclose(file);
    printf("Configuration loaded from: %s\n", filename);
    return true;
}

// Load predefined presets
void metric_config_load_code_quality_preset(MetricConfig* config)
{
    if (!config) return;

    // Focus on code quality metrics
    config->enable_cyclomatic_complexity = true;
    config->enable_lines_of_code = true;
    config->enable_halstead_metrics = true;
    config->enable_maintainability_index = true;
    config->enable_comment_density = true;
    config->enable_class_cohesion = true;
    config->enable_class_coupling = true;
    config->enable_dead_code_detection = true;
    config->enable_duplication_detection = true;

    // Stricter thresholds for code quality
    config->cyclomatic_complexity_threshold = 8.0f;
    config->halstead_volume_threshold = 800.0f;
    config->halstead_difficulty_threshold = 15.0f;
    config->halstead_effort_threshold = 15000.0f;
    config->maintainability_index_threshold = 60.0f;
    config->comment_density_threshold = 20.0f;
    config->class_cohesion_threshold = 0.6f;
    config->class_coupling_threshold = 0.6f;
    config->dead_code_percentage_threshold = 15.0f;
    config->duplication_percentage_threshold = 25.0f;

    // Balanced weights
    config->cyclomatic_complexity_weight = 0.15f;
    config->halstead_metrics_weight = 0.15f;
    config->maintainability_index_weight = 0.2f;
    config->comment_density_weight = 0.15f;
    config->class_cohesion_weight = 0.1f;
    config->class_coupling_weight = 0.1f;
    config->dead_code_weight = 0.1f;
    config->duplication_weight = 0.05f;

    config->normalization_method = 0; // Min-Max
    config->auto_normalize = true;
    strcpy(config->current_preset_name, "Code Quality Focus");
}

void metric_config_load_performance_preset(MetricConfig* config)
{
    if (!config) return;

    // Focus on performance-related metrics
    config->enable_cyclomatic_complexity = true;
    config->enable_lines_of_code = false;
    config->enable_halstead_metrics = true;
    config->enable_maintainability_index = false;
    config->enable_comment_density = false;
    config->enable_class_cohesion = false;
    config->enable_class_coupling = true;
    config->enable_dead_code_detection = true;
    config->enable_duplication_detection = false;

    // Performance-focused thresholds
    config->cyclomatic_complexity_threshold = 12.0f;
    config->halstead_volume_threshold = 1200.0f;
    config->halstead_difficulty_threshold = 25.0f;
    config->halstead_effort_threshold = 25000.0f;
    config->maintainability_index_threshold = 40.0f;
    config->comment_density_threshold = 10.0f;
    config->class_cohesion_threshold = 0.4f;
    config->class_coupling_threshold = 0.8f;
    config->dead_code_percentage_threshold = 25.0f;
    config->duplication_percentage_threshold = 40.0f;

    // Performance weights
    config->cyclomatic_complexity_weight = 0.3f;
    config->halstead_metrics_weight = 0.4f;
    config->maintainability_index_weight = 0.1f;
    config->comment_density_weight = 0.05f;
    config->class_cohesion_weight = 0.05f;
    config->class_coupling_weight = 0.05f;
    config->dead_code_weight = 0.05f;
    config->duplication_weight = 0.05f;

    config->normalization_method = 1; // Z-Score
    config->auto_normalize = true;
    strcpy(config->current_preset_name, "Performance Focus");
}

void metric_config_load_maintainability_preset(MetricConfig* config)
{
    if (!config) return;

    // Focus on maintainability metrics
    config->enable_cyclomatic_complexity = true;
    config->enable_lines_of_code = true;
    config->enable_halstead_metrics = false;
    config->enable_maintainability_index = true;
    config->enable_comment_density = true;
    config->enable_class_cohesion = true;
    config->enable_class_coupling = true;
    config->enable_dead_code_detection = false;
    config->enable_duplication_detection = true;

    // Maintainability-focused thresholds
    config->cyclomatic_complexity_threshold = 15.0f;
    config->halstead_volume_threshold = 1500.0f;
    config->halstead_difficulty_threshold = 30.0f;
    config->halstead_effort_threshold = 35000.0f;
    config->maintainability_index_threshold = 70.0f;
    config->comment_density_threshold = 25.0f;
    config->class_cohesion_threshold = 0.7f;
    config->class_coupling_threshold = 0.5f;
    config->dead_code_percentage_threshold = 10.0f;
    config->duplication_percentage_threshold = 20.0f;

    // Maintainability weights
    config->cyclomatic_complexity_weight = 0.1f;
    config->halstead_metrics_weight = 0.05f;
    config->maintainability_index_weight = 0.3f;
    config->comment_density_weight = 0.2f;
    config->class_cohesion_weight = 0.15f;
    config->class_coupling_weight = 0.15f;
    config->dead_code_weight = 0.025f;
    config->duplication_weight = 0.025f;

    config->normalization_method = 0; // Min-Max
    config->auto_normalize = true;
    strcpy(config->current_preset_name, "Maintainability Focus");
}