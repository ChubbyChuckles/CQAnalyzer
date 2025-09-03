#ifndef SCENE_H
#define SCENE_H

#include "cqanalyzer.h"
#include "visualizer/color.h"
#include "visualizer/lighting.h"
#include "visualizer/visualization_filters.h"

// Forward declaration to avoid circular dependencies
typedef struct DependencyGraph DependencyGraph;

/**
 * @file scene.h
 * @brief 3D scene management
 *
 * Provides functions to manage 3D scene objects and rendering.
 */

typedef enum
{
    VISUALIZATION_NONE,
    VISUALIZATION_SCATTER_PLOT,
    VISUALIZATION_TREE,
    VISUALIZATION_NETWORK,
    VISUALIZATION_BAR_CHART,
    VISUALIZATION_HEATMAP
} VisualizationMode;

typedef struct
{
    float position[3];
    Color color;
    float scale[3];
    Material material;
    Light light;
} SceneObject;

/**
 * @brief Structure to hold the complete visualization state
 */
typedef struct
{
    // Camera state
    struct
    {
        float position[3];
        float target[3];
        float up[3];
        float fov;
        float near_plane;
        float far_plane;
    } camera;

    // Scene state
    VisualizationMode mode;
    char x_metric[64];
    char y_metric[64];
    char z_metric[64];
    char color_metric[64];
    char surface_type[32];
    int heatmap_resolution;

    // Filters and display options
    VisualizationFilters filters;
    DisplayOptions display_options;

    // Version info for compatibility
    int version;
} VisualizationState;

/**
 * @brief Initialize 3D scene
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_init(void);

/**
 * @brief Shutdown 3D scene
 */
void scene_shutdown(void);

/**
 * @brief Add object to scene
 *
 * @param object Object to add
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_add_object(const SceneObject *object);

/**
 * @brief Remove object from scene
 *
 * @param index Index of object to remove
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_remove_object(int index);

/**
 * @brief Update scene objects
 *
 * @param delta_time Time elapsed since last update
 */
void scene_update(float delta_time);

/**
 * @brief Render scene
 */
void scene_render(void);

/**
 * @brief Clear all objects from scene
 */
void scene_clear(void);

/**
 * @brief Set the current visualization mode
 *
 * @param mode Visualization mode to set
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_set_visualization_mode(VisualizationMode mode);

/**
 * @brief Get the current visualization mode
 *
 * @return Current visualization mode
 */
VisualizationMode scene_get_visualization_mode(void);

/**
 * @brief Create scatter plot visualization
 *
 * @param x_metric X-axis metric name
 * @param y_metric Y-axis metric name
 * @param z_metric Z-axis metric name
 * @param color_metric Color metric name (optional)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_create_scatter_plot(const char *x_metric, const char *y_metric,
                                  const char *z_metric, const char *color_metric);

CQError scene_create_scatter_plot_filtered(const char *x_metric, const char *y_metric,
                                          const char *z_metric, const char *color_metric,
                                          const VisualizationFilters *filters,
                                          const DisplayOptions *options);

/**
 * @brief Create tree visualization from dependency graph
 *
 * @param dependency_graph Pointer to dependency graph
 * @param color_metric Name of metric for node coloring (optional)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_create_tree_visualization(const DependencyGraph *dependency_graph, const char *color_metric);

/**
 * @brief Create network graph visualization from dependency graph
 *
 * @param dependency_graph Pointer to dependency graph
 * @param color_metric Name of metric for node coloring (optional)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_create_network_visualization(const DependencyGraph *dependency_graph, const char *color_metric);

/**
 * @brief Create heatmap visualization from metric data
 *
 * @param metric_name Name of the metric to visualize
 * @param surface_type Type of 3D surface ("plane", "sphere", "cylinder")
 * @param resolution Grid resolution for the surface
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_create_heatmap_visualization(const char *metric_name, const char *surface_type, int resolution);

/**
 * @brief Save current visualization state to file
 *
 * @param filepath Path to save the state file
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_save_visualization_state(const char *filepath);

/**
 * @brief Load visualization state from file
 *
 * @param filepath Path to the state file to load
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_load_visualization_state(const char *filepath);

/**
 * @brief Get current visualization state
 *
 * @param state Pointer to VisualizationState structure to fill
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_get_current_state(VisualizationState *state);

/**
 * @brief Set visualization state
 *
 * @param state Pointer to VisualizationState structure to apply
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_set_state(const VisualizationState *state);

#endif // SCENE_H
