#ifndef HEATMAP_H
#define HEATMAP_H

#include "cqanalyzer.h"
#include "visualizer/color.h"

/**
 * @file heatmap.h
 * @brief 3D heatmap visualization
 *
 * Provides functions to create and render heatmap visualizations
 * on 3D surfaces using metric data.
 */

/**
 * @brief Initialize heatmap visualization system
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError heatmap_init(void);

/**
 * @brief Shutdown heatmap visualization system
 */
void heatmap_shutdown(void);

/**
 * @brief Create heatmap visualization from metric data
 *
 * @param metric_name Name of the metric to visualize
 * @param surface_type Type of 3D surface (plane, sphere, cylinder)
 * @param resolution Grid resolution for the surface
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError heatmap_create(const char *metric_name, const char *surface_type, int resolution);

/**
 * @brief Update heatmap data
 *
 * @param metric_name Name of the metric to update
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError heatmap_update(const char *metric_name);

/**
 * @brief Render heatmap visualization
 */
void heatmap_render(void);

/**
 * @brief Set heatmap color gradient
 *
 * @param gradient_name Name of predefined gradient ("viridis", "plasma", "inferno", "magma")
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError heatmap_set_gradient(const char *gradient_name);

/**
 * @brief Set custom color gradient
 *
 * @param colors Array of colors for gradient
 * @param num_colors Number of colors in gradient
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError heatmap_set_custom_gradient(const Color *colors, int num_colors);

/**
 * @brief Set heatmap opacity
 *
 * @param opacity Opacity value (0.0 to 1.0)
 */
void heatmap_set_opacity(float opacity);

/**
 * @brief Set heatmap scale
 *
 * @param scale Scale factor for height mapping
 */
void heatmap_set_scale(float scale);

#endif // HEATMAP_H