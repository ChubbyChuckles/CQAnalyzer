#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../src/ui/metric_config.h"
#include "../src/ui/metric_applicator.h"
#include "../src/ui/imgui_integration.h"

// Test metric configuration save/load
void test_metric_config_save_load(void)
{
    printf("Testing metric configuration save/load...\n");

    MetricConfig original_config = {0};
    MetricConfig loaded_config = {0};

    // Initialize with test values
    original_config.enable_cyclomatic_complexity = true;
    original_config.enable_lines_of_code = false;
    original_config.enable_halstead_metrics = true;
    original_config.cyclomatic_complexity_threshold = 15.0f;
    original_config.halstead_volume_threshold = 2000.0f;
    original_config.normalization_method = 1; // Z-Score
    original_config.auto_normalize = false;
    strcpy(original_config.current_preset_name, "TestPreset");

    // Save configuration
    bool save_result = metric_config_save_to_file("test_config.json", &original_config);
    assert(save_result == true);

    // Load configuration
    bool load_result = metric_config_load_from_file("test_config.json", &loaded_config);
    assert(load_result == true);

    // Verify loaded values match original
    assert(loaded_config.enable_cyclomatic_complexity == original_config.enable_cyclomatic_complexity);
    assert(loaded_config.enable_lines_of_code == original_config.enable_lines_of_code);
    assert(loaded_config.enable_halstead_metrics == original_config.enable_halstead_metrics);
    assert(loaded_config.cyclomatic_complexity_threshold == original_config.cyclomatic_complexity_threshold);
    assert(loaded_config.halstead_volume_threshold == original_config.halstead_volume_threshold);
    assert(loaded_config.normalization_method == original_config.normalization_method);
    assert(loaded_config.auto_normalize == original_config.auto_normalize);

    // Clean up
    remove("test_config.json");

    printf("✓ Metric configuration save/load test passed\n");
}

// Test metric applicator
void test_metric_applicator(void)
{
    printf("Testing metric applicator...\n");

    MetricConfig config = {0};
    MetricResults results = {0};

    // Initialize config
    config.enable_cyclomatic_complexity = true;
    config.enable_halstead_metrics = true;
    config.enable_maintainability_index = true;
    config.cyclomatic_complexity_threshold = 10.0f;
    config.halstead_volume_threshold = 1000.0f;
    config.maintainability_index_threshold = 50.0f;
    config.cyclomatic_complexity_weight = 0.5f;
    config.halstead_metrics_weight = 0.3f;
    config.maintainability_index_weight = 0.2f;
    config.auto_normalize = true;
    config.normalization_method = 0; // Min-Max

    // Sample Halstead metrics
    HalsteadMetrics halstead = {
        .n1 = 10, .n2 = 20, .N1 = 100, .N2 = 150,
        .volume = 800.0, .difficulty = 15.0, .effort = 12000.0,
        .time = 667.0, .bugs = 0.5
    };

    // Apply configuration
    bool apply_result = apply_metric_configuration(&config,
                                                  12,   // complexity (above threshold)
                                                  500,  // physical LOC
                                                  400,  // logical LOC
                                                  50,   // comment LOC
                                                  &halstead,
                                                  45.0, // maintainability (below threshold)
                                                  10.0, // comment density
                                                  0.6,  // cohesion
                                                  0.7,  // coupling
                                                  25.0, // dead code %
                                                  35.0, // duplication %
                                                  &results);

    assert(apply_result == true);

    // Verify results
    assert(results.cyclomatic_complexity == 12);
    assert(results.halstead.volume == 800.0);
    assert(results.maintainability_index == 45.0);
    assert(results.complexity_violation == true);  // 12 > 10
    assert(results.halstead_violation == false);   // 800 < 1000
    assert(results.maintainability_violation == true); // 45 < 50

    // Verify combined score calculation
    double combined_score = calculate_combined_score(&config, &results);
    assert(combined_score >= 0.0 && combined_score <= 100.0);

    // Verify threshold violations check
    bool has_violations = check_threshold_violations(&config, &results);
    assert(has_violations == true);

    printf("✓ Metric applicator test passed\n");
}

// Test preset loading
void test_preset_loading(void)
{
    printf("Testing preset loading...\n");

    MetricConfig config = {0};

    // Test code quality preset
    metric_config_load_code_quality_preset(&config);
    assert(config.enable_cyclomatic_complexity == true);
    assert(config.enable_maintainability_index == true);
    assert(config.cyclomatic_complexity_threshold == 8.0f);
    assert(strcmp(config.current_preset_name, "Code Quality Focus") == 0);

    // Test performance preset
    metric_config_load_performance_preset(&config);
    assert(config.enable_cyclomatic_complexity == true);
    assert(config.enable_lines_of_code == false);
    assert(config.enable_maintainability_index == false);
    assert(config.cyclomatic_complexity_threshold == 12.0f);
    assert(strcmp(config.current_preset_name, "Performance Focus") == 0);

    // Test maintainability preset
    metric_config_load_maintainability_preset(&config);
    assert(config.enable_cyclomatic_complexity == true);
    assert(config.enable_maintainability_index == true);
    assert(config.enable_comment_density == true);
    assert(config.maintainability_index_threshold == 70.0f);
    assert(strcmp(config.current_preset_name, "Maintainability Focus") == 0);

    printf("✓ Preset loading test passed\n");
}

// Test recommendations generation
void test_recommendations(void)
{
    printf("Testing recommendations generation...\n");

    MetricConfig config = {0};
    MetricResults results = {0};
    char recommendations[1024] = "";

    // Initialize config with all metrics enabled
    config.enable_cyclomatic_complexity = true;
    config.enable_maintainability_index = true;
    config.enable_comment_density = true;
    config.enable_dead_code_detection = true;
    config.enable_duplication_detection = true;

    // Set thresholds
    config.cyclomatic_complexity_threshold = 10.0f;
    config.maintainability_index_threshold = 50.0f;
    config.comment_density_threshold = 15.0f;
    config.dead_code_percentage_threshold = 20.0f;
    config.duplication_percentage_threshold = 30.0f;

    // Create violations
    results.complexity_violation = true;
    results.maintainability_violation = true;
    results.comment_density_violation = true;
    results.dead_code_violation = true;
    results.duplication_violation = true;

    // Generate recommendations
    get_recommendations(&config, &results, recommendations, sizeof(recommendations));

    // Verify recommendations contain expected content
    assert(strstr(recommendations, "Refactor complex functions") != NULL);
    assert(strstr(recommendations, "Improve code maintainability") != NULL);
    assert(strstr(recommendations, "Add more documentation comments") != NULL);
    assert(strstr(recommendations, "Remove dead/unused code") != NULL);
    assert(strstr(recommendations, "Eliminate code duplication") != NULL);

    printf("✓ Recommendations test passed\n");
}

int main(void)
{
    printf("Running metric configuration tests...\n\n");

    test_metric_config_save_load();
    test_metric_applicator();
    test_preset_loading();
    test_recommendations();

    printf("\n✓ All metric configuration tests passed!\n");
    return 0;
}