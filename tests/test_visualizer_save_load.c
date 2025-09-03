#include <CUnit/CUnit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "visualizer/scene.h"
#include "visualizer/visualization_filters.h"

/**
 * @brief Test saving and loading visualization state
 */
void test_visualization_save_load(void)
{
    const char *test_file = "/tmp/test_visualization_state.bin";

    // Initialize scene
    CQError init_result = scene_init();
    CU_ASSERT_EQUAL(init_result, CQ_SUCCESS);

    if (init_result != CQ_SUCCESS)
    {
        return;
    }

    // Create a test visualization state
    VisualizationState original_state;
    memset(&original_state, 0, sizeof(VisualizationState));

    // Set up test data
    original_state.version = 1;
    original_state.mode = VISUALIZATION_SCATTER_PLOT;
    strncpy(original_state.x_metric, "complexity", sizeof(original_state.x_metric) - 1);
    strncpy(original_state.y_metric, "lines_of_code", sizeof(original_state.y_metric) - 1);
    strncpy(original_state.z_metric, "cyclomatic_complexity", sizeof(original_state.z_metric) - 1);
    strncpy(original_state.color_metric, "maintainability_index", sizeof(original_state.color_metric) - 1);

    // Set up camera state
    original_state.camera.position[0] = 5.0f;
    original_state.camera.position[1] = 3.0f;
    original_state.camera.position[2] = 10.0f;
    original_state.camera.target[0] = 0.0f;
    original_state.camera.target[1] = 0.0f;
    original_state.camera.target[2] = 0.0f;
    original_state.camera.up[0] = 0.0f;
    original_state.camera.up[1] = 1.0f;
    original_state.camera.up[2] = 0.0f;
    original_state.camera.fov = 45.0f;
    original_state.camera.near_plane = 0.1f;
    original_state.camera.far_plane = 1000.0f;

    // Set up display options
    original_state.display_options.show_axes = true;
    original_state.display_options.show_labels = false;
    original_state.display_options.show_grid = true;
    original_state.display_options.point_size = 2.5f;
    original_state.display_options.label_scale = 1.0f;

    // Apply the test state
    CQError set_result = scene_set_state(&original_state);
    CU_ASSERT_EQUAL(set_result, CQ_SUCCESS);

    // Save the state
    CQError save_result = scene_save_visualization_state(test_file);
    CU_ASSERT_EQUAL(save_result, CQ_SUCCESS);

    // Modify the current state to ensure loading works
    VisualizationState modified_state = original_state;
    modified_state.mode = VISUALIZATION_NONE;
    modified_state.display_options.show_axes = false;
    scene_set_state(&modified_state);

    // Load the state back
    CQError load_result = scene_load_visualization_state(test_file);
    CU_ASSERT_EQUAL(load_result, CQ_SUCCESS);

    // Get the current state and verify it matches the original
    VisualizationState loaded_state;
    CQError get_result = scene_get_current_state(&loaded_state);
    CU_ASSERT_EQUAL(get_result, CQ_SUCCESS);

    // Verify the loaded state matches the original
    CU_ASSERT_EQUAL(loaded_state.version, original_state.version);
    CU_ASSERT_EQUAL(loaded_state.mode, original_state.mode);
    CU_ASSERT_STRING_EQUAL(loaded_state.x_metric, original_state.x_metric);
    CU_ASSERT_STRING_EQUAL(loaded_state.y_metric, original_state.y_metric);
    CU_ASSERT_STRING_EQUAL(loaded_state.z_metric, original_state.z_metric);
    CU_ASSERT_STRING_EQUAL(loaded_state.color_metric, original_state.color_metric);

    // Verify camera state
    CU_ASSERT_DOUBLE_EQUAL(loaded_state.camera.position[0], original_state.camera.position[0], 0.001);
    CU_ASSERT_DOUBLE_EQUAL(loaded_state.camera.position[1], original_state.camera.position[1], 0.001);
    CU_ASSERT_DOUBLE_EQUAL(loaded_state.camera.position[2], original_state.camera.position[2], 0.001);
    CU_ASSERT_DOUBLE_EQUAL(loaded_state.camera.fov, original_state.camera.fov, 0.001);

    // Verify display options
    CU_ASSERT_EQUAL(loaded_state.display_options.show_axes, original_state.display_options.show_axes);
    CU_ASSERT_EQUAL(loaded_state.display_options.show_labels, original_state.display_options.show_labels);
    CU_ASSERT_EQUAL(loaded_state.display_options.show_grid, original_state.display_options.show_grid);
    CU_ASSERT_DOUBLE_EQUAL(loaded_state.display_options.point_size, original_state.display_options.point_size, 0.001);

    // Clean up
    scene_shutdown();
    remove(test_file);
}

/**
 * @brief Add visualizer tests to the test suite
 */
void add_visualizer_tests(CU_pSuite suite)
{
    CU_add_test(suite, "Visualization Save/Load", test_visualization_save_load);
}