#ifndef COMPLEXITY_LANDSCAPE_H
#define COMPLEXITY_LANDSCAPE_H

#include "cqanalyzer.h"
#include "visualizer/color.h"
#include "visualizer/gradient.h"

/**
 * @file complexity_landscape.h
 * @brief 3D complexity landscape visualization
 *
 * Provides functions to create and manage 3D landscape visualizations
 * where terrain height represents code complexity metrics.
 */

/**
 * @brief Visualization mode enumeration
 */
typedef enum {
    LANDSCAPE_MODE_GRID,      // Regular grid layout
    LANDSCAPE_MODE_CIRCULAR,  // Circular layout
    LANDSCAPE_MODE_HIERARCHICAL, // Directory-based hierarchical layout
    LANDSCAPE_MODE_SCATTER    // Scatter plot mode
} LandscapeMode;

/**
 * @brief Complexity landscape configuration
 */
typedef struct {
    LandscapeMode mode;           // Visualization mode
    char metric_name[256];        // Primary metric for height
    char color_metric[256];       // Metric for coloring
    float scale_factor;           // Height scaling factor
    float base_height;            // Base terrain height
    int grid_resolution;          // Grid resolution for grid mode
    float spacing;                // Spacing between elements
    bool show_labels;             // Whether to show file labels
    bool show_grid;               // Whether to show coordinate grid
    Gradient gradient;            // Color gradient for visualization
} LandscapeConfig;

/**
 * @brief Initialize complexity landscape visualization system
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError complexity_landscape_init(void);

/**
 * @brief Shutdown complexity landscape visualization system
 */
void complexity_landscape_shutdown(void);

/**
 * @brief Create complexity landscape with specified configuration
 *
 * @param config Landscape configuration
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError complexity_landscape_create(const LandscapeConfig *config);

/**
 * @brief Update landscape with new metric data
 *
 * @param metric_name Name of metric to update
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError complexity_landscape_update(const char *metric_name);

/**
 * @brief Render the complexity landscape
 */
void complexity_landscape_render(void);

/**
 * @brief Set visualization mode
 *
 * @param mode New visualization mode
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError complexity_landscape_set_mode(LandscapeMode mode);

/**
 * @brief Set primary metric for height mapping
 *
 * @param metric_name Name of metric to use for height
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError complexity_landscape_set_metric(const char *metric_name);

/**
 * @brief Set color metric for coloring the landscape
 *
 * @param metric_name Name of metric to use for coloring
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError complexity_landscape_set_color_metric(const char *metric_name);

/**
 * @brief Set height scaling factor
 *
 * @param scale Scaling factor for terrain height
 */
void complexity_landscape_set_scale(float scale);

/**
 * @brief Set color gradient
 *
 * @param gradient_name Name of predefined gradient or NULL for custom
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError complexity_landscape_set_gradient(const char *gradient_name);

/**
 * @brief Set custom color gradient
 *
 * @param colors Array of colors
 * @param num_colors Number of colors in array
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError complexity_landscape_set_custom_gradient(const Color *colors, int num_colors);

/**
 * @brief Enable/disable file labels
 *
 * @param enabled Whether to show labels
 */
void complexity_landscape_show_labels(bool enabled);

/**
 * @brief Enable/disable coordinate grid
 *
 * @param enabled Whether to show grid
 */
void complexity_landscape_show_grid(bool enabled);

/**
 * @brief Set window dimensions for tooltip calculations
 *
 * @param width Window width
 * @param height Window height
 */
void complexity_landscape_set_window_size(int width, int height);

/**
 * @brief Update mouse position for tooltip tracking
 *
 * @param screen_x Screen X coordinate
 * @param screen_y Screen Y coordinate
 */
void complexity_landscape_update_mouse_position(int screen_x, int screen_y);

/**
 * @brief Get tooltip information for a screen position
 *
 * @param screen_x Screen X coordinate
 * @param screen_y Screen Y coordinate
 * @param filepath Output buffer for file path
 * @param metric_value Output metric value
 * @return true if tooltip data found, false otherwise
 */
bool complexity_landscape_get_tooltip(int screen_x, int screen_y, char *filepath, double *metric_value);

/**
 * @brief Get legend information for current visualization
 *
 * @param min_value Output minimum metric value
 * @param max_value Output maximum metric value
 * @param gradient Output gradient information
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError complexity_landscape_get_legend(double *min_value, double *max_value, Gradient *gradient);

#endif // COMPLEXITY_LANDSCAPE_H