#ifndef NETWORK_GRAPH_H
#define NETWORK_GRAPH_H

#include "cqanalyzer.h"
#include "visualizer/color.h"
#include "data/dependency_graph.h"

/**
 * @file network_graph.h
 * @brief Network graph visualization for dependency relationships
 *
 * Provides functions to create and render 3D network graphs showing
 * code dependencies, function calls, and relationships.
 */

/**
 * @brief Create network graph visualization from dependency graph
 *
 * @param dependency_graph Pointer to dependency graph
 * @param color_metric Name of metric for node coloring (optional)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError network_graph_create(const DependencyGraph *dependency_graph, const char *color_metric);

/**
 * @brief Render the network graph
 */
void network_graph_render(void);

/**
 * @brief Clear the network graph data
 */
void network_graph_clear(void);

/**
 * @brief Get the number of nodes in the network graph
 *
 * @return Number of nodes
 */
int network_graph_get_node_count(void);

/**
 * @brief Get the number of edges in the network graph
 *
 * @return Number of edges
 */
int network_graph_get_edge_count(void);

/**
 * @brief Get node information by index
 *
 * @param index Node index
 * @param x Pointer to store X position (optional)
 * @param y Pointer to store Y position (optional)
 * @param z Pointer to store Z position (optional)
 * @param color Pointer to store node color (optional)
 * @param label Pointer to store node label (optional)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError network_graph_get_node(int index, float *x, float *y, float *z,
                              Color *color, char *label);

/**
 * @brief Get edge information by index
 *
 * @param index Edge index
 * @param from_node_index Pointer to store source node index
 * @param to_node_index Pointer to store target node index
 * @param color Pointer to store edge color (optional)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError network_graph_get_edge(int index, int *from_node_index, int *to_node_index,
                              Color *color);

/**
 * @brief Draw legend for network graph
 */
void network_graph_draw_legend(void);

#endif // NETWORK_GRAPH_H