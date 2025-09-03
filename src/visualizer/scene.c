#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "visualizer/scene.h"
#include "visualizer/camera.h"
#include "visualizer/network_graph.h"
#include "visualizer/scatter_plot.h"
#include "visualizer/tree_visualization.h"
#include "visualizer/heatmap.h"
#include "visualizer/picking.h"
#include "visualizer/visualization_filters.h"
#include "ui/input_handler.h"
#include "utils/logger.h"

#define MAX_SCENE_OBJECTS 1000

static SceneObject *scene_objects = NULL;
static int num_objects = 0;
static int max_objects = 0;
static GLFWwindow *window = NULL;
static VisualizationMode current_mode = VISUALIZATION_NONE;

// Current visualization parameters
static char current_x_metric[64] = "";
static char current_y_metric[64] = "";
static char current_z_metric[64] = "";
static char current_color_metric[64] = "";
static char current_surface_type[32] = "plane";
static int current_heatmap_resolution = 32;

// Current filters and display options
static VisualizationFilters current_filters;
static DisplayOptions current_display_options;

CQError scene_init(void)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        LOG_ERROR("Failed to initialize GLFW");
        return CQ_ERROR_UNKNOWN;
    }

    // Set OpenGL version hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create window
    window = glfwCreateWindow(800, 600, "CQAnalyzer Visualizer", NULL, NULL);
    if (!window)
    {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return CQ_ERROR_UNKNOWN;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        LOG_ERROR("Failed to initialize GLEW");
        glfwDestroyWindow(window);
        glfwTerminate();
        return CQ_ERROR_UNKNOWN;
    }

    // Set viewport
    glViewport(0, 0, 800, 600);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Allocate scene objects
    scene_objects = (SceneObject *)malloc(MAX_SCENE_OBJECTS * sizeof(SceneObject));
    if (!scene_objects)
    {
        LOG_ERROR("Failed to allocate memory for scene objects");
        glfwDestroyWindow(window);
        glfwTerminate();
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    max_objects = MAX_SCENE_OBJECTS;
    num_objects = 0;

    // Initialize filters and display options
    visualization_filters_init(&current_filters);
    display_options_init(&current_display_options);

    LOG_INFO("OpenGL context initialized with GLFW window management");
    LOG_INFO("3D scene initialized with capacity for %d objects", max_objects);
    return CQ_SUCCESS;
}

void scene_shutdown(void)
{
    // Shutdown picking system
    picking_shutdown();

    if (scene_objects)
    {
        free(scene_objects);
        scene_objects = NULL;
    }

    if (window)
    {
        glfwDestroyWindow(window);
        window = NULL;
    }

    glfwTerminate();

    num_objects = 0;
    max_objects = 0;

    LOG_INFO("OpenGL context and GLFW terminated");
    LOG_INFO("3D scene shutdown");
}

CQError scene_add_object(const SceneObject *object)
{
    if (!object)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (num_objects >= max_objects)
    {
        LOG_ERROR("Scene is full (%d objects)", max_objects);
        return CQ_ERROR_UNKNOWN;
    }

    scene_objects[num_objects] = *object;
    num_objects++;

    LOG_DEBUG("Added object to scene (total: %d)", num_objects);
    return CQ_SUCCESS;
}

