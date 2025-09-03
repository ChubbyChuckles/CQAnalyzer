#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "visualizer/renderer.h"
#include "visualizer/shader.h"
#include "visualizer/camera.h"
#include "visualizer/color.h"
#include "visualizer/gradient.h"
#include "visualizer/text_renderer.h"
#include "visualizer/lighting.h"
#include "visualizer/visualization_filters.h"
#include "visualizer/scene.h"
#include "visualizer/picking.h"
#include "visualizer/profiler.h"
#include "ui/input_handler.h"
#include "ui/imgui_integration.h"
#include "utils/logger.h"
#include "utils/bmp_writer.h"

static bool renderer_initialized = false;
static int window_width = 800;
static int window_height = 600;
static GLFWwindow *window = NULL;
static Shader basic_shader;
static Shader phong_shader;
static Camera camera;
static GLuint cube_vao, cube_vbo, cube_ebo;
static GLuint sphere_vao, sphere_vbo, sphere_ebo;
static GLuint line_vao, line_vbo;
static TextRenderer text_renderer;
static bool text_renderer_initialized = false;

// Fullscreen state management
static bool is_fullscreen = false;
static int windowed_x = 0;
static int windowed_y = 0;
static int windowed_width = 800;
static int windowed_height = 600;

// Camera control variables
static double last_mouse_x = 0.0;
static double last_mouse_y = 0.0;
static bool mouse_dragging_left = false;
static bool mouse_dragging_right = false;
static float camera_sensitivity = 0.005f;
static float zoom_sensitivity = 0.1f;

// Keyboard shortcut state variables
static bool wireframe_mode = false;
static bool lighting_enabled = true;
static bool key_states[GLFW_KEY_LAST] = {false};

// Forward declarations
static void setup_cube_geometry(void);
static void setup_sphere_geometry(void);
static void setup_line_geometry(void);
static void create_model_matrix(float x, float y, float z, float scale, float model[16]);
static void handle_keyboard_shortcuts(void);
static void set_material_uniforms(const Material *material);
static void set_light_uniforms(const Light *light);
static void set_view_position_uniform();
static void toggle_fullscreen(void);
static void enter_fullscreen(void);
static void exit_fullscreen(void);

CQError renderer_init(int width, int height, const char *title)
{
    if (renderer_initialized)
    {
        return CQ_SUCCESS;
    }

    window_width = width;
    window_height = height;
    windowed_width = width;
    windowed_height = height;

    LOG_INFO("Initializing 3D renderer (%dx%d): %s", width, height, title);

    // Initialize GLFW
    if (!glfwInit())
    {
        LOG_ERROR("Failed to initialize GLFW");
        return CQ_ERROR_UNKNOWN;
    }

    // Set GLFW window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create window
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window)
    {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return CQ_ERROR_UNKNOWN;
    }

    // Make the window's context current
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
    glViewport(0, 0, width, height);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Initialize camera
    camera_init(&camera);

    // Initialize picking system
    if (picking_init() != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to initialize picking system");
        glfwDestroyWindow(window);
        glfwTerminate();
        return CQ_ERROR_UNKNOWN;
    }

    // Initialize ImGui
    if (!imgui_init(window))
    {
        LOG_ERROR("Failed to initialize ImGui");
        picking_shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
        return CQ_ERROR_UNKNOWN;
    }

    // Create basic shader from source
    const char *vertex_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 ourColor;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "    ourColor = aColor;\n"
        "}\0";

    const char *fragment_shader_source =
        "#version 330 core\n"
        "in vec3 ourColor;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(ourColor, 1.0);\n"
        "}\0";

    if (shader_load_from_source(vertex_shader_source, fragment_shader_source, &basic_shader) != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to load basic shader");
        return CQ_ERROR_UNKNOWN;
    }

    // Load Phong shader from files
    if (shader_load_from_files("src/visualizer/shaders/phong.vert", "src/visualizer/shaders/phong.frag", &phong_shader) != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to load Phong shader");
        return CQ_ERROR_UNKNOWN;
    }

    // Set up cube geometry
    setup_cube_geometry();

    // Set up sphere geometry
    setup_sphere_geometry();

    // Set up line geometry
    setup_line_geometry();

    // Initialize performance profiler
    if (profiler_init() != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to initialize performance profiler");
        return CQ_ERROR_UNKNOWN;
    }

    renderer_initialized = true;
    LOG_INFO("3D renderer initialized successfully");
    return CQ_SUCCESS;
}

// Helper function to create model matrix
static void create_model_matrix(float x, float y, float z, float scale, float model[16])
{
    // Identity matrix
    model[0] = scale; model[1] = 0.0f;  model[2] = 0.0f;  model[3] = 0.0f;
    model[4] = 0.0f;  model[5] = scale; model[6] = 0.0f;  model[7] = 0.0f;
    model[8] = 0.0f;  model[9] = 0.0f;  model[10] = scale; model[11] = 0.0f;
    model[12] = x;    model[13] = y;    model[14] = z;    model[15] = 1.0f;
}

