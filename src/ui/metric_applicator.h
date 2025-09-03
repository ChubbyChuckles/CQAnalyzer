#ifndef METRIC_APPLICATOR_H
#define METRIC_APPLICATOR_H

#include "imgui_integration.h"
#include "analyzer/metric_calculator.h"

// Structure to hold calculated metric results with configuration applied
typedef struct {
    // Raw metric values
    int cyclomatic_complexity;
    int physical_loc;
    int logical_loc;
    int comment_loc;
    HalsteadMetrics halstead;
    double maintainability_index;
    double comment_density;
    double class_cohesion;
    double class_coupling;
    double dead_code_percentage;
    double duplication_percentage;

    // Configuration-applied values
    double normalized_complexity;
    double normalized_halstead;
    double normalized_maintainability;
    double normalized_comment_density;
    double normalized_cohesion;
    double normalized_coupling;
    double normalized_dead_code;
    double normalized_duplication;

    // Combined score
    double combined_score;

    // Threshold violations
    bool complexity_violation;
    bool halstead_violation;
    bool maintainability_violation;
    bool comment_density_violation;
    bool cohesion_violation;
    bool coupling_violation;
    bool dead_code_violation;
    bool duplication_violation;
} MetricResults;

// Apply metric configuration to raw metric values
bool apply_metric_configuration(const MetricConfig* config,
                               int raw_complexity,
                               int raw_physical_loc,
                               int raw_logical_loc,
                               int raw_comment_loc,
                               const HalsteadMetrics* raw_halstead,
                               double raw_maintainability,
                               double raw_comment_density,
                               double raw_cohesion,
                               double raw_coupling,
                               double raw_dead_code,
                               double raw_duplication,
                               MetricResults* results);

// Calculate combined score from configured metrics
double calculate_combined_score(const MetricConfig* config, const MetricResults* results);

// Check if any configured thresholds are violated
bool check_threshold_violations(const MetricConfig* config, const MetricResults* results);

// Get recommended actions based on metric results
void get_recommendations(const MetricConfig* config, const MetricResults* results,
                        char* recommendations, size_t max_length);

#endif // METRIC_APPLICATOR_H