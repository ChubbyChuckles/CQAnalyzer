#ifndef METRIC_CONFIG_H
#define METRIC_CONFIG_H

#include <stdbool.h>
#include "imgui_integration.h"

// Preset management functions
bool metric_config_save_preset(const char* preset_name, const MetricConfig* config);
bool metric_config_load_preset(const char* preset_name, MetricConfig* config);
bool metric_config_delete_preset(const char* preset_name);
bool metric_config_list_presets(char presets[][64], int* count, int max_count);

// Configuration file management
bool metric_config_save_to_file(const char* filename, const MetricConfig* config);
bool metric_config_load_from_file(const char* filename, MetricConfig* config);

// Default configurations
void metric_config_load_code_quality_preset(MetricConfig* config);
void metric_config_load_performance_preset(MetricConfig* config);
void metric_config_load_maintainability_preset(MetricConfig* config);

#endif // METRIC_CONFIG_H