static void setup_cube_geometry(void)
{
    // Cube vertices (position + normal + color)
    float vertices[] = {
        // Front face (normal: 0, 0, 1)
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f,
        // Back face (normal: 0, 0, -1)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        // Left face (normal: -1, 0, 0)
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 1.0f,
        // Right face (normal: 1, 0, 0)
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f,
        // Top face (normal: 0, 1, 0)
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 1.0f,
        // Bottom face (normal: 0, -1, 0)
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,       // Front
        4, 5, 6, 6, 7, 4,       // Back
        8, 9, 10, 10, 11, 8,    // Left
        12, 13, 14, 14, 15, 12, // Right
        16, 17, 18, 18, 19, 16, // Top
        20, 21, 22, 22, 23, 20  // Bottom
    };

    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &cube_vbo);
    glGenBuffers(1, &cube_ebo);

    glBindVertexArray(cube_vao);

    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Color attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

static void setup_sphere_geometry(void)
{
    // Simple sphere approximation using triangles
    const int stacks = 8;
    const int slices = 8;
    const int num_vertices = (stacks + 1) * (slices + 1);
    const int num_indices = stacks * slices * 6;

    float *vertices = (float *)malloc(num_vertices * 9 * sizeof(float)); // position + normal + color
    unsigned int *indices = (unsigned int *)malloc(num_indices * sizeof(unsigned int));

    int vertex_index = 0;
    int index_index = 0;

    for (int i = 0; i <= stacks; ++i)
    {
        float phi = (float)M_PI * i / stacks;
        for (int j = 0; j <= slices; ++j)
        {
            float theta = 2.0f * (float)M_PI * j / slices;

            float x = sinf(phi) * cosf(theta);
            float y = cosf(phi);
            float z = sinf(phi) * sinf(theta);

            // Position
            vertices[vertex_index++] = x;
            vertices[vertex_index++] = y;
            vertices[vertex_index++] = z;

            // Normal (same as position for unit sphere)
            vertices[vertex_index++] = x;
            vertices[vertex_index++] = y;
            vertices[vertex_index++] = z;

            // Color (blue-ish)
            vertices[vertex_index++] = 0.0f;
            vertices[vertex_index++] = 0.5f;
            vertices[vertex_index++] = 1.0f;
        }
    }

    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            indices[index_index++] = first;
            indices[index_index++] = second;
            indices[index_index++] = first + 1;

            indices[index_index++] = second;
            indices[index_index++] = second + 1;
            indices[index_index++] = first + 1;
        }
    }

    glGenVertexArrays(1, &sphere_vao);
    glGenBuffers(1, &sphere_vbo);
    glGenBuffers(1, &sphere_ebo);

    glBindVertexArray(sphere_vao);

    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * 9 * sizeof(float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Color attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    free(vertices);
    free(indices);
}

