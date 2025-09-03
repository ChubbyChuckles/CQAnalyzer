#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "visualizer/tree_visualization.h"
#include "visualizer/renderer.h"
#include "visualizer/color.h"
#include "visualizer/gradient.h"
#include "data/data_store.h"
#include "data/ast_types.h"
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
    char node_type[32]; // "project", "file", "class", "function", "variable"
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

// New forward declarations for project-based tree
static void traverse_project_recursive(const Project *project, uint32_t file_index,
                                     uint32_t class_index, uint32_t func_index,
                                     float *current_x, int depth, uint32_t parent_id);
static void calculate_project_node_positions(const Project *project);
static Color get_project_node_color(const char *node_type, const char *color_metric, uint32_t node_id);

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

CQError tree_visualization_create_from_project(const Project *project, const char *color_metric)
{
    if (!project)
    {
        LOG_ERROR("Invalid project for tree visualization");
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Clear previous data
    tree_visualization_clear();

    // Calculate positions for all nodes in project hierarchy
    calculate_project_node_positions(project);

    LOG_INFO("Created project tree visualization with %d nodes, max depth %d", num_tree_nodes, max_tree_depth);
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

static void calculate_project_node_positions(const Project *project)
{
    if (!project)
    {
        return;
    }

    // Start with project root node
    float current_x = 0.0f;

    // Add project root node
    tree_nodes[num_tree_nodes].x = current_x;
    tree_nodes[num_tree_nodes].y = 0.0f; // Root level
    tree_nodes[num_tree_nodes].z = 0.0f;
    tree_nodes[num_tree_nodes].node_id = 0; // Project root
    tree_nodes[num_tree_nodes].parent_id = 0;
    tree_nodes[num_tree_nodes].depth = 0;
    tree_nodes[num_tree_nodes].type = DEPENDENCY_MODULE;
    strcpy(tree_nodes[num_tree_nodes].node_type, "project");

    // Get project name from string pool
    const char *project_name = string_pool_get(&project->string_pool, project->root_path_id);
    if (project_name)
    {
        snprintf(tree_nodes[num_tree_nodes].label, sizeof(tree_nodes[num_tree_nodes].label), "Project: %s", project_name);
    }
    else
    {
        strcpy(tree_nodes[num_tree_nodes].label, "Project");
    }

    tree_nodes[num_tree_nodes].color = get_project_node_color("project", NULL, 0);
    num_tree_nodes++;
    max_tree_depth = 0;

    // Traverse files
    for (uint32_t i = 0; i < project->files.count; i++)
    {
        traverse_project_recursive(project, i, UINT32_MAX, UINT32_MAX, &current_x, 1, 0);
        current_x += NODE_SPACING_X * 2.0f; // Space between file trees
    }
}

static void traverse_project_recursive(const Project *project, uint32_t file_index,
                                     uint32_t class_index, uint32_t func_index,
                                     float *current_x, int depth, uint32_t parent_id)
{
    if (num_tree_nodes >= MAX_TREE_NODES)
    {
        return;
    }

    uint32_t current_node_index = num_tree_nodes;

    if (file_index != UINT32_MAX && class_index == UINT32_MAX && func_index == UINT32_MAX)
    {
        // File node
        FileInfo *file = file_array_get(&project->files, file_index);
        if (!file) return;

        tree_nodes[num_tree_nodes].x = *current_x;
        tree_nodes[num_tree_nodes].y = -depth * NODE_SPACING_Y;
        tree_nodes[num_tree_nodes].z = 0.0f;
        tree_nodes[num_tree_nodes].node_id = file_index + 1; // Offset to avoid 0
        tree_nodes[num_tree_nodes].parent_id = parent_id;
        tree_nodes[num_tree_nodes].depth = depth;
        tree_nodes[num_tree_nodes].type = DEPENDENCY_MODULE;
        strcpy(tree_nodes[num_tree_nodes].node_type, "file");

        const char *filename = string_pool_get(&project->string_pool, file->filepath_id);
        if (filename)
        {
            // Extract just the filename from path
            const char *basename = strrchr(filename, '/');
            if (!basename) basename = strrchr(filename, '\\');
            if (basename) basename++;
            else basename = filename;

            snprintf(tree_nodes[num_tree_nodes].label, sizeof(tree_nodes[num_tree_nodes].label), "%s", basename);
        }
        else
        {
            sprintf(tree_nodes[num_tree_nodes].label, "File_%u", file_index);
        }

        tree_nodes[num_tree_nodes].color = get_project_node_color("file", NULL, file_index);
        num_tree_nodes++;
        max_tree_depth = depth > max_tree_depth ? depth : max_tree_depth;

        // Process classes in this file
        float child_x_start = *current_x - (file->class_count - 1) * NODE_SPACING_X / 2.0f;
        for (uint32_t i = 0; i < file->class_count; i++)
        {
            uint32_t class_idx = file->class_start + i;
            traverse_project_recursive(project, file_index, class_idx, UINT32_MAX,
                                     &child_x_start, depth + 1, tree_nodes[current_node_index].node_id);
            child_x_start += NODE_SPACING_X;
        }

        // Process global functions in this file
        child_x_start = *current_x - (file->function_count - 1) * NODE_SPACING_X / 2.0f;
        for (uint32_t i = 0; i < file->function_count; i++)
        {
            uint32_t func_idx = file->function_start + i;
            FunctionInfo *func = function_array_get(&project->functions, func_idx);
            if (func && func->class_id == 0) // Global function
            {
                traverse_project_recursive(project, file_index, UINT32_MAX, func_idx,
                                         &child_x_start, depth + 1, tree_nodes[current_node_index].node_id);
                child_x_start += NODE_SPACING_X;
            }
        }
    }
    else if (class_index != UINT32_MAX)
    {
        // Class node
        ClassInfo *cls = class_array_get(&project->classes, class_index);
        if (!cls) return;

        tree_nodes[num_tree_nodes].x = *current_x;
        tree_nodes[num_tree_nodes].y = -depth * NODE_SPACING_Y;
        tree_nodes[num_tree_nodes].z = 0.0f;
        tree_nodes[num_tree_nodes].node_id = class_index + 1000; // Offset for classes
        tree_nodes[num_tree_nodes].parent_id = parent_id;
        tree_nodes[num_tree_nodes].depth = depth;
        tree_nodes[num_tree_nodes].type = DEPENDENCY_TYPE;
        strcpy(tree_nodes[num_tree_nodes].node_type, "class");

        const char *class_name = string_pool_get(&project->string_pool, cls->name_id);
        if (class_name)
        {
            snprintf(tree_nodes[num_tree_nodes].label, sizeof(tree_nodes[num_tree_nodes].label), "Class: %s", class_name);
        }
        else
        {
            sprintf(tree_nodes[num_tree_nodes].label, "Class_%u", class_index);
        }

        tree_nodes[num_tree_nodes].color = get_project_node_color("class", NULL, class_index);
        num_tree_nodes++;
        max_tree_depth = depth > max_tree_depth ? depth : max_tree_depth;

        // Process methods in this class
        float child_x_start = *current_x - (cls->method_count - 1) * NODE_SPACING_X / 2.0f;
        for (uint32_t i = 0; i < cls->method_count; i++)
        {
            uint32_t method_idx = cls->method_indices[i];
            traverse_project_recursive(project, file_index, class_index, method_idx,
                                     &child_x_start, depth + 1, tree_nodes[current_node_index].node_id);
            child_x_start += NODE_SPACING_X;
        }
    }
    else if (func_index != UINT32_MAX)
    {
        // Function node
        FunctionInfo *func = function_array_get(&project->functions, func_index);
        if (!func) return;

        tree_nodes[num_tree_nodes].x = *current_x;
        tree_nodes[num_tree_nodes].y = -depth * NODE_SPACING_Y;
        tree_nodes[num_tree_nodes].z = 0.0f;
        tree_nodes[num_tree_nodes].node_id = func_index + 2000; // Offset for functions
        tree_nodes[num_tree_nodes].parent_id = parent_id;
        tree_nodes[num_tree_nodes].depth = depth;
        tree_nodes[num_tree_nodes].type = DEPENDENCY_FUNCTION_CALL;
        strcpy(tree_nodes[num_tree_nodes].node_type, "function");

        const char *func_name = string_pool_get(&project->string_pool, func->name_id);
        if (func_name)
        {
            snprintf(tree_nodes[num_tree_nodes].label, sizeof(tree_nodes[num_tree_nodes].label), "Func: %s", func_name);
        }
        else
        {
            sprintf(tree_nodes[num_tree_nodes].label, "Func_%u", func_index);
        }

        tree_nodes[num_tree_nodes].color = get_project_node_color("function", NULL, func_index);
        num_tree_nodes++;
        max_tree_depth = depth > max_tree_depth ? depth : max_tree_depth;
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

static Color get_project_node_color(const char *node_type, const char *color_metric, uint32_t node_id)
{
    if (!node_type)
    {
        return COLOR_GRAY;
    }

    if (strcmp(node_type, "project") == 0)
    {
        return COLOR_PURPLE;
    }
    else if (strcmp(node_type, "file") == 0)
    {
        return COLOR_BLUE;
    }
    else if (strcmp(node_type, "class") == 0)
    {
        return COLOR_RED;
    }
    else if (strcmp(node_type, "function") == 0)
    {
        return COLOR_GREEN;
    }
    else if (strcmp(node_type, "variable") == 0)
    {
        return COLOR_YELLOW;
    }
    else
    {
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
    renderer_draw_text_3d("Code Structure:", legend_x, legend_y, legend_z, 0.4f, &text_color);

    // Project node
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_PURPLE);
    renderer_draw_text_3d("Project", legend_x, legend_y - spacing, legend_z, 0.3f, &text_color);

    // File nodes
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - 2 * spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_BLUE);
    renderer_draw_text_3d("Files", legend_x, legend_y - 2 * spacing, legend_z, 0.3f, &text_color);

    // Class nodes
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - 3 * spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_RED);
    renderer_draw_text_3d("Classes", legend_x, legend_y - 3 * spacing, legend_z, 0.3f, &text_color);

    // Function nodes
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - 4 * spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_GREEN);
    renderer_draw_text_3d("Functions", legend_x, legend_y - 4 * spacing, legend_z, 0.3f, &text_color);
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