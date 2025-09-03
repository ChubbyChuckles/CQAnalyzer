#include "visualizer/bar_chart.h"
#include "visualizer/renderer.h"
#include "visualizer/color.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

/**
 * @file bar_chart.c
 * @brief 3D bar chart implementation
 */

static BarChartBar bars[MAX_BAR_CHART_BARS];
static int bar_count = 0;
static BarChartConfig config;
static Color default_color;
static bool initialized = false;

/**
 * @brief Initialize bar chart system
 */
CQError bar_chart_init(void)
{
    if (initialized)
        return CQ_SUCCESS;

    // Initialize default configuration
    memset(&config, 0, sizeof(BarChartConfig));
    strcpy(config.title, "Bar Chart");
    config.bar_width = 0.8f;
    config.bar_depth = 0.8f;
    config.bar_spacing = 1.0f;
    config.max_height = 10.0f;
    config.show_labels = true;
    config.show_values = true;
    config.show_grid = true;
    config.background_color = color_create(0.1f, 0.1f, 0.1f, 1.0f);
    config.grid_color = color_create(0.3f, 0.3f, 0.3f, 1.0f);

    // Set default bar color
    default_color = color_create(0.2f, 0.6f, 1.0f, 1.0f);

    bar_count = 0;
    initialized = true;

    return CQ_SUCCESS;
}

/**
 * @brief Shutdown bar chart system
 */
void bar_chart_shutdown(void)
{
    bar_chart_clear();
    initialized = false;
}

/**
 * @brief Create a new bar chart
 */
CQError bar_chart_create(const char *title)
{
    if (!initialized)
        return CQ_ERROR_UNKNOWN;

    bar_chart_clear();

    if (title)
        strncpy(config.title, title, sizeof(config.title) - 1);

    return CQ_SUCCESS;
}

/**
 * @brief Add a bar to the chart
 */
CQError bar_chart_add_bar(float value, const char *label, const Color *color)
{
    if (!initialized)
        return CQ_ERROR_UNKNOWN;

    if (bar_count >= MAX_BAR_CHART_BARS)
        return CQ_ERROR_MEMORY_ALLOCATION;

    BarChartBar *bar = &bars[bar_count];
    bar->value = value;

    if (label)
        strncpy(bar->label, label, sizeof(bar->label) - 1);
    else
        sprintf(bar->label, "Bar %d", bar_count + 1);

    if (color)
        bar->color = *color;
    else
        bar->color = default_color;

    bar_count++;
    return CQ_SUCCESS;
}

/**
 * @brief Set bar chart configuration
 */
CQError bar_chart_set_config(const BarChartConfig *new_config)
{
    if (!initialized)
        return CQ_ERROR_UNKNOWN;

    if (!new_config)
        return CQ_ERROR_INVALID_ARGUMENT;

    config = *new_config;
    return CQ_SUCCESS;
}

/**
 * @brief Get current bar chart configuration
 */
CQError bar_chart_get_config(BarChartConfig *out_config)
{
    if (!initialized)
        return CQ_ERROR_UNKNOWN;

    if (!out_config)
        return CQ_ERROR_INVALID_ARGUMENT;

    *out_config = config;
    return CQ_SUCCESS;
}

/**
 * @brief Render the 3D bar chart
 */