static void setup_line_geometry(void)
{
    glGenVertexArrays(1, &line_vao);
    glGenBuffers(1, &line_vbo);

    glBindVertexArray(line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

    // Allocate buffer for 2 vertices (start and end points)
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void renderer_shutdown(void)
{
    if (!renderer_initialized)
    {
        return;
    }

    LOG_INFO("Shutting down 3D renderer");

    // Shutdown picking system
    picking_shutdown();

    // Shutdown ImGui
    imgui_shutdown();

    // Shutdown text renderer if initialized
    if (text_renderer_initialized)
    {
        text_renderer_shutdown(&text_renderer);
        text_renderer_initialized = false;
    }

    // Shutdown performance profiler
    profiler_shutdown();

    // Clean up OpenGL resources
    glDeleteVertexArrays(1, &cube_vao);
    glDeleteBuffers(1, &cube_vbo);
    glDeleteBuffers(1, &cube_ebo);
    glDeleteVertexArrays(1, &sphere_vao);
    glDeleteBuffers(1, &sphere_vbo);
    glDeleteBuffers(1, &sphere_ebo);
    glDeleteVertexArrays(1, &line_vao);
    glDeleteBuffers(1, &line_vbo);

    // Destroy GLFW window and terminate GLFW
    if (window)
    {
        glfwDestroyWindow(window);
        window = NULL;
    }
    glfwTerminate();

    renderer_initialized = false;
    LOG_INFO("3D renderer shutdown complete");
}

bool renderer_is_running(void)
{
    if (!renderer_initialized || !window)
    {
        return false;
    }

    return !glfwWindowShouldClose(window);
}

void renderer_update(void)
{
    if (!renderer_initialized)
    {
        return;
    }

    // Start profiler timing for update
    profiler_start_update();

    // Get current mouse position
    double current_mouse_x, current_mouse_y;
    // Note: In a real implementation, this would get mouse position from GLFW
    // For now, we'll assume input_handler provides this
    // input_handle_mouse_move is called externally, so we need to get the current position

    // Since input_handler stores mouse position, we can access it
    // But we need to track deltas ourselves
    static bool first_frame = true;

    if (first_frame)
    {
        // Initialize last mouse position
        // This is a simplification - in real GLFW integration, we'd get initial position
        last_mouse_x = 400.0; // Assume center of 800x600 window
        last_mouse_y = 300.0;
        first_frame = false;
    }

    // Get current mouse position from input handler
    double mouse_x, mouse_y;
    input_get_mouse_position(&mouse_x, &mouse_y);

    // Handle mouse button states for dragging
    bool left_button_pressed = input_is_mouse_button_pressed(0); // Left button
    bool right_button_pressed = input_is_mouse_button_pressed(1); // Right button

    // Handle picking on left mouse button click (press without drag)
    static bool left_button_was_pressed = false;
    if (left_button_pressed && !left_button_was_pressed && !mouse_dragging_left)
    {
        // Perform picking
        int picked_object = picking_pick_object((float)mouse_x, (float)mouse_y,
                                               window_width, window_height);
        if (picked_object != -1)
        {
            // Toggle selection
            if (picking_is_selected(picked_object))
            {
                picking_deselect_object(picked_object);
                LOG_INFO("Deselected object ID: %d", picked_object);
            }
            else
            {
                picking_select_object(picked_object);
                LOG_INFO("Selected object ID: %d", picked_object);
            }
        }
    }
    left_button_was_pressed = left_button_pressed;

    // Start dragging on button press
    if (left_button_pressed && !mouse_dragging_left)
    {
        mouse_dragging_left = true;
        // Reset last position to current to avoid jump
        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
    }
    else if (!left_button_pressed)
    {
        mouse_dragging_left = false;
    }

    if (right_button_pressed && !mouse_dragging_right)
    {
        mouse_dragging_right = true;
        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
    }
    else if (!right_button_pressed)
    {
        mouse_dragging_right = false;
    }

    // Calculate mouse delta
    double delta_x = mouse_x - last_mouse_x;
    double delta_y = mouse_y - last_mouse_y;

    // Update camera based on mouse input
    if (mouse_dragging_left)
    {
        // Left mouse drag: rotate camera
        float yaw = (float)delta_x * camera_sensitivity;
        float pitch = (float)delta_y * camera_sensitivity;
        camera_rotate(&camera, yaw, pitch);
    }

    if (mouse_dragging_right)
    {
        // Right mouse drag: pan camera (move in XY plane)
        // Calculate right and up vectors for panning
        float forward[3] = {
            camera.target[0] - camera.position[0],
            camera.target[1] - camera.position[1],
            camera.target[2] - camera.position[2]
        };

        // Normalize forward
        float forward_length = sqrtf(forward[0]*forward[0] + forward[1]*forward[1] + forward[2]*forward[2]);
        if (forward_length > 0.0f)
        {
            forward[0] /= forward_length;
            forward[1] /= forward_length;
            forward[2] /= forward_length;
        }

        // Calculate right vector
        float right[3] = {
            forward[1] * camera.up[2] - forward[2] * camera.up[1],
            forward[2] * camera.up[0] - forward[0] * camera.up[2],
            forward[0] * camera.up[1] - forward[1] * camera.up[0]
        };

        // Normalize right
        float right_length = sqrtf(right[0]*right[0] + right[1]*right[1] + right[2]*right[2]);
        if (right_length > 0.0f)
        {
            right[0] /= right_length;
            right[1] /= right_length;
            right[2] /= right_length;
        }

        // Pan camera
        float pan_speed = 0.01f;
        float pan_x = (float)delta_x * pan_speed;
        float pan_y = (float)delta_y * pan_speed;

        camera_move(&camera, -right[0] * pan_x, -right[1] * pan_x, -right[2] * pan_x);
        camera_move(&camera, -camera.up[0] * pan_y, -camera.up[1] * pan_y, -camera.up[2] * pan_y);

        // Move target as well to maintain look direction
        camera.target[0] -= right[0] * pan_x;
        camera.target[1] -= right[1] * pan_x;
        camera.target[2] -= right[2] * pan_x;
        camera.target[0] -= camera.up[0] * pan_y;
        camera.target[1] -= camera.up[1] * pan_y;
        camera.target[2] -= camera.up[2] * pan_y;
    }

    // Update last mouse position
    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;

    // Handle scroll for zooming
    double scroll_x_delta, scroll_y_delta;
    input_get_scroll_delta(&scroll_x_delta, &scroll_y_delta);

    if (scroll_y_delta != 0.0)
    {
        float zoom_factor = (float)scroll_y_delta * zoom_sensitivity;
        camera_zoom(&camera, zoom_factor);
    }

    // Handle keyboard shortcuts
    handle_keyboard_shortcuts();

    // End profiler timing for update
    profiler_end_update();
}

// Helper function to handle keyboard shortcuts
static void handle_keyboard_shortcuts(void)
{
    // ESC: Exit application
    if (input_is_key_pressed(GLFW_KEY_ESCAPE))
    {
        LOG_INFO("ESC pressed - exiting application");
        // Note: In a real implementation, this would signal the main loop to exit
        // For now, we'll just log it
    }

    // R: Reset camera to default position
    if (input_is_key_pressed(GLFW_KEY_R))
    {
        camera_init(&camera);
        LOG_INFO("Camera reset to default position");
    }

    // + / =: Zoom in
    if (input_is_key_pressed(GLFW_KEY_EQUAL) || input_is_key_pressed(GLFW_KEY_KP_ADD))
    {
        camera_zoom(&camera, -zoom_sensitivity * 2.0f);
        LOG_DEBUG("Zoom in");
    }

    // -: Zoom out
    if (input_is_key_pressed(GLFW_KEY_MINUS) || input_is_key_pressed(GLFW_KEY_KP_SUBTRACT))
    {
        camera_zoom(&camera, zoom_sensitivity * 2.0f);
        LOG_DEBUG("Zoom out");
    }

    // 1: Switch to scatter plot mode
    if (input_is_key_pressed(GLFW_KEY_1))
    {
        scene_set_visualization_mode(VISUALIZATION_SCATTER_PLOT);
        LOG_INFO("Switched to scatter plot visualization");
    }

    // 2: Switch to tree mode
    if (input_is_key_pressed(GLFW_KEY_2))
    {
        scene_set_visualization_mode(VISUALIZATION_TREE);
        LOG_INFO("Switched to tree visualization");
    }

    // 3: Switch to network mode
    if (input_is_key_pressed(GLFW_KEY_3))
    {
        scene_set_visualization_mode(VISUALIZATION_NETWORK);
        LOG_INFO("Switched to network visualization");
    }

    // 4: Switch to heatmap mode
    if (input_is_key_pressed(GLFW_KEY_4))
    {
        scene_set_visualization_mode(VISUALIZATION_HEATMAP);
        LOG_INFO("Switched to heatmap visualization");
    }

    // W: Toggle wireframe mode
    if (input_is_key_pressed(GLFW_KEY_W))
    {
        wireframe_mode = !wireframe_mode;
        if (wireframe_mode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            LOG_INFO("Wireframe mode enabled");
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            LOG_INFO("Wireframe mode disabled");
        }
    }

    // L: Toggle lighting
    if (input_is_key_pressed(GLFW_KEY_L))
    {
        lighting_enabled = !lighting_enabled;
        LOG_INFO("Lighting %s", lighting_enabled ? "enabled" : "disabled");
        // Note: This would need to be passed to shaders in a full implementation
    }

    // S: Take screenshot
    if (input_is_key_pressed(GLFW_KEY_S))
    {
        renderer_take_screenshot("screenshot.bmp");
    }

    // V: Toggle video recording
    static bool video_recording = false;
    if (input_is_key_pressed(GLFW_KEY_V))
    {
        video_recording = !video_recording;
        if (video_recording)
        {
            renderer_start_video_recording("video_frames/frame_%04d.bmp");
            LOG_INFO("Video recording started");
        }
        else
        {
            renderer_stop_video_recording();
            LOG_INFO("Video recording stopped");
        }
    }

    // F11: Toggle fullscreen
    if (input_is_key_pressed(GLFW_KEY_F11))
    {
        toggle_fullscreen();
    }

    // P: Toggle profiler overlay
    if (input_is_key_pressed(GLFW_KEY_P))
    {
        profiler_toggle_overlay();
    }

    // H: Show help
    if (input_is_key_pressed(GLFW_KEY_H))
    {
        LOG_INFO("Keyboard shortcuts:");
        LOG_INFO("  ESC: Exit");
        LOG_INFO("  F11: Toggle fullscreen");
        LOG_INFO("  R: Reset camera");
        LOG_INFO("  +/-: Zoom in/out");
        LOG_INFO("  1-4: Switch visualization modes");
        LOG_INFO("  W: Toggle wireframe");
        LOG_INFO("  L: Toggle lighting");
        LOG_INFO("  S: Take screenshot");
        LOG_INFO("  V: Toggle video recording");
        LOG_INFO("  P: Toggle profiler overlay");
        LOG_INFO("  Ctrl+O: Open project");
        LOG_INFO("  Ctrl+B: Open file browser");
        LOG_INFO("  Ctrl+M: Toggle metric configuration panel");
        LOG_INFO("  Ctrl+C: Toggle camera controls panel");
        LOG_INFO("  Ctrl+D: Toggle display options panel");
        LOG_INFO("  Ctrl+L: Toggle color scheme panel");
        LOG_INFO("  Ctrl+A: Toggle animation controls panel");
        LOG_INFO("  Ctrl+, : Open settings dialog");
        LOG_INFO("  Ctrl+Shift+S: Save dock layout");
        LOG_INFO("  Ctrl+Shift+L: Load dock layout");
        LOG_INFO("  Ctrl+Shift+R: Reset dock layout");
        LOG_INFO("  F1: Toggle ImGui demo");
        LOG_INFO("  Shift+F1: Show keyboard shortcuts dialog");
        LOG_INFO("  A: Toggle axes");
        LOG_INFO("  G: Toggle grid");
        LOG_INFO("  P: Toggle points");
        LOG_INFO("  B: Toggle labels");
        LOG_INFO("  H: Show this help");
    }

    // Display toggles are now handled in scene.c based on current visualization mode
}

void renderer_handle_scroll(double x_offset, double y_offset)
{
    if (!renderer_initialized)
    {
        return;
    }

    // Use Y offset for zoom (positive = zoom in, negative = zoom out)
    float zoom_factor = (float)y_offset * zoom_sensitivity;
    camera_zoom(&camera, zoom_factor);
}

void renderer_render(void)
{
    if (!renderer_initialized)
    {
        return;
    }

    // Start profiler timing for render
    profiler_start_render();

    // Start ImGui frame
    imgui_new_frame();

    // Initialize menu state if not already done
    static bool menu_initialized = false;
    if (!menu_initialized)
    {
        menu_state_init();
        menu_initialized = true;
    }

    // Create dock space
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    bool open = true;
    ImGui::Begin("CQAnalyzer DockSpace", &open, window_flags);
    ImGui::PopStyleVar(3);

    // Create dock space
    ImGuiID dockspace_id = ImGui::GetID("CQAnalyzerDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    // Show main menu bar
    imgui_show_main_menu_bar();

    // Show about dialog if requested
    imgui_show_about_dialog();

    // Show help dialogs if requested
    imgui_show_help_keyboard_shortcuts(&menu_state.show_help_keyboard_shortcuts);
    imgui_show_help_documentation(&menu_state.show_help_documentation);
    imgui_show_help_faq(&menu_state.show_help_faq);
    imgui_show_help_system_info(&menu_state.show_help_system_info);

    // TODO: Clear buffers
    // TODO: Set up view and projection matrices
    // TODO: Render 3D scene

    // Show ImGui demo window (can be toggled)
    static bool show_demo_window = false;
    static bool show_main_panel = true;
    static bool show_metrics = false;

    // Handle keyboard shortcuts for ImGui
    if (input_is_key_pressed(GLFW_KEY_F1))
    {
        show_demo_window = !show_demo_window;
    }

    // F1: Show help (keyboard shortcuts)
    if (input_is_key_pressed(GLFW_KEY_F1) && input_is_key_pressed(GLFW_KEY_LEFT_SHIFT))
    {
        menu_state.show_help_keyboard_shortcuts = !menu_state.show_help_keyboard_shortcuts;
    }
    if (input_is_key_pressed(GLFW_KEY_F2))
    {
        show_main_panel = !show_main_panel;
    }
    if (input_is_key_pressed(GLFW_KEY_F3))
    {
        show_metrics = !show_metrics;
    }

    // Ctrl+O: Open project selector
    if (input_is_key_pressed(GLFW_KEY_O) && input_is_key_pressed(GLFW_KEY_LEFT_CONTROL))
    {
        menu_state.show_project_selector = true;
    }

    // Ctrl+B: Open file browser
    if (input_is_key_pressed(GLFW_KEY_B) && input_is_key_pressed(GLFW_KEY_LEFT_CONTROL))
    {
        menu_state.show_file_browser = true;
    }

    // Ctrl+M: Toggle metric configuration panel
    if (input_is_key_pressed(GLFW_KEY_M) && input_is_key_pressed(GLFW_KEY_LEFT_CONTROL))
    {
        menu_state.show_metric_config_panel = !menu_state.show_metric_config_panel;
    }

    // Ctrl+C: Toggle camera controls panel
    if (input_is_key_pressed(GLFW_KEY_C) && input_is_key_pressed(GLFW_KEY_LEFT_CONTROL))
    {
        menu_state.show_camera_controls = !menu_state.show_camera_controls;
    }

    // Ctrl+D: Toggle display options panel
    if (input_is_key_pressed(GLFW_KEY_D) && input_is_key_pressed(GLFW_KEY_LEFT_CONTROL))
    {
        menu_state.show_display_options = !menu_state.show_display_options;
    }

    // Ctrl+L: Toggle color scheme panel
    if (input_is_key_pressed(GLFW_KEY_L) && input_is_key_pressed(GLFW_KEY_LEFT_CONTROL))
    {
        menu_state.show_color_scheme = !menu_state.show_color_scheme;
    }

    // Ctrl+A: Toggle animation controls panel
    if (input_is_key_pressed(GLFW_KEY_A) && input_is_key_pressed(GLFW_KEY_LEFT_CONTROL))
    {
        menu_state.show_animation_controls = !menu_state.show_animation_controls;
    }

    // Ctrl+, : Open settings dialog
    if (input_is_key_pressed(GLFW_KEY_COMMA) && input_is_key_pressed(GLFW_KEY_LEFT_CONTROL))
    {
        menu_state.show_settings_dialog = true;
    }

    // Ctrl+Shift+S: Save dock layout
    if (input_is_key_pressed(GLFW_KEY_S) && input_is_key_pressed(GLFW_KEY_LEFT_CONTROL) && input_is_key_pressed(GLFW_KEY_LEFT_SHIFT))
    {
        imgui_save_dock_layout("current");
    }

    // Ctrl+Shift+L: Load dock layout
    if (input_is_key_pressed(GLFW_KEY_L) && input_is_key_pressed(GLFW_KEY_LEFT_CONTROL) && input_is_key_pressed(GLFW_KEY_LEFT_SHIFT))
    {
        imgui_load_dock_layout("current");
    }

    // Ctrl+Shift+R: Reset dock layout
    if (input_is_key_pressed(GLFW_KEY_R) && input_is_key_pressed(GLFW_KEY_LEFT_CONTROL) && input_is_key_pressed(GLFW_KEY_LEFT_SHIFT))
    {
        imgui_reset_dock_layout();
    }

    // Show ImGui windows
    imgui_show_demo_window(&show_demo_window);
    imgui_show_main_control_panel(&show_main_panel);
    imgui_show_metrics_window(&show_metrics);

    // Apply control panel settings to the rendering system
    imgui_apply_camera_settings();
    imgui_apply_display_settings();
    imgui_apply_color_scheme();

    // Show additional windows based on menu state
    imgui_show_visualization_settings(&menu_state.show_visualization_settings);
    imgui_show_analysis_results(&menu_state.show_analysis_results);
    imgui_show_metric_config_panel(&menu_state.show_metric_config_panel);
    imgui_show_file_browser_dialog(&menu_state.show_file_browser);
    imgui_show_project_selector_dialog(&menu_state.show_project_selector);

    // Show new control panels
    imgui_show_camera_control_panel(&menu_state.show_camera_controls);
    imgui_show_display_options_panel(&menu_state.show_display_options);
    imgui_show_color_scheme_panel(&menu_state.show_color_scheme);
    imgui_show_animation_control_panel(&menu_state.show_animation_controls);
    imgui_show_visualization_mode_panel(&menu_state.show_visualization_settings); // Reuse existing flag

    // Show settings dialog
    imgui_show_settings_dialog(&menu_state.show_settings_dialog);

    // End dock space
    ImGui::End();

    // Render ImGui
    imgui_render();

    // Render profiler overlay if enabled
    profiler_render_overlay();

    // Capture video frame if recording is active
    renderer_capture_video_frame();

    // End profiler timing for render
    profiler_end_render();

    LOG_WARNING("Scene rendering not yet implemented");
}

void renderer_present(void)
{
    if (!renderer_initialized || !window)
    {
        return;
    }

    // End frame timing before presenting
    profiler_end_frame();

    // Swap buffers
    glfwSwapBuffers(window);

    // Poll events
    glfwPollEvents();

    // Start next frame timing after presenting
    profiler_start_frame();
}

void renderer_draw_cube(float x, float y, float z, float size, float r, float g, float b)
{
    if (!renderer_initialized)
    {
        return;
    }

    shader_use(&basic_shader);

    // Set up matrices
    float model[16];
    create_model_matrix(x, y, z, size, model);

    float view[16];
    camera_get_view_matrix(&camera, view);

    float projection[16];
    float aspect = (float)window_width / (float)window_height;
    camera_get_projection_matrix(&camera, aspect, projection);

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "model"), 1, GL_FALSE, model);
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "projection"), 1, GL_FALSE, projection);

    // Draw cube
    glBindVertexArray(cube_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void renderer_draw_sphere(float x, float y, float z, float radius, float r, float g, float b)
{
    if (!renderer_initialized)
    {
        return;
    }

    shader_use(&basic_shader);

    // Set up matrices
    float model[16];
    create_model_matrix(x, y, z, radius, model);

    float view[16];
    camera_get_view_matrix(&camera, view);

    float projection[16];
    float aspect = (float)window_width / (float)window_height;
    camera_get_projection_matrix(&camera, aspect, projection);

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "model"), 1, GL_FALSE, model);
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "projection"), 1, GL_FALSE, projection);

    // Draw sphere
    glBindVertexArray(sphere_vao);
    glDrawElements(GL_TRIANGLES, 8 * 8 * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void renderer_draw_line(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b)
{
    if (!renderer_initialized)
    {
        return;
    }

    // Update line vertices
    float vertices[] = {
        x1, y1, z1, r, g, b,
        x2, y2, z2, r, g, b
    };

    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    shader_use(&basic_shader);

    // Set up matrices (identity for lines)
    float model[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    float view[16];
    camera_get_view_matrix(&camera, view);

    float projection[16];
    float aspect = (float)window_width / (float)window_height;
    camera_get_projection_matrix(&camera, aspect, projection);

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "model"), 1, GL_FALSE, model);
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "projection"), 1, GL_FALSE, projection);

    // Draw line
    glBindVertexArray(line_vao);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}

