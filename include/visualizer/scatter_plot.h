#ifndef SCATTER_PLOT_H
#define SCATTER_PLOT_H

#include "cqanalyzer.h"
#include "visualizer/color.h"

/**
 * @file scatter_plot.h
 * @brief 3D scatter plot visualization for metric correlations
 *
 * Provides functions to create and render 3D scatter plots
 * showing correlations between different code metrics.
 */

/**
 * @brief Create a 3D scatter plot from metric data
 *
 * @param x_metric Name of metric for X axis
 * @param y_metric Name of metric for Y axis
 * @param z_metric Name of metric for Z axis
 * @param color_metric Name of metric for point coloring (optional, can be NULL)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scatter_plot_create(const char *x_metric, const char *y_metric,
                           const char *z_metric, const char *color_metric);

/**
 * @brief Render the scatter plot
 */
void scatter_plot_render(void);

/**
 * @brief Draw coordinate axes and grid
 */
void scatter_plot_draw_axes(void);

/**
 * @brief Clear all scatter plot data
 */
void scatter_plot_clear(void);

/**
 * @brief Get the number of points in the scatter plot
 *
 * @return Number of points
 */
int scatter_plot_get_point_count(void);

/**
 * @brief Get data for a specific scatter plot point
 *
 * @param index Point index
 * @param x Pointer to store X coordinate (can be NULL)
 * @param y Pointer to store Y coordinate (can be NULL)
 * @param z Pointer to store Z coordinate (can be NULL)
 * @param color Pointer to store color (can be NULL)
 * @param label Pointer to store label string (can be NULL)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scatter_plot_get_point(int index, float *x, float *y, float *z,
                              Color *color, char *label);

#endif // SCATTER_PLOT_H