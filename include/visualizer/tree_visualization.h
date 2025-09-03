#ifndef TREE_VISUALIZATION_H
#define TREE_VISUALIZATION_H

#include "cqanalyzer.h"
#include "visualizer/color.h"
#include "data/dependency_graph.h"
#include "data/ast_types.h"

/**
 * @file tree_visualization.h
 * @brief Hierarchical tree visualization for code structure
 *
 * Provides functions to create and render 3D tree visualizations
 * showing hierarchical code organization and relationships.
 */

/**
 * @brief Create a 3D tree visualization from dependency graph
 *
 * @param dependency_graph Pointer to dependency graph containing hierarchy
 * @param color_metric Name of metric for node coloring (optional, can be NULL)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError tree_visualization_create(const DependencyGraph *dependency_graph, const char *color_metric);

/**
 * @brief Create a 3D tree visualization from project code structure
 *
 * @param project Pointer to project containing code structure
 * @param color_metric Name of metric for node coloring (optional, can be NULL)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError tree_visualization_create_from_project(const Project *project, const char *color_metric);

/**
 * @brief Render the tree visualization
 */
void tree_visualization_render(void);

/**
 * @brief Clear all tree visualization data
 */
void tree_visualization_clear(void);

/**
 * @brief Get the number of nodes in the tree visualization
 *
 * @return Number of nodes
 */
int tree_visualization_get_node_count(void);

/**
 * @brief Get data for a specific tree node
 *
 * @param index Node index
 * @param x Pointer to store X coordinate (can be NULL)
 * @param y Pointer to store Y coordinate (can be NULL)
 * @param z Pointer to store Z coordinate (can be NULL)
 * @param color Pointer to store color (can be NULL)
 * @param label Pointer to store label string (can be NULL)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError tree_visualization_get_node(int index, float *x, float *y, float *z,
                                   Color *color, char *label);

#endif // TREE_VISUALIZATION_H