#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "visualizer/scene.h"
#include "visualizer/scatter_plot.h"
#include "visualizer/tree_visualization.h"
#include "utils/logger.h"

#define MAX_SCENE_OBJECTS 1000

static SceneObject *scene_objects = NULL;
static int num_objects = 0;
static int max_objects = 0;
static GLFWwindow *window = NULL;
static VisualizationMode current_mode = VISUALIZATION_NONE;

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

    LOG_INFO("OpenGL context initialized with GLFW window management");
    LOG_INFO("3D scene initialized with capacity for %d objects", max_objects);
    return CQ_SUCCESS;
}

void scene_shutdown(void)
{
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
    // Render based on current visualization mode
    switch (current_mode)
    {
    case VISUALIZATION_SCATTER_PLOT:
        scatter_plot_render();
        break;

    case VISUALIZATION_TREE:
        tree_visualization_render();
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
        LOG_INFO("Scatter plot visualization created successfully");
    }
    else
    {
        LOG_ERROR("Failed to create scatter plot visualization");
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
