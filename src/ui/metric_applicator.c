#include "metric_applicator.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

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
                               MetricResults* results)
{
    if (!config || !raw_halstead || !results) {
        return false;
    }

    // Store raw values
    results->cyclomatic_complexity = raw_complexity;
    results->physical_loc = raw_physical_loc;
    results->logical_loc = raw_logical_loc;
    results->comment_loc = raw_comment_loc;
    memcpy(&results->halstead, raw_halstead, sizeof(HalsteadMetrics));
    results->maintainability_index = raw_maintainability;
    results->comment_density = raw_comment_density;
    results->class_cohesion = raw_cohesion;
    results->class_coupling = raw_coupling;
    results->dead_code_percentage = raw_dead_code;
    results->duplication_percentage = raw_duplication;

    // Apply normalization if enabled
    if (config->auto_normalize) {
        // Normalize each enabled metric
        if (config->enable_cyclomatic_complexity) {
            results->normalized_complexity = normalize_metric(
                (double)raw_complexity, 1.0, 50.0, 10.0, 5.0,
                (NormalizationMethod)config->normalization_method);
        }

        if (config->enable_halstead_metrics) {
            results->normalized_halstead = normalize_metric(
                raw_halstead->volume, 100.0, 10000.0, 1000.0, 500.0,
                (NormalizationMethod)config->normalization_method);
        }

        if (config->enable_maintainability_index) {
            results->normalized_maintainability = normalize_metric(
                raw_maintainability, 0.0, 100.0, 50.0, 20.0,
                (NormalizationMethod)config->normalization_method);
        }

        if (config->enable_comment_density) {
            results->normalized_comment_density = normalize_metric(
                raw_comment_density, 0.0, 50.0, 15.0, 10.0,
                (NormalizationMethod)config->normalization_method);
        }

        if (config->enable_class_cohesion) {
            results->normalized_cohesion = normalize_metric(
                raw_cohesion, 0.0, 1.0, 0.5, 0.2,
                (NormalizationMethod)config->normalization_method);
        }

        if (config->enable_class_coupling) {
            results->normalized_coupling = normalize_metric(
                raw_coupling, 0.0, 1.0, 0.5, 0.2,
                (NormalizationMethod)config->normalization_method);
        }

        if (config->enable_dead_code_detection) {
            results->normalized_dead_code = normalize_metric(
                raw_dead_code, 0.0, 100.0, 20.0, 15.0,
                (NormalizationMethod)config->normalization_method);
        }

        if (config->enable_duplication_detection) {
            results->normalized_duplication = normalize_metric(
                raw_duplication, 0.0, 100.0, 30.0, 20.0,
                (NormalizationMethod)config->normalization_method);
        }
    } else {
        // Use raw values directly
        results->normalized_complexity = (double)raw_complexity;
        results->normalized_halstead = raw_halstead->volume;
        results->normalized_maintainability = raw_maintainability;
        results->normalized_comment_density = raw_comment_density;
        results->normalized_cohesion = raw_cohesion;
        results->normalized_coupling = raw_coupling;
        results->normalized_dead_code = raw_dead_code;
        results->normalized_duplication = raw_duplication;
    }

    // Check threshold violations
    results->complexity_violation = config->enable_cyclomatic_complexity &&
                                   ((double)raw_complexity > config->cyclomatic_complexity_threshold);

    results->halstead_violation = config->enable_halstead_metrics &&
                                 (raw_halstead->volume > config->halstead_volume_threshold ||
                                  raw_halstead->difficulty > config->halstead_difficulty_threshold ||
                                  raw_halstead->effort > config->halstead_effort_threshold);

    results->maintainability_violation = config->enable_maintainability_index &&
                                        (raw_maintainability < config->maintainability_index_threshold);

    results->comment_density_violation = config->enable_comment_density &&
                                        (raw_comment_density < config->comment_density_threshold);

    results->cohesion_violation = config->enable_class_cohesion &&
                                 (raw_cohesion < config->class_cohesion_threshold);

    results->coupling_violation = config->enable_class_coupling &&
                                 (raw_coupling > config->class_coupling_threshold);

    results->dead_code_violation = config->enable_dead_code_detection &&
                                  (raw_dead_code > config->dead_code_percentage_threshold);

    results->duplication_violation = config->enable_duplication_detection &&
                                    (raw_duplication > config->duplication_percentage_threshold);

    // Calculate combined score
    results->combined_score = calculate_combined_score(config, results);

    return true;
}