void bar_chart_render(void)
{
    if (!initialized || bar_count == 0)
        return;

    // Calculate chart dimensions
    float total_width = bar_count * (config.bar_width + config.bar_spacing) - config.bar_spacing;
    float start_x = -total_width / 2.0f;

    // Find maximum value for scaling
    float max_value = 0.0f;
    for (int i = 0; i < bar_count; i++)
    {
        if (bars[i].value > max_value)
            max_value = bars[i].value;
    }

    if (max_value == 0.0f)
        max_value = 1.0f;

    float scale_factor = config.max_height / max_value;

    // Draw grid lines if enabled
    if (config.show_grid)
    {
        // Draw ground plane as a large flat cube
        renderer_draw_cube_color(0.0f, -0.05f, 0.0f, total_width + 2.0f, &config.grid_color);

        // Draw grid lines
        for (int i = 0; i <= 10; i++)
        {
            float y = (float)i * config.max_height / 10.0f;
            float line_start = start_x - 1.0f;
            float line_end = start_x + total_width + 1.0f;

            renderer_draw_line_color(line_start, y, -1.0f, line_end, y, -1.0f, &config.grid_color);
            renderer_draw_line_color(line_start, y, 1.0f, line_end, y, 1.0f, &config.grid_color);
        }
    }

    // Draw bars
    for (int i = 0; i < bar_count; i++)
    {
        BarChartBar *bar = &bars[i];
        float x = start_x + i * (config.bar_width + config.bar_spacing) + config.bar_width / 2.0f;
        float height = bar->value * scale_factor;

        // Draw the bar as a 3D rectangular prism using lines
        float half_width = config.bar_width / 2.0f;
        float half_depth = config.bar_depth / 2.0f;

        // Front face
        renderer_draw_line_color(x - half_width, 0.0f, half_depth, x + half_width, 0.0f, half_depth, &bar->color);
        renderer_draw_line_color(x + half_width, 0.0f, half_depth, x + half_width, height, half_depth, &bar->color);
        renderer_draw_line_color(x + half_width, height, half_depth, x - half_width, height, half_depth, &bar->color);
        renderer_draw_line_color(x - half_width, height, half_depth, x - half_width, 0.0f, half_depth, &bar->color);

        // Back face
        renderer_draw_line_color(x - half_width, 0.0f, -half_depth, x + half_width, 0.0f, -half_depth, &bar->color);
        renderer_draw_line_color(x + half_width, 0.0f, -half_depth, x + half_width, height, -half_depth, &bar->color);
        renderer_draw_line_color(x + half_width, height, -half_depth, x - half_width, height, -half_depth, &bar->color);
        renderer_draw_line_color(x - half_width, height, -half_depth, x - half_width, 0.0f, -half_depth, &bar->color);

        // Connecting edges
        renderer_draw_line_color(x - half_width, 0.0f, half_depth, x - half_width, 0.0f, -half_depth, &bar->color);
        renderer_draw_line_color(x + half_width, 0.0f, half_depth, x + half_width, 0.0f, -half_depth, &bar->color);
        renderer_draw_line_color(x + half_width, height, half_depth, x + half_width, height, -half_depth, &bar->color);
        renderer_draw_line_color(x - half_width, height, half_depth, x - half_width, height, -half_depth, &bar->color);

        // Draw labels if enabled
        if (config.show_labels && strlen(bar->label) > 0)
        {
            renderer_draw_text_3d(bar->label, x, height + 0.5f, half_depth + 0.2f, 0.5f, &bar->color);
        }

        // Draw values if enabled
        if (config.show_values)
        {
            char value_str[32];
            sprintf(value_str, "%.1f", bar->value);
            renderer_draw_text_3d(value_str, x, height + 1.0f, half_depth + 0.2f, 0.4f, &bar->color);
        }
    }

    // Draw chart title
    if (strlen(config.title) > 0)
    {
        renderer_draw_text(config.title, -total_width / 2.0f, config.max_height + 2.0f, 0.8f, &COLOR_WHITE);
    }

    // Draw axes labels
    renderer_draw_text("Categories", 0.0f, -2.0f, 0.6f, &COLOR_WHITE);
    renderer_draw_text_3d("Values", -total_width / 2.0f - 2.0f, config.max_height / 2.0f, 0.0f, 0.6f, &COLOR_WHITE);
}

/**
 * @brief Clear all bar chart data
 */
void bar_chart_clear(void)
{
    bar_count = 0;
    memset(bars, 0, sizeof(bars));
}

/**
 * @brief Get the number of bars in the chart
 */
int bar_chart_get_bar_count(void)
{
    return bar_count;
}

/**
 * @brief Get data for a specific bar
 */
CQError bar_chart_get_bar(int index, float *value, char *label, Color *color)
{
    if (!initialized)
        return CQ_ERROR_UNKNOWN;

    if (index < 0 || index >= bar_count)
        return CQ_ERROR_INVALID_ARGUMENT;

    BarChartBar *bar = &bars[index];

    if (value)
        *value = bar->value;

    if (label)
        strcpy(label, bar->label);

    if (color)
        *color = bar->color;

    return CQ_SUCCESS;
}

/**
 * @brief Set the maximum height for scaling
 */
void bar_chart_set_max_height(float max_height)
{
    if (max_height > 0.0f)
        config.max_height = max_height;
}

/**
 * @brief Auto-scale bars based on current data
 */
void bar_chart_auto_scale(void)
{
    if (bar_count == 0)
        return;

    float max_value = 0.0f;
    for (int i = 0; i < bar_count; i++)
    {
        if (bars[i].value > max_value)
            max_value = bars[i].value;
    }

    if (max_value > 0.0f)
        config.max_height = max_value * 1.1f; // Add 10% padding
}

/**
 * @brief Set default bar color
 */
void bar_chart_set_default_color(const Color *color)
{
    if (color)
        default_color = *color;
}