CQError scene_remove_object(int index)
{
    if (index < 0 || index >= num_objects)
    {
        LOG_ERROR("Invalid object index: %d", index);
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Shift remaining objects
    for (int i = index; i < num_objects - 1; i++)
    {
        scene_objects[i] = scene_objects[i + 1];
    }

    num_objects--;
    LOG_DEBUG("Removed object from scene (remaining: %d)", num_objects);

    return CQ_SUCCESS;
}

void scene_update(float delta_time)
{
    // TODO: Update object positions, animations, etc.
    LOG_WARNING("Scene update not yet implemented");
}

void scene_render(void)
{
    // Handle keyboard shortcuts for display toggles based on current mode
    scene_handle_display_shortcuts();

    // Render based on current visualization mode
    switch (current_mode)
    {
    case VISUALIZATION_SCATTER_PLOT:
        scatter_plot_render();
        break;

    case VISUALIZATION_TREE:
        tree_visualization_render();
        break;

    case VISUALIZATION_NETWORK:
        network_graph_render();
        break;

    case VISUALIZATION_HEATMAP:
        heatmap_render();
        break;

    case VISUALIZATION_NONE:
    default:
        // Render scene objects if any
        if (num_objects == 0)
        {
            return;
        }
        // TODO: Render all scene objects
        LOG_WARNING("Scene object rendering not yet implemented");
        break;
    }
}

void scene_clear(void)
{
    num_objects = 0;
    LOG_DEBUG("Scene cleared");
}

CQError scene_set_visualization_mode(VisualizationMode mode)
{
    current_mode = mode;
    LOG_INFO("Visualization mode set to: %d", mode);
    return CQ_SUCCESS;
}

VisualizationMode scene_get_visualization_mode(void)
{
    return current_mode;
}

CQError scene_create_scatter_plot(const char *x_metric, const char *y_metric,
                                 const char *z_metric, const char *color_metric)
{
    CQError result = scatter_plot_create(x_metric, y_metric, z_metric, color_metric);
    if (result == CQ_SUCCESS)
    {
        current_mode = VISUALIZATION_SCATTER_PLOT;

        // Store parameters for save/load functionality
        if (x_metric) strncpy(current_x_metric, x_metric, sizeof(current_x_metric) - 1);
        if (y_metric) strncpy(current_y_metric, y_metric, sizeof(current_y_metric) - 1);
        if (z_metric) strncpy(current_z_metric, z_metric, sizeof(current_z_metric) - 1);
        if (color_metric) strncpy(current_color_metric, color_metric, sizeof(current_color_metric) - 1);

        LOG_INFO("Scatter plot visualization created successfully");
    }
    else
    {
        LOG_ERROR("Failed to create scatter plot visualization");
    }
    return result;
}

CQError scene_create_scatter_plot_filtered(const char *x_metric, const char *y_metric,
                                          const char *z_metric, const char *color_metric,
                                          const VisualizationFilters *filters,
                                          const DisplayOptions *options)
{
    CQError result = scatter_plot_create_filtered(x_metric, y_metric, z_metric, color_metric,
                                                 filters, options);
    if (result == CQ_SUCCESS)
    {
        current_mode = VISUALIZATION_SCATTER_PLOT;

        // Store parameters for save/load functionality
        if (x_metric) strncpy(current_x_metric, x_metric, sizeof(current_x_metric) - 1);
        if (y_metric) strncpy(current_y_metric, y_metric, sizeof(current_y_metric) - 1);
        if (z_metric) strncpy(current_z_metric, z_metric, sizeof(current_z_metric) - 1);
        if (color_metric) strncpy(current_color_metric, color_metric, sizeof(current_color_metric) - 1);

        // Store filters and display options
        if (filters) current_filters = *filters;
        if (options) current_display_options = *options;

        LOG_INFO("Filtered scatter plot visualization created successfully");
    }
    else
    {
        LOG_ERROR("Failed to create filtered scatter plot visualization");
    }
    return result;
}

CQError scene_create_tree_visualization(const DependencyGraph *dependency_graph, const char *color_metric)
{
    CQError result = tree_visualization_create(dependency_graph, color_metric);
    if (result == CQ_SUCCESS)
    {
        current_mode = VISUALIZATION_TREE;
        LOG_INFO("Tree visualization created successfully");
    }
    else
    {
        LOG_ERROR("Failed to create tree visualization");
    }
    return result;
}

CQError scene_create_network_visualization(const DependencyGraph *dependency_graph, const char *color_metric)
{
    CQError result = network_graph_create(dependency_graph, color_metric);
    if (result == CQ_SUCCESS)
    {
        current_mode = VISUALIZATION_NETWORK;
        LOG_INFO("Network visualization created successfully");
    }
    else
    {
        LOG_ERROR("Failed to create network visualization");
    }
    return result;
}

CQError scene_create_heatmap_visualization(const char *metric_name, const char *surface_type, int resolution)
{
    // Initialize heatmap system if not already done
    CQError init_result = heatmap_init();
    if (init_result != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to initialize heatmap system");
        return init_result;
    }

    CQError result = heatmap_create(metric_name, surface_type, resolution);
    if (result == CQ_SUCCESS)
    {
        current_mode = VISUALIZATION_HEATMAP;

        // Store parameters for save/load functionality
        if (metric_name) strncpy(current_color_metric, metric_name, sizeof(current_color_metric) - 1);
        if (surface_type) strncpy(current_surface_type, surface_type, sizeof(current_surface_type) - 1);
        current_heatmap_resolution = resolution;

        LOG_INFO("Heatmap visualization created successfully for metric '%s'", metric_name);
    }
    else
    {
        LOG_ERROR("Failed to create heatmap visualization");
    }
    return result;
}

void scene_handle_display_shortcuts(void)
{
    // Handle display toggle shortcuts based on current visualization mode
    switch (current_mode)
    {
    case VISUALIZATION_SCATTER_PLOT:
        // A: Toggle axes
        if (input_is_key_pressed(GLFW_KEY_A))
        {
            scatter_plot_toggle_axes();
        }

        // G: Toggle grid
        if (input_is_key_pressed(GLFW_KEY_G))
        {
            scatter_plot_toggle_grid();
        }

        // P: Toggle points
        if (input_is_key_pressed(GLFW_KEY_P))
        {
            scatter_plot_toggle_points();
        }

        // B: Toggle labels
        if (input_is_key_pressed(GLFW_KEY_B))
        {
            scatter_plot_toggle_labels();
        }
        break;

    case VISUALIZATION_TREE:
    case VISUALIZATION_NETWORK:
    case VISUALIZATION_HEATMAP:
        // TODO: Implement display toggles for other visualization types
        break;

    default:
        break;
    }
}

CQError scene_get_current_state(VisualizationState *state)
{
    if (!state)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Get camera state (this would need to be implemented in camera.c)
    // For now, use default values
    memset(&state->camera, 0, sizeof(state->camera));
    state->camera.fov = 45.0f;
    state->camera.near_plane = 0.1f;
    state->camera.far_plane = 1000.0f;

    // Get scene state
    state->mode = current_mode;
    strncpy(state->x_metric, current_x_metric, sizeof(state->x_metric) - 1);
    strncpy(state->y_metric, current_y_metric, sizeof(state->y_metric) - 1);
    strncpy(state->z_metric, current_z_metric, sizeof(state->z_metric) - 1);
    strncpy(state->color_metric, current_color_metric, sizeof(state->color_metric) - 1);
    strncpy(state->surface_type, current_surface_type, sizeof(state->surface_type) - 1);
    state->heatmap_resolution = current_heatmap_resolution;

    // Get filters and display options
    state->filters = current_filters;
    state->display_options = current_display_options;

    // Set version
    state->version = 1;

    return CQ_SUCCESS;
}

CQError scene_set_state(const VisualizationState *state)
{
    if (!state)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Set camera state (this would need to be implemented in camera.c)
    // For now, skip camera restoration

    // Set scene state
    current_mode = state->mode;
    strncpy(current_x_metric, state->x_metric, sizeof(current_x_metric) - 1);
    strncpy(current_y_metric, state->y_metric, sizeof(current_y_metric) - 1);
    strncpy(current_z_metric, state->z_metric, sizeof(current_z_metric) - 1);
    strncpy(current_color_metric, state->color_metric, sizeof(current_color_metric) - 1);
    strncpy(current_surface_type, state->surface_type, sizeof(current_surface_type) - 1);
    current_heatmap_resolution = state->heatmap_resolution;

    // Set filters and display options
    current_filters = state->filters;
    current_display_options = state->display_options;

    // Recreate the visualization based on the loaded state
    CQError result = CQ_SUCCESS;
    switch (state->mode)
    {
    case VISUALIZATION_SCATTER_PLOT:
        if (strlen(state->x_metric) > 0 && strlen(state->y_metric) > 0 && strlen(state->z_metric) > 0)
        {
            result = scene_create_scatter_plot_filtered(state->x_metric, state->y_metric,
                                                       state->z_metric, state->color_metric,
                                                       &state->filters, &state->display_options);
        }
        break;

    case VISUALIZATION_HEATMAP:
        if (strlen(state->color_metric) > 0)
        {
            result = scene_create_heatmap_visualization(state->color_metric,
                                                       state->surface_type,
                                                       state->heatmap_resolution);
        }
        break;

    case VISUALIZATION_NONE:
    default:
        // No visualization to recreate
        break;
    }

    return result;
}

CQError scene_save_visualization_state(const char *filepath)
{
    if (!filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    FILE *file = fopen(filepath, "wb");
    if (!file)
    {
        LOG_ERROR("Failed to open file for writing: %s", filepath);
        return CQ_ERROR_FILE_NOT_FOUND;
    }

    VisualizationState state;
    CQError result = scene_get_current_state(&state);
    if (result != CQ_SUCCESS)
    {
        fclose(file);
        return result;
    }

    // Write the state to file
    size_t written = fwrite(&state, sizeof(VisualizationState), 1, file);
    if (written != 1)
    {
        LOG_ERROR("Failed to write visualization state to file");
        fclose(file);
        return CQ_ERROR_UNKNOWN;
    }

    fclose(file);
    LOG_INFO("Visualization state saved to: %s", filepath);
    return CQ_SUCCESS;
}

CQError scene_load_visualization_state(const char *filepath)
{
    if (!filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        LOG_ERROR("Failed to open file for reading: %s", filepath);
        return CQ_ERROR_FILE_NOT_FOUND;
    }

    VisualizationState state;
    size_t read = fread(&state, sizeof(VisualizationState), 1, file);
    if (read != 1)
    {
        LOG_ERROR("Failed to read visualization state from file");
        fclose(file);
        return CQ_ERROR_UNKNOWN;
    }

    fclose(file);

    // Validate version
    if (state.version != 1)
    {
        LOG_ERROR("Unsupported visualization state version: %d", state.version);
        return CQ_ERROR_UNKNOWN;
    }

    // Apply the loaded state
    CQError result = scene_set_state(&state);
    if (result == CQ_SUCCESS)
    {
        LOG_INFO("Visualization state loaded from: %s", filepath);
    }

    return result;
}
