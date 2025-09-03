#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "visualizer/tree_visualization.h"
#include "visualizer/renderer.h"
#include "visualizer/color.h"
#include "visualizer/gradient.h"
#include "data/data_store.h"
#include "analyzer/metric_calculator.h"
#include "utils/logger.h"

#define MAX_TREE_NODES 1000
#define NODE_SPACING_X 2.0f
#define NODE_SPACING_Y 1.5f
#define NODE_SPACING_Z 1.0f
#define NODE_SIZE 0.1f
#define CONNECTION_THICKNESS 0.02f

typedef struct
{
    float x, y, z;
    Color color;
    char label[256];
    uint32_t node_id;
    uint32_t parent_id;
    int depth;
    DependencyType type;
} TreeNode;

static TreeNode tree_nodes[MAX_TREE_NODES];
static int num_tree_nodes = 0;
static int max_tree_depth = 0;

// Forward declarations for recursive functions
static void traverse_tree_recursive(const DependencyTree *tree, TreeNode *node,
                                   float *current_x, int depth, uint32_t parent_id);
static void calculate_node_positions(const DependencyTree *tree);
static void render_tree_recursive(int node_index);
static Color get_node_color(DependencyType type, const char *color_metric, uint32_t node_id);

CQError tree_visualization_create(const DependencyGraph *dependency_graph, const char *color_metric)
{
    if (!dependency_graph)
    {
        LOG_ERROR("Invalid dependency graph for tree visualization");
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Clear previous data
    tree_visualization_clear();

    // Calculate positions for all nodes
    calculate_node_positions(&dependency_graph->hierarchy);

    LOG_INFO("Created tree visualization with %d nodes, max depth %d", num_tree_nodes, max_tree_depth);
    return CQ_SUCCESS;
}

static void calculate_node_positions(const DependencyTree *tree)
{
    if (!tree || !tree->root)
    {
        return;
    }

    // Start with root node at origin
    float current_x = 0.0f;
    traverse_tree_recursive(tree, tree->root, &current_x, 0, 0);
}

static void traverse_tree_recursive(const DependencyTree *tree, TreeNode *node,
                                   float *current_x, int depth, uint32_t parent_id)
{
    if (!node || num_tree_nodes >= MAX_TREE_NODES)
    {
        return;
    }

    // Calculate node position
    tree_nodes[num_tree_nodes].x = *current_x;
    tree_nodes[num_tree_nodes].y = -depth * NODE_SPACING_Y; // Negative Y goes down
    tree_nodes[num_tree_nodes].z = 0.0f;
    tree_nodes[num_tree_nodes].node_id = node->id;
    tree_nodes[num_tree_nodes].parent_id = parent_id;
    tree_nodes[num_tree_nodes].depth = depth;
    tree_nodes[num_tree_nodes].type = node->type;

    // Get node name from string pool (simplified - using node ID as label for now)
    sprintf(tree_nodes[num_tree_nodes].label, "Node_%u", node->id);

    // Set node color
    tree_nodes[num_tree_nodes].color = get_node_color(node->type, NULL, node->id);

    num_tree_nodes++;
    max_tree_depth = depth > max_tree_depth ? depth : max_tree_depth;

    // Process children
    if (node->first_child)
    {
        float child_x_start = *current_x - (node->child_count - 1) * NODE_SPACING_X / 2.0f;
        TreeNode *child = node->first_child;

        while (child && num_tree_nodes < MAX_TREE_NODES)
        {
            traverse_tree_recursive(tree, child, &child_x_start, depth + 1, node->id);
            child_x_start += NODE_SPACING_X;
            child = child->next_sibling;
        }
    }
}

static Color get_node_color(DependencyType type, const char *color_metric, uint32_t node_id)
{
    // Color based on node type
    switch (type)
    {
    case DEPENDENCY_FUNCTION_CALL:
        return COLOR_BLUE;
    case DEPENDENCY_INHERITANCE:
        return COLOR_RED;
    case DEPENDENCY_COMPOSITION:
        return COLOR_GREEN;
    case DEPENDENCY_TYPE:
        return COLOR_YELLOW;
    case DEPENDENCY_MODULE:
        return COLOR_PURPLE;
    default:
        return COLOR_GRAY;
    }
}

void tree_visualization_render(void)
{
    if (num_tree_nodes == 0)
    {
        return;
    }

    // Draw connections first (behind nodes)
    for (int i = 0; i < num_tree_nodes; i++)
    {
        if (tree_nodes[i].parent_id != 0)
        {
            // Find parent node
            for (int j = 0; j < num_tree_nodes; j++)
            {
                if (tree_nodes[j].node_id == tree_nodes[i].parent_id)
                {
                    Color connection_color = {0.5f, 0.5f, 0.5f, 0.8f};
                    renderer_draw_line_color(
                        tree_nodes[j].x, tree_nodes[j].y, tree_nodes[j].z,
                        tree_nodes[i].x, tree_nodes[i].y, tree_nodes[i].z,
                        &connection_color
                    );
                    break;
                }
            }
        }
    }

    // Draw nodes
    for (int i = 0; i < num_tree_nodes; i++)
    {
        renderer_draw_sphere_color(
            tree_nodes[i].x,
            tree_nodes[i].y,
            tree_nodes[i].z,
            NODE_SIZE,
            &tree_nodes[i].color
        );

        // Draw labels for some nodes to avoid clutter
        if (i % 5 == 0 || tree_nodes[i].depth < 2) // Label every 5th node or first two levels
        {
            Color label_color = {1.0f, 1.0f, 1.0f, 1.0f};
            renderer_draw_text_3d(
                tree_nodes[i].label,
                tree_nodes[i].x + NODE_SIZE * 1.5f,
                tree_nodes[i].y + NODE_SIZE * 1.5f,
                tree_nodes[i].z,
                0.3f,
                &label_color
            );
        }
    }

    // Draw a simple legend
    tree_visualization_draw_legend();
}

void tree_visualization_draw_legend(void)
{
    float legend_x = -8.0f;
    float legend_y = 4.0f;
    float legend_z = 0.0f;
    float spacing = 0.5f;

    Color text_color = {1.0f, 1.0f, 1.0f, 1.0f};

    // Title
    renderer_draw_text_3d("Node Types:", legend_x, legend_y, legend_z, 0.4f, &text_color);

    // Function nodes
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_BLUE);
    renderer_draw_text_3d("Functions", legend_x, legend_y - spacing, legend_z, 0.3f, &text_color);

    // Class nodes
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - 2 * spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_RED);
    renderer_draw_text_3d("Classes", legend_x, legend_y - 2 * spacing, legend_z, 0.3f, &text_color);

    // Module nodes
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - 3 * spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_PURPLE);
    renderer_draw_text_3d("Modules", legend_x, legend_y - 3 * spacing, legend_z, 0.3f, &text_color);
}

void tree_visualization_clear(void)
{
    num_tree_nodes = 0;
    max_tree_depth = 0;
    LOG_DEBUG("Tree visualization cleared");
}

int tree_visualization_get_node_count(void)
{
    return num_tree_nodes;
}

CQError tree_visualization_get_node(int index, float *x, float *y, float *z,
                                   Color *color, char *label)
{
    if (index < 0 || index >= num_tree_nodes)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (x) *x = tree_nodes[index].x;
    if (y) *y = tree_nodes[index].y;
    if (z) *z = tree_nodes[index].z;
    if (color) *color = tree_nodes[index].color;
    if (label) strcpy(label, tree_nodes[index].label);

    return CQ_SUCCESS;
}