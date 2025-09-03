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
#include "ui/input_handler.h"
#include "utils/logger.h"

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

CQError renderer_init(int width, int height, const char *title)
{
    if (renderer_initialized)
    {
        return CQ_SUCCESS;
    }

    window_width = width;
    window_height = height;

    LOG_INFO("Initializing 3D renderer (%dx%d): %s", width, height, title);

    // Initialize camera
    camera_init(&camera);

    // Initialize picking system
    if (picking_init() != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to initialize picking system");
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

    // Shutdown text renderer if initialized
    if (text_renderer_initialized)
    {
        text_renderer_shutdown(&text_renderer);
        text_renderer_initialized = false;
    }

    // TODO: Clean up OpenGL resources
    // TODO: Destroy GLFW window
    // TODO: Terminate GLFW

    LOG_WARNING("3D renderer shutdown not yet implemented");
    renderer_initialized = false;
}

bool renderer_is_running(void)
{
    if (!renderer_initialized)
    {
        return false;
    }

    // TODO: Check if window should close
    // For now, return true to indicate running
    return true;
}

void renderer_update(void)
{
    if (!renderer_initialized)
    {
        return;
    }

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

    // S: Take screenshot (placeholder)
    if (input_is_key_pressed(GLFW_KEY_S))
    {
        LOG_INFO("Screenshot requested (not yet implemented)");
        // TODO: Implement screenshot functionality
    }

    // H: Show help
    if (input_is_key_pressed(GLFW_KEY_H))
    {
        LOG_INFO("Keyboard shortcuts:");
        LOG_INFO("  ESC: Exit");
        LOG_INFO("  R: Reset camera");
        LOG_INFO("  +/-: Zoom in/out");
        LOG_INFO("  1-4: Switch visualization modes");
        LOG_INFO("  W: Toggle wireframe");
        LOG_INFO("  L: Toggle lighting");
        LOG_INFO("  S: Screenshot");
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

    // TODO: Clear buffers
    // TODO: Set up view and projection matrices
    // TODO: Render 3D scene
    // TODO: Draw UI elements

    LOG_WARNING("Scene rendering not yet implemented");
}

void renderer_present(void)
{
    if (!renderer_initialized)
    {
        return;
    }

    // TODO: Swap buffers
    // TODO: Poll events

    LOG_WARNING("Buffer presentation not yet implemented");
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