// Helper function to set material uniforms
static void set_material_uniforms(const Material *material)
{
    if (!material)
    {
        return;
    }

    shader_set_vec3(&phong_shader, "materialAmbient", material->ambient.r, material->ambient.g, material->ambient.b);
    shader_set_vec3(&phong_shader, "materialDiffuse", material->diffuse.r, material->diffuse.g, material->diffuse.b);
    shader_set_vec3(&phong_shader, "materialSpecular", material->specular.r, material->specular.g, material->specular.b);
    shader_set_float(&phong_shader, "materialShininess", material->shininess);
}

// Helper function to set light uniforms
static void set_light_uniforms(const Light *light)
{
    if (!light)
    {
        return;
    }

    shader_set_vec3(&phong_shader, "lightAmbient", light->ambient.r, light->ambient.g, light->ambient.b);
    shader_set_vec3(&phong_shader, "lightDiffuse", light->diffuse.r, light->diffuse.g, light->diffuse.b);
    shader_set_vec3(&phong_shader, "lightSpecular", light->specular.r, light->specular.g, light->specular.b);
    shader_set_vec3(&phong_shader, "lightDirection", light->direction[0], light->direction[1], light->direction[2]);
    shader_set_vec3(&phong_shader, "lightPosition", light->position[0], light->position[1], light->position[2]);
    shader_set_int(&phong_shader, "lightType", (int)light->type);
    shader_set_float(&phong_shader, "lightConstant", light->constant);
    shader_set_float(&phong_shader, "lightLinear", light->linear);
    shader_set_float(&phong_shader, "lightQuadratic", light->quadratic);
    shader_set_float(&phong_shader, "lightCutoff", light->cutoff);
    shader_set_float(&phong_shader, "lightOuterCutoff", light->outer_cutoff);
    shader_set_int(&phong_shader, "lightEnabled", light->enabled ? 1 : 0);
}

