#ifndef BAR_CHART_H
#define BAR_CHART_H

#include "cqanalyzer.h"
#include "visualizer/color.h"

/**
 * @file bar_chart.h
 * @brief 3D bar chart visualization for comparative metric analysis
 *
 * Provides functions to create and render 3D bar charts
 * for comparing different code metrics across categories.
 */

#define MAX_BAR_CHART_BARS 100
#define MAX_BAR_LABEL_LENGTH 64

/**
 * @brief Bar chart data structure
 */
typedef struct
{
    float value;                    // Bar value/height
    char label[MAX_BAR_LABEL_LENGTH]; // Bar label
    Color color;                    // Bar color
} BarChartBar;

/**
 * @brief Bar chart configuration
 */
typedef struct
{
    char title[128];                // Chart title
    float bar_width;                // Width of each bar
    float bar_depth;                // Depth of each bar
    float bar_spacing;              // Spacing between bars
    float max_height;               // Maximum bar height (for scaling)
    bool show_labels;               // Whether to show bar labels
    bool show_values;               // Whether to show bar values
    bool show_grid;                 // Whether to show grid lines
    Color background_color;         // Chart background color
    Color grid_color;               // Grid line color
} BarChartConfig;

/**
 * @brief Initialize bar chart system
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError bar_chart_init(void);

/**
 * @brief Shutdown bar chart system
 */
void bar_chart_shutdown(void);

/**
 * @brief Create a new bar chart
 *
 * @param title Chart title
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError bar_chart_create(const char *title);

/**
 * @brief Add a bar to the chart
 *
 * @param value Bar value (height)
 * @param label Bar label
 * @param color Bar color (can be NULL for default)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError bar_chart_add_bar(float value, const char *label, const Color *color);

/**
 * @brief Set bar chart configuration
 *
 * @param config Configuration structure
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError bar_chart_set_config(const BarChartConfig *config);

/**
 * @brief Get current bar chart configuration
 *
 * @param config Output configuration structure
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError bar_chart_get_config(BarChartConfig *config);

/**
 * @brief Render the 3D bar chart
 */
void bar_chart_render(void);

/**
 * @brief Clear all bar chart data
 */
void bar_chart_clear(void);

/**
 * @brief Get the number of bars in the chart
 *
 * @return Number of bars
 */
int bar_chart_get_bar_count(void);

/**
 * @brief Get data for a specific bar
 *
 * @param index Bar index
 * @param value Pointer to store value (can be NULL)
 * @param label Pointer to store label string (can be NULL)
 * @param color Pointer to store color (can be NULL)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError bar_chart_get_bar(int index, float *value, char *label, Color *color);

/**
 * @brief Set the maximum height for scaling
 *
 * @param max_height Maximum height value
 */
void bar_chart_set_max_height(float max_height);

/**
 * @brief Auto-scale bars based on current data
 */
void bar_chart_auto_scale(void);

/**
 * @brief Set default bar color
 *
 * @param color Default color for new bars
 */
void bar_chart_set_default_color(const Color *color);

#endif // BAR_CHART_H