// Calculate combined score from configured metrics
double calculate_combined_score(const MetricConfig* config, const MetricResults* results)
{
    if (!config || !results) {
        return 0.0;
    }

    double total_weight = 0.0;
    double weighted_score = 0.0;

    // Calculate weighted score for each enabled metric
    if (config->enable_cyclomatic_complexity) {
        double score = 1.0 - (results->normalized_complexity / 50.0); // Lower complexity is better
        if (score < 0.0) score = 0.0;
        weighted_score += score * config->cyclomatic_complexity_weight;
        total_weight += config->cyclomatic_complexity_weight;
    }

    if (config->enable_halstead_metrics) {
        double score = 1.0 - (results->normalized_halstead / 10000.0); // Lower volume is better
        if (score < 0.0) score = 0.0;
        weighted_score += score * config->halstead_metrics_weight;
        total_weight += config->halstead_metrics_weight;
    }

    if (config->enable_maintainability_index) {
        double score = results->normalized_maintainability / 100.0; // Higher maintainability is better
        weighted_score += score * config->maintainability_index_weight;
        total_weight += config->maintainability_index_weight;
    }

    if (config->enable_comment_density) {
        double score = results->normalized_comment_density / 50.0; // Higher comment density is better
        if (score > 1.0) score = 1.0;
        weighted_score += score * config->comment_density_weight;
        total_weight += config->comment_density_weight;
    }

    if (config->enable_class_cohesion) {
        double score = results->normalized_cohesion; // Higher cohesion is better
        weighted_score += score * config->class_cohesion_weight;
        total_weight += config->class_cohesion_weight;
    }

    if (config->enable_class_coupling) {
        double score = 1.0 - results->normalized_coupling; // Lower coupling is better
        weighted_score += score * config->class_coupling_weight;
        total_weight += config->class_coupling_weight;
    }

    if (config->enable_dead_code_detection) {
        double score = 1.0 - (results->normalized_dead_code / 100.0); // Lower dead code is better
        weighted_score += score * config->dead_code_weight;
        total_weight += config->dead_code_weight;
    }

    if (config->enable_duplication_detection) {
        double score = 1.0 - (results->normalized_duplication / 100.0); // Lower duplication is better
        weighted_score += score * config->duplication_weight;
        total_weight += config->duplication_weight;
    }

    // Return weighted average
    return total_weight > 0.0 ? (weighted_score / total_weight) * 100.0 : 0.0;
}

// Check if any configured thresholds are violated
bool check_threshold_violations(const MetricConfig* config, const MetricResults* results)
{
    if (!config || !results) {
        return false;
    }

    return results->complexity_violation ||
           results->halstead_violation ||
           results->maintainability_violation ||
           results->comment_density_violation ||
           results->cohesion_violation ||
           results->coupling_violation ||
           results->dead_code_violation ||
           results->duplication_violation;
}

// Get recommended actions based on metric results
void get_recommendations(const MetricConfig* config, const MetricResults* results,
                        char* recommendations, size_t max_length)
{
    if (!config || !results || !recommendations || max_length == 0) {
        return;
    }

    recommendations[0] = '\0';
    size_t current_length = 0;

    // Add recommendations based on violations
    if (results->complexity_violation && current_length < max_length - 50) {
        strncat(recommendations, "• Refactor complex functions (CC > threshold)\n", max_length - current_length - 1);
        current_length = strlen(recommendations);
    }

    if (results->halstead_violation && current_length < max_length - 50) {
        strncat(recommendations, "• Simplify complex algorithms (high Halstead metrics)\n", max_length - current_length - 1);
        current_length = strlen(recommendations);
    }

    if (results->maintainability_violation && current_length < max_length - 50) {
        strncat(recommendations, "• Improve code maintainability (add comments, refactor)\n", max_length - current_length - 1);
        current_length = strlen(recommendations);
    }

    if (results->comment_density_violation && current_length < max_length - 50) {
        strncat(recommendations, "• Add more documentation comments\n", max_length - current_length - 1);
        current_length = strlen(recommendations);
    }

    if (results->cohesion_violation && current_length < max_length - 50) {
        strncat(recommendations, "• Improve class cohesion (group related methods)\n", max_length - current_length - 1);
        current_length = strlen(recommendations);
    }

    if (results->coupling_violation && current_length < max_length - 50) {
        strncat(recommendations, "• Reduce class coupling (minimize dependencies)\n", max_length - current_length - 1);
        current_length = strlen(recommendations);
    }

    if (results->dead_code_violation && current_length < max_length - 50) {
        strncat(recommendations, "• Remove dead/unused code\n", max_length - current_length - 1);
        current_length = strlen(recommendations);
    }

    if (results->duplication_violation && current_length < max_length - 50) {
        strncat(recommendations, "• Eliminate code duplication (extract common code)\n", max_length - current_length - 1);
        current_length = strlen(recommendations);
    }

    // If no violations, add general recommendations
    if (strlen(recommendations) == 0 && current_length < max_length - 50) {
        strncat(recommendations, "• Code quality metrics are within acceptable ranges\n", max_length - current_length - 1);
    }
}