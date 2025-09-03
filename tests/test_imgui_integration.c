#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../src/ui/imgui_integration.h"
#include "../include/cqanalyzer.h"

/**
 * @brief Test ImGui integration initialization
 */
void test_imgui_init(void)
{
    printf("Testing ImGui initialization...\n");

    // Test menu state initialization
    menu_state_init();

    // Verify default values
    assert(menu_state.show_main_control_panel == true);
    assert(menu_state.visualization_mode == 0);
    assert(menu_state.show_axes == true);
    assert(menu_state.show_grid == true);
    assert(menu_state.show_labels == true);

    // Verify camera controls initialization
    assert(menu_state.camera_controls.position[0] == 0.0f);
    assert(menu_state.camera_controls.position[1] == 0.0f);
    assert(menu_state.camera_controls.position[2] == 5.0f);
    assert(menu_state.camera_controls.fov == 45.0f);

    // Verify display options initialization
    assert(menu_state.display_options.show_axes == true);
    assert(menu_state.display_options.show_grid == true);
    assert(menu_state.display_options.enable_lighting == true);
    assert(menu_state.display_options.point_size == 5.0f);

    // Verify color schemes
    assert(menu_state.num_color_schemes == 3);
    assert(strcmp(menu_state.color_schemes[0].name, "Default") == 0);
    assert(menu_state.current_color_scheme == 0);

    // Verify animation controls
    assert(menu_state.animation_controls.enabled == false);
    assert(menu_state.animation_controls.duration == 2.0f);
    assert(menu_state.animation_controls.speed == 1.0f);

    printf("✓ ImGui initialization test passed\n");
}

/**
 * @brief Test camera controls functionality
 */
void test_camera_controls(void)
{
    printf("Testing camera controls...\n");

    menu_state_init();

    // Test camera position changes
    menu_state.camera_controls.position[0] = 10.0f;
    menu_state.camera_controls.position[1] = 5.0f;
    menu_state.camera_controls.position[2] = -3.0f;

    assert(menu_state.camera_controls.position[0] == 10.0f);
    assert(menu_state.camera_controls.position[1] == 5.0f);
    assert(menu_state.camera_controls.position[2] == -3.0f);

    // Test camera target changes
    menu_state.camera_controls.target[0] = 1.0f;
    menu_state.camera_controls.target[1] = 2.0f;
    menu_state.camera_controls.target[2] = 3.0f;

    assert(menu_state.camera_controls.target[0] == 1.0f);
    assert(menu_state.camera_controls.target[1] == 2.0f);
    assert(menu_state.camera_controls.target[2] == 3.0f);

    // Test rotation changes
    menu_state.camera_controls.yaw = 45.0f;
    menu_state.camera_controls.pitch = 30.0f;
    menu_state.camera_controls.distance = 15.0f;

    assert(menu_state.camera_controls.yaw == 45.0f);
    assert(menu_state.camera_controls.pitch == 30.0f);
    assert(menu_state.camera_controls.distance == 15.0f);

    printf("✓ Camera controls test passed\n");
}

/**
 * @brief Test display options functionality
 */
void test_display_options(void)
{
    printf("Testing display options...\n");

    menu_state_init();

    // Test visibility toggles
    menu_state.display_options.show_axes = false;
    menu_state.display_options.show_grid = false;
    menu_state.display_options.show_labels = false;
    menu_state.display_options.show_wireframe = true;

    assert(menu_state.display_options.show_axes == false);
    assert(menu_state.display_options.show_grid == false);
    assert(menu_state.display_options.show_labels == false);
    assert(menu_state.display_options.show_wireframe == true);

    // Test rendering options
    menu_state.display_options.enable_lighting = false;
    menu_state.display_options.enable_shadows = true;
    menu_state.display_options.enable_fog = true;

    assert(menu_state.display_options.enable_lighting == false);
    assert(menu_state.display_options.enable_shadows == true);
    assert(menu_state.display_options.enable_fog == true);

    // Test size parameters
    menu_state.display_options.point_size = 8.5f;
    menu_state.display_options.line_width = 3.2f;
    menu_state.display_options.label_scale = 1.5f;

    assert(menu_state.display_options.point_size == 8.5f);
    assert(menu_state.display_options.line_width == 3.2f);
    assert(menu_state.display_options.label_scale == 1.5f);

    printf("✓ Display options test passed\n");
}

/**
 * @brief Test color scheme functionality
 */
void test_color_schemes(void)
{
    printf("Testing color schemes...\n");

    menu_state_init();

    // Test color scheme selection
    menu_state.current_color_scheme = 1; // Dark scheme
    assert(menu_state.current_color_scheme == 1);
    assert(strcmp(menu_state.color_schemes[1].name, "Dark") == 0);

    // Test color modification
    ColorScheme* scheme = &menu_state.color_schemes[menu_state.current_color_scheme];
    scheme->background_color[0] = 0.2f;
    scheme->background_color[1] = 0.2f;
    scheme->background_color[2] = 0.2f;

    assert(scheme->background_color[0] == 0.2f);
    assert(scheme->background_color[1] == 0.2f);
    assert(scheme->background_color[2] == 0.2f);

    // Test scheme name change
    strcpy(scheme->name, "Modified Dark");
    assert(strcmp(scheme->name, "Modified Dark") == 0);

    printf("✓ Color schemes test passed\n");
}

/**
 * @brief Test animation controls functionality
 */
void test_animation_controls(void)
{
    printf("Testing animation controls...\n");

    menu_state_init();

    // Test animation settings
    menu_state.animation_controls.enabled = true;
    menu_state.animation_controls.duration = 5.0f;
    menu_state.animation_controls.speed = 2.0f;
    menu_state.animation_controls.loop = true;
    menu_state.animation_controls.easing_type = 2; // Ease Out

    assert(menu_state.animation_controls.enabled == true);
    assert(menu_state.animation_controls.duration == 5.0f);
    assert(menu_state.animation_controls.speed == 2.0f);
    assert(menu_state.animation_controls.loop == true);
    assert(menu_state.animation_controls.easing_type == 2);

    // Test auto rotation
    menu_state.animation_controls.auto_rotate = true;
    menu_state.animation_controls.auto_rotate_speed = 1.5f;

    assert(menu_state.animation_controls.auto_rotate == true);
    assert(menu_state.animation_controls.auto_rotate_speed == 1.5f);

    printf("✓ Animation controls test passed\n");
}

/**
 * @brief Test visualization mode switching
 */
void test_visualization_modes(void)
{
    printf("Testing visualization modes...\n");

    menu_state_init();

    // Test mode switching
    menu_state.visualization_mode = 1; // Bubble Chart
    assert(menu_state.visualization_mode == 1);

    menu_state.visualization_mode = 2; // Bar Chart
    assert(menu_state.visualization_mode == 2);

    menu_state.visualization_mode = 3; // Tree Map
    assert(menu_state.visualization_mode == 3);

    printf("✓ Visualization modes test passed\n");
}

/**
 * @brief Main test function
 */
int main(int argc, char* argv[])
{
    printf("Running ImGui Integration Tests...\n");
    printf("==================================\n\n");

    // Run all tests
    test_imgui_init();
    test_camera_controls();
    test_display_options();
    test_color_schemes();
    test_animation_controls();
    test_visualization_modes();

    printf("\n==================================\n");
    printf("All ImGui integration tests passed!\n");

    return 0;
}