// Helper function to set camera position for lighting
static void set_view_position_uniform()
{
    shader_set_vec3(&phong_shader, "viewPos", camera.position[0], camera.position[1], camera.position[2]);
}

void renderer_draw_cube_lit(float x, float y, float z, float size, const Material *material, const Light *light)
{
    if (!renderer_initialized || !material || !light)
    {
        return;
    }

    shader_use(&phong_shader);

    // Set up matrices
    float model[16];
    create_model_matrix(x, y, z, size, model);

    float view[16];
    camera_get_view_matrix(&camera, view);

    float projection[16];
    float aspect = (float)window_width / (float)window_height;
    camera_get_projection_matrix(&camera, aspect, projection);

    // Set uniforms
    shader_set_mat4(&phong_shader, "model", model);
    shader_set_mat4(&phong_shader, "view", view);
    shader_set_mat4(&phong_shader, "projection", projection);

    // Set lighting uniforms
    set_material_uniforms(material);
    set_light_uniforms(light);
    set_view_position_uniform();

    // Draw cube
    glBindVertexArray(cube_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void renderer_draw_sphere_lit(float x, float y, float z, float radius, const Material *material, const Light *light)
{
    if (!renderer_initialized || !material || !light)
    {
        return;
    }

    shader_use(&phong_shader);

    // Set up matrices
    float model[16];
    create_model_matrix(x, y, z, radius, model);

    float view[16];
    camera_get_view_matrix(&camera, view);

    float projection[16];
    float aspect = (float)window_width / (float)window_height;
    camera_get_projection_matrix(&camera, aspect, projection);

    // Set uniforms
    shader_set_mat4(&phong_shader, "model", model);
    shader_set_mat4(&phong_shader, "view", view);
    shader_set_mat4(&phong_shader, "projection", projection);

    // Set lighting uniforms
    set_material_uniforms(material);
    set_light_uniforms(light);
    set_view_position_uniform();

    // Draw sphere
    glBindVertexArray(sphere_vao);
    glDrawElements(GL_TRIANGLES, 8 * 8 * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

CQError renderer_init_text(const char *font_path, int font_size)
{
    if (text_renderer_initialized)
    {
        return CQ_SUCCESS;
    }

    if (!renderer_initialized)
    {
        LOG_ERROR("Cannot initialize text renderer: main renderer not initialized");
        return CQ_ERROR_UNKNOWN;
    }

    CQError result = text_renderer_init(&text_renderer, font_path, font_size);
    if (result == CQ_SUCCESS)
    {
        text_renderer_initialized = true;
        LOG_INFO("Text renderer initialized successfully");
    }

    return result;
}

void renderer_shutdown_text(void)
{
    if (text_renderer_initialized)
    {
        text_renderer_shutdown(&text_renderer);
        text_renderer_initialized = false;
    }
}

void renderer_draw_text(const char *text, float x, float y, float scale, const Color *color)
{
    if (!text_renderer_initialized || !text)
    {
        return;
    }

    text_renderer_render_text(&text_renderer, text, x, y, scale, color);
}

void renderer_draw_text_3d(const char *text, float x, float y, float z, float scale, const Color *color)
{
    if (!text_renderer_initialized || !text)
    {
        return;
    }

    text_renderer_render_text_3d(&text_renderer, text, x, y, z, scale, color);
}

void renderer_draw_cube_color(float x, float y, float z, float size, const Color *color)
{
    renderer_draw_cube(x, y, z, size, color->r, color->g, color->b);
}

void renderer_draw_sphere_color(float x, float y, float z, float radius, const Color *color)
{
    renderer_draw_sphere(x, y, z, radius, color->r, color->g, color->b);
}

void renderer_draw_line_color(float x1, float y1, float z1, float x2, float y2, float z2, const Color *color)
{
    renderer_draw_line(x1, y1, z1, x2, y2, z2, color->r, color->g, color->b);
}

void renderer_draw_line_gradient(float x1, float y1, float z1, float x2, float y2, float z2, const Gradient *gradient)
{
    if (!renderer_initialized)
    {
        return;
    }

    // Get colors for start and end
    Color start_color = gradient_get_color(gradient, 0.0f);
    Color end_color = gradient_get_color(gradient, 1.0f);

    // Update line vertices with gradient colors
    float vertices[] = {
        x1, y1, z1, start_color.r, start_color.g, start_color.b,
        x2, y2, z2, end_color.r, end_color.g, end_color.b
    };

    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    shader_use(&basic_shader);

    // Set up matrices (identity for lines)
    float model[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    float view[16];
    camera_get_view_matrix(&camera, view);

    float projection[16];
    float aspect = (float)window_width / (float)window_height;
    camera_get_projection_matrix(&camera, aspect, projection);

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "model"), 1, GL_FALSE, model);
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(basic_shader.program_id, "projection"), 1, GL_FALSE, projection);

    // Draw line
    glBindVertexArray(line_vao);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}

// Fullscreen toggle functionality
static void toggle_fullscreen(void)
{
    if (!window)
    {
        return;
    }

    if (is_fullscreen)
    {
        exit_fullscreen();
    }
    else
    {
        enter_fullscreen();
    }
}

static void enter_fullscreen(void)
{
    if (!window || is_fullscreen)
    {
        return;
    }

    // Store current window position and size
    glfwGetWindowPos(window, &windowed_x, &windowed_y);
    glfwGetWindowSize(window, &windowed_width, &windowed_height);

    // Get primary monitor
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (!monitor)
    {
        LOG_ERROR("Failed to get primary monitor for fullscreen");
        return;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    if (!mode)
    {
        LOG_ERROR("Failed to get video mode for fullscreen");
        return;
    }

    // Enter fullscreen
    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    is_fullscreen = true;

    // Update viewport
    glViewport(0, 0, mode->width, mode->height);
    window_width = mode->width;
    window_height = mode->height;

    LOG_INFO("Entered fullscreen mode (%dx%d)", mode->width, mode->height);
}

static void exit_fullscreen(void)
{
    if (!window || !is_fullscreen)
    {
        return;
    }

    // Exit fullscreen
    glfwSetWindowMonitor(window, NULL, windowed_x, windowed_y, windowed_width, windowed_height, 0);
    is_fullscreen = false;

    // Update viewport
    glViewport(0, 0, windowed_width, windowed_height);
    window_width = windowed_width;
    window_height = windowed_height;

    LOG_INFO("Exited fullscreen mode (%dx%d at %d,%d)", windowed_width, windowed_height, windowed_x, windowed_y);
}

// Screenshot and video recording functionality
static bool video_recording_active = false;
static char video_filename_pattern[256] = {0};
static int video_frame_count = 0;

/**
 * Take a screenshot of the current frame
 */
void renderer_take_screenshot(const char *filename)
{
    if (!renderer_initialized)
    {
        LOG_ERROR("Cannot take screenshot: renderer not initialized");
        return;
    }

    // Allocate buffer for pixel data
    int buffer_size = window_width * window_height * 3; // RGB
    unsigned char *pixels = (unsigned char *)malloc(buffer_size);
    if (!pixels)
    {
        LOG_ERROR("Failed to allocate memory for screenshot");
        return;
    }

    // Read pixels from framebuffer
    glReadPixels(0, 0, window_width, window_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Save as BMP
    if (write_bmp(filename, window_width, window_height, pixels) == 0)
    {
        LOG_INFO("Screenshot saved to: %s", filename);
    }
    else
    {
        LOG_ERROR("Failed to save screenshot to: %s", filename);
    }

    free(pixels);
}

/**
 * Start video recording
 */
void renderer_start_video_recording(const char *filename_pattern)
{
    if (!renderer_initialized)
    {
        LOG_ERROR("Cannot start video recording: renderer not initialized");
        return;
    }

    if (video_recording_active)
    {
        LOG_WARNING("Video recording already active");
        return;
    }

    // Create video_frames directory if it doesn't exist
    // Note: In a real implementation, you'd check and create the directory
    // For now, we'll assume it exists or let the BMP writer fail gracefully

    strncpy(video_filename_pattern, filename_pattern, sizeof(video_filename_pattern) - 1);
    video_frame_count = 0;
    video_recording_active = true;
}

/**
 * Stop video recording
 */
void renderer_stop_video_recording(void)
{
    if (!video_recording_active)
    {
        LOG_WARNING("No active video recording to stop");
        return;
    }

    video_recording_active = false;
    LOG_INFO("Video recording stopped. Captured %d frames", video_frame_count);
}

/**
 * Capture a frame for video recording (called during render loop)
 */
void renderer_capture_video_frame(void)
{
    if (!video_recording_active || !renderer_initialized)
    {
        return;
    }

    char filename[256];
    snprintf(filename, sizeof(filename), video_filename_pattern, video_frame_count);

    // Allocate buffer for pixel data
    int buffer_size = window_width * window_height * 3; // RGB
    unsigned char *pixels = (unsigned char *)malloc(buffer_size);
    if (!pixels)
    {
        LOG_ERROR("Failed to allocate memory for video frame");
        return;
    }

    // Read pixels from framebuffer
    glReadPixels(0, 0, window_width, window_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Save frame
    if (write_bmp(filename, window_width, window_height, pixels) == 0)
    {
        video_frame_count++;
    }
    else
    {
        LOG_ERROR("Failed to save video frame: %s", filename);
    }

    free(pixels);
}