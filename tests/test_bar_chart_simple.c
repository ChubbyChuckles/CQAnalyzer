#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "visualizer/bar_chart.h"
#include "visualizer/color.h"

// Mock renderer functions to avoid linking issues
void renderer_draw_text_3d(const char *text, float x, float y, float z, float scale, const Color *color) {}
void renderer_draw_line_color(float x1, float y1, float z1, float x2, float y2, float z2, const Color *color) {}
void renderer_draw_cube_color(float x, float y, float z, float size, const Color *color) {}

int main(int argc, char *argv[])
{
    printf("Testing 3D Bar Chart Implementation (Data Management)\n");
    printf("====================================================\n\n");

    // Test 1: Initialize bar chart
    printf("Test 1: Initializing bar chart...\n");
    BarChart chart;
    CQError result = bar_chart_init(&chart, "Test Metrics");
    assert(result == CQ_SUCCESS);
    assert(chart.bar_count == 0);
    assert(chart.max_value == 0.0f);
    assert(chart.show_labels == true);
    assert(chart.show_values == true);
    printf("✓ Bar chart initialized successfully\n\n");

    // Test 2: Add bars
    printf("Test 2: Adding bars...\n");
    Color red = color_create(1.0f, 0.0f, 0.0f, 1.0f);
    Color green = color_create(0.0f, 1.0f, 0.0f, 1.0f);
    Color blue = color_create(0.0f, 0.0f, 1.0f, 1.0f);

    result = bar_chart_add_bar(&chart, 10.5f, "Complexity", &red);
    assert(result == CQ_SUCCESS);
    assert(chart.bar_count == 1);
    assert(chart.max_value == 10.5f);

    result = bar_chart_add_bar(&chart, 25.0f, "LOC", &green);
    assert(result == CQ_SUCCESS);
    assert(chart.bar_count == 2);
    assert(chart.max_value == 25.0f);

    result = bar_chart_add_bar(&chart, 15.2f, "Functions", &blue);
    assert(result == CQ_SUCCESS);
    assert(chart.bar_count == 3);
    assert(chart.max_value == 25.0f);

    printf("✓ Added 3 bars successfully\n\n");

    // Test 3: Check bar data
    printf("Test 3: Verifying bar data...\n");
    assert(chart.bars[0].value == 10.5f);
    assert(strcmp(chart.bars[0].label, "Complexity") == 0);
    assert(chart.bars[0].color.r == 1.0f);
    assert(chart.bars[0].color.g == 0.0f);
    assert(chart.bars[0].color.b == 0.0f);

    assert(chart.bars[1].value == 25.0f);
    assert(strcmp(chart.bars[1].label, "LOC") == 0);
    assert(chart.bars[1].color.g == 1.0f);

    assert(chart.bars[2].value == 15.2f);
    assert(strcmp(chart.bars[2].label, "Functions") == 0);
    assert(chart.bars[2].color.b == 1.0f);

    printf("✓ Bar data verified\n\n");

    // Test 4: Test appearance settings
    printf("Test 4: Testing appearance settings...\n");
    bar_chart_set_appearance(&chart, 1.0f, 1.0f, 1.5f, 0.5f);
    assert(chart.bar_width == 1.0f);
    assert(chart.bar_depth == 1.0f);
    assert(chart.spacing == 1.5f);
    assert(chart.base_height == 0.5f);
    printf("✓ Appearance settings applied\n\n");

    // Test 5: Test display options
    printf("Test 5: Testing display options...\n");
    bar_chart_set_display_options(&chart, false, false);
    assert(chart.show_labels == false);
    assert(chart.show_values == false);

    bar_chart_set_display_options(&chart, true, true);
    assert(chart.show_labels == true);
    assert(chart.show_values == true);
    printf("✓ Display options set\n\n");

    // Test 6: Test utility functions
    printf("Test 6: Testing utility functions...\n");
    assert(bar_chart_get_bar_count(&chart) == 3);
    assert(bar_chart_get_max_value(&chart) == 25.0f);
    printf("✓ Utility functions work correctly\n\n");

    // Test 7: Test clearing
    printf("Test 7: Testing clear function...\n");
    bar_chart_clear(&chart);
    assert(chart.bar_count == 0);
    assert(chart.max_value == 0.0f);
    printf("✓ Chart cleared successfully\n\n");

    // Test 8: Test adding bars after clear
    printf("Test 8: Adding bars after clear...\n");
    result = bar_chart_add_bar(&chart, 5.0f, "Test1", &red);
    assert(result == CQ_SUCCESS);
    result = bar_chart_add_bar(&chart, 8.0f, "Test2", &green);
    assert(result == CQ_SUCCESS);
    assert(chart.bar_count == 2);
    assert(chart.max_value == 8.0f);
    printf("✓ Bars added after clear\n\n");

    // Test 9: Test render function (without actual rendering)
    printf("Test 9: Testing render function call...\n");
    // This should not crash, even though it won't actually render
    bar_chart_render(&chart, 0.0f, 0.0f, 0.0f);
    printf("✓ Render function called successfully\n\n");

    printf("All tests passed! ✓\n");
    printf("3D Bar Chart implementation is working correctly.\n");
    printf("The bar chart can:\n");
    printf("  - Initialize with a title\n");
    printf("  - Add bars with values, labels, and colors\n");
    printf("  - Track maximum values automatically\n");
    printf("  - Configure appearance (width, depth, spacing)\n");
    printf("  - Control display options (labels, values)\n");
    printf("  - Clear and reuse the chart\n");
    printf("  - Render 3D bars (when renderer is available)\n");

    return 0;
}