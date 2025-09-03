#ifndef BUBBLE_CHART_H
#define BUBBLE_CHART_H

#include "cqanalyzer.h"
#include "visualizer/color.h"

/**
 * @file bubble_chart.h
 * @brief Bubble chart visualization for multi-dimensional metrics
 *
 * Provides functions to create and render bubble charts
 * showing correlations between different code metrics with
 * bubble sizes representing additional dimensions.
 */

/**
 * @brief Create a bubble chart from metric data
 *
 * @param x_metric Name of metric for X axis
 * @param y_metric Name of metric for Y axis
 * @param z_metric Name of metric for Z axis
 * @param size_metric Name of metric for bubble size
 * @param color_metric Name of metric for bubble coloring (optional, can be NULL)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError bubble_chart_create(const char *x_metric, const char *y_metric,
                           const char *z_metric, const char *size_metric,
                           const char *color_metric);

/**
 * @brief Render the bubble chart
 */
void bubble_chart_render(void);

/**
 * @brief Draw coordinate axes and grid
 */
void bubble_chart_draw_axes(void);

/**
 * @brief Clear all bubble chart data
 */
void bubble_chart_clear(void);

/**
 * @brief Get the number of bubbles in the chart
 *
 * @return Number of bubbles
 */
int bubble_chart_get_bubble_count(void);

/**
 * @brief Get data for a specific bubble
 *
 * @param index Bubble index
 * @param x Pointer to store X coordinate (can be NULL)
 * @param y Pointer to store Y coordinate (can be NULL)
 * @param z Pointer to store Z coordinate (can be NULL)
 * @param size Pointer to store bubble size (can be NULL)
 * @param color Pointer to store color (can be NULL)
 * @param label Pointer to store label string (can be NULL)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError bubble_chart_get_bubble(int index, float *x, float *y, float *z,
                               float *size, Color *color, char *label);

#endif // BUBBLE_CHART_H