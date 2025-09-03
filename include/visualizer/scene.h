#ifndef SCENE_H
#define SCENE_H

#include "cqanalyzer.h"
#include "visualizer/color.h"
#include "visualizer/lighting.h"

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
    VISUALIZATION_BAR_CHART
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

/**
 * @brief Create tree visualization from dependency graph
 *
 * @param dependency_graph Pointer to dependency graph
 * @param color_metric Name of metric for node coloring (optional)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_create_tree_visualization(const DependencyGraph *dependency_graph, const char *color_metric);

#endif // SCENE_H
