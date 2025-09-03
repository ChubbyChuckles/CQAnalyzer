#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "visualizer/complexity_landscape.h"
#include "visualizer/shader.h"
#include "visualizer/color.h"
#include "visualizer/gradient.h"
#include "visualizer/renderer.h"
#include "data/data_store.h"
#include "data/metric_aggregator.h"
#include "utils/logger.h"

#define MAX_FILES 10000
#define MAX_PATH_LENGTH 512
#define MAX_LANDSCAPE_VERTICES 100000

// File information structure
typedef struct {
    char filepath[MAX_PATH_LENGTH];
    float x, y, z;  // Position in 3D space
    float height;   // Complexity height
    float color_value; // Value for coloring
    Color color;
} LandscapeFile;

// Landscape data
static bool landscape_initialized = false;
static Shader landscape_shader;
static GLuint landscape_vao, landscape_vbo, landscape_ebo;
static LandscapeFile *files = NULL;
static int num_files = 0;
static LandscapeConfig current_config;
static double metric_min = 0.0;
static double metric_max = 1.0;
static double color_metric_min = 0.0;
static double color_metric_max = 1.0;
static float *vertex_data = NULL;
static unsigned int *index_data = NULL;
static int num_vertices = 0;
static int num_indices = 0;

// Tooltip state
static int mouse_x = 0;
static int mouse_y = 0;
static int window_width = 800;
static int window_height = 600;
static bool show_tooltip = false;
static char tooltip_filepath[MAX_PATH_LENGTH] = "";
static double tooltip_metric_value = 0.0;

// Predefined gradients
static const Color complexity_colors[] = {
    {0.0, 0.5, 0.0, 1.0},     // Green (low complexity)
    {0.5, 0.5, 0.0, 1.0},     // Yellow (medium complexity)
    {0.5, 0.0, 0.0, 1.0},     // Red (high complexity)
    {0.3, 0.0, 0.3, 1.0}      // Purple (very high complexity)
};

static const Color heatmap_colors[] = {
    {0.0, 0.0, 0.5, 1.0},     // Blue (low)
    {0.0, 0.5, 0.5, 1.0},     // Cyan
    {0.0, 0.5, 0.0, 1.0},     // Green
    {0.5, 0.5, 0.0, 1.0},     // Yellow
    {0.5, 0.0, 0.0, 1.0},     // Red (high)
    {0.3, 0.0, 0.3, 1.0}      // Purple (very high)
};

CQError complexity_landscape_init(void)
{
    if (landscape_initialized) {
        return CQ_SUCCESS;
    }

    LOG_INFO("Initializing complexity landscape visualization system");

    // Create landscape shader
    const char *vertex_shader =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "layout (location = 2) in vec3 aNormal;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 FragPos;\n"
        "out vec3 ourColor;\n"
        "out vec3 Normal;\n"
        "void main()\n"
        "{\n"
        "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
        "    ourColor = aColor;\n"
        "    Normal = aNormal;\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}\0";

    const char *fragment_shader =
        "#version 330 core\n"
        "in vec3 FragPos;\n"
        "in vec3 ourColor;\n"
        "in vec3 Normal;\n"
        "uniform vec3 lightPos;\n"
        "uniform vec3 lightColor;\n"
        "uniform vec3 viewPos;\n"
        "uniform float ambientStrength;\n"
        "uniform float specularStrength;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    // Ambient\n"
        "    vec3 ambient = ambientStrength * lightColor;\n"
        "    \n"
        "    // Diffuse\n"
        "    vec3 norm = normalize(Normal);\n"
        "    vec3 lightDir = normalize(lightPos - FragPos);\n"
        "    float diff = max(dot(norm, lightDir), 0.0);\n"
        "    vec3 diffuse = diff * lightColor;\n"
        "    \n"
        "    // Specular\n"
        "    vec3 viewDir = normalize(viewPos - FragPos);\n"
        "    vec3 reflectDir = reflect(-lightDir, norm);\n"
        "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
        "    vec3 specular = specularStrength * spec * lightColor;\n"
        "    \n"
        "    vec3 result = (ambient + diffuse + specular) * ourColor;\n"
        "    FragColor = vec4(result, 1.0);\n"
        "}\0";

    if (shader_load_from_source(vertex_shader, fragment_shader, &landscape_shader) != CQ_SUCCESS) {
        LOG_ERROR("Failed to load landscape shader");
        return CQ_ERROR_UNKNOWN;
    }

    // Allocate file data
    files = (LandscapeFile *)malloc(MAX_FILES * sizeof(LandscapeFile));
    if (!files) {
        LOG_ERROR("Failed to allocate file data");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    // Set default configuration
    current_config.mode = LANDSCAPE_MODE_GRID;
    strcpy(current_config.metric_name, "cyclomatic_complexity");
    strcpy(current_config.color_metric, "cyclomatic_complexity");
    current_config.scale_factor = 1.0f;
    current_config.base_height = 0.1f;
    current_config.grid_resolution = 32;
    current_config.spacing = 2.0f;
    current_config.show_labels = true;
    current_config.show_grid = true;

    // Set default gradient
    complexity_landscape_set_gradient("complexity");

    landscape_initialized = true;
    LOG_INFO("Complexity landscape visualization system initialized successfully");
    return CQ_SUCCESS;
}

void complexity_landscape_shutdown(void)
{
    if (!landscape_initialized) {
        return;
    }

    LOG_INFO("Shutting down complexity landscape visualization system");

    if (files) {
        free(files);
        files = NULL;
    }

    if (vertex_data) {
        free(vertex_data);
        vertex_data = NULL;
    }

    if (index_data) {
        free(index_data);
        index_data = NULL;
    }

    // Clean up OpenGL resources
    glDeleteVertexArrays(1, &landscape_vao);
    glDeleteBuffers(1, &landscape_vbo);
    glDeleteBuffers(1, &landscape_ebo);

    landscape_initialized = false;
}

static CQError load_file_data(void)
{
    char filepaths[MAX_FILES][MAX_PATH_LENGTH];
    num_files = data_store_get_all_files(filepaths, MAX_FILES);

    if (num_files == 0) {
        LOG_WARNING("No files found in data store");
        return CQ_ERROR_UNKNOWN;
    }

    LOG_INFO("Loading data for %d files", num_files);

    // Load metric data for each file
    for (int i = 0; i < num_files; ++i) {
        strcpy(files[i].filepath, filepaths[i]);

        // Get height metric
        double height_value = data_store_get_metric(filepaths[i], current_config.metric_name);
        if (height_value < 0.0) {
            height_value = 0.0; // Default for missing data
        }
        files[i].height = height_value;

        // Get color metric
        double color_value = data_store_get_metric(filepaths[i], current_config.color_metric);
        if (color_value < 0.0) {
            color_value = 0.0; // Default for missing data
        }
        files[i].color_value = color_value;
    }

    // Calculate min/max values
    metric_min = DBL_MAX;
    metric_max = DBL_MIN;
    color_metric_min = DBL_MAX;
    color_metric_max = DBL_MIN;

    for (int i = 0; i < num_files; ++i) {
        if (files[i].height < metric_min) metric_min = files[i].height;
        if (files[i].height > metric_max) metric_max = files[i].height;
        if (files[i].color_value < color_metric_min) color_metric_min = files[i].color_value;
        if (files[i].color_value > color_metric_max) color_metric_max = files[i].color_value;
    }

    LOG_INFO("Metric range: %.2f - %.2f, Color range: %.2f - %.2f",
             metric_min, metric_max, color_metric_min, color_metric_max);

    return CQ_SUCCESS;
}

static void position_files_grid(void)
{
    int grid_size = (int)ceil(sqrt(num_files));
    float start_x = -(grid_size - 1) * current_config.spacing * 0.5f;
    float start_z = -(grid_size - 1) * current_config.spacing * 0.5f;

    for (int i = 0; i < num_files; ++i) {
        int row = i / grid_size;
        int col = i % grid_size;

        files[i].x = start_x + col * current_config.spacing;
        files[i].z = start_z + row * current_config.spacing;
        files[i].y = current_config.base_height + files[i].height * current_config.scale_factor;
    }
}

static void position_files_circular(void)
{
    float radius = current_config.spacing * sqrt(num_files) * 0.5f;

    for (int i = 0; i < num_files; ++i) {
        float angle = 2.0f * M_PI * i / num_files;
        files[i].x = radius * cosf(angle);
        files[i].z = radius * sinf(angle);
        files[i].y = current_config.base_height + files[i].height * current_config.scale_factor;
    }
}

static void position_files_hierarchical(void)
{
    // Simple hierarchical layout based on directory depth
    float base_spacing = current_config.spacing;

    for (int i = 0; i < num_files; ++i) {
        // Count directory depth
        int depth = 0;
        const char *path = files[i].filepath;
        for (int j = 0; path[j]; ++j) {
            if (path[j] == '/' || path[j] == '\\') {
                depth++;
            }
        }

        // Position based on depth and index
        files[i].x = (i % 10 - 5) * base_spacing;
        files[i].z = depth * base_spacing * 2.0f;
        files[i].y = current_config.base_height + files[i].height * current_config.scale_factor;
    }
}

static void position_files_scatter(void)
{
    // Random scatter plot positioning
    srand(42); // Fixed seed for reproducible layout

    for (int i = 0; i < num_files; ++i) {
        files[i].x = ((float)rand() / RAND_MAX - 0.5f) * current_config.spacing * 10.0f;
        files[i].z = ((float)rand() / RAND_MAX - 0.5f) * current_config.spacing * 10.0f;
        files[i].y = current_config.base_height + files[i].height * current_config.scale_factor;
    }
}

static void assign_colors(void)
{
    for (int i = 0; i < num_files; ++i) {
        // Normalize color value
        float normalized_value = 0.0f;
        if (color_metric_max > color_metric_min) {
            normalized_value = (files[i].color_value - color_metric_min) /
                              (color_metric_max - color_metric_min);
        }

        // Get color from gradient
        files[i].color = gradient_get_color(&current_config.gradient, normalized_value);
    }
}

static CQError generate_geometry(void)
{
    // For now, create simple cubes for each file
    // TODO: Create proper terrain mesh
    num_vertices = num_files * 24; // 24 vertices per cube (6 faces * 4 vertices)
    num_indices = num_files * 36;  // 36 indices per cube (6 faces * 6 indices)

    vertex_data = (float *)realloc(vertex_data, num_vertices * 9 * sizeof(float)); // pos(3) + color(3) + normal(3)
    index_data = (unsigned int *)realloc(index_data, num_indices * sizeof(unsigned int));

    if (!vertex_data || !index_data) {
        LOG_ERROR("Failed to allocate geometry data");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    int vertex_index = 0;
    int index_index = 0;

    for (int i = 0; i < num_files; ++i) {
        float x = files[i].x;
        float y = files[i].y;
        float z = files[i].z;
        float size = 0.5f; // Cube size

        // Cube vertices with position, color, and normal
        float cube_vertices[] = {
            // Front face
            x-size, y-size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, 0,0,1,
            x+size, y-size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, 0,0,1,
            x+size, y+size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, 0,0,1,
            x-size, y+size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, 0,0,1,
            // Back face
            x-size, y-size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, 0,0,-1,
            x+size, y-size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, 0,0,-1,
            x+size, y+size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, 0,0,-1,
            x-size, y+size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, 0,0,-1,
            // Left face
            x-size, y+size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, -1,0,0,
            x-size, y+size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, -1,0,0,
            x-size, y-size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, -1,0,0,
            x-size, y-size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, -1,0,0,
            // Right face
            x+size, y+size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, 1,0,0,
            x+size, y+size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, 1,0,0,
            x+size, y-size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, 1,0,0,
            x+size, y-size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, 1,0,0,
            // Top face
            x-size, y+size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, 0,1,0,
            x+size, y+size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, 0,1,0,
            x+size, y+size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, 0,1,0,
            x-size, y+size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, 0,1,0,
            // Bottom face
            x-size, y-size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, 0,-1,0,
            x+size, y-size, z-size, files[i].color.r, files[i].color.g, files[i].color.b, 0,-1,0,
            x+size, y-size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, 0,-1,0,
            x-size, y-size, z+size, files[i].color.r, files[i].color.g, files[i].color.b, 0,-1,0
        };

        // Copy vertices
        memcpy(&vertex_data[vertex_index], cube_vertices, sizeof(cube_vertices));
        vertex_index += 24 * 9;

        // Cube indices
        unsigned int base_index = i * 24;
        unsigned int cube_indices[] = {
            base_index+0, base_index+1, base_index+2, base_index+2, base_index+3, base_index+0,       // Front
            base_index+4, base_index+5, base_index+6, base_index+6, base_index+7, base_index+4,       // Back
            base_index+8, base_index+9, base_index+10, base_index+10, base_index+11, base_index+8,    // Left
            base_index+12, base_index+13, base_index+14, base_index+14, base_index+15, base_index+12, // Right
            base_index+16, base_index+17, base_index+18, base_index+18, base_index+19, base_index+16, // Top
            base_index+20, base_index+21, base_index+22, base_index+22, base_index+23, base_index+20  // Bottom
        };

        // Copy indices
        memcpy(&index_data[index_index], cube_indices, sizeof(cube_indices));
        index_index += 36;
    }

    return CQ_SUCCESS;
}

CQError complexity_landscape_create(const LandscapeConfig *config)
{
    if (!landscape_initialized) {
        return CQ_ERROR_UNKNOWN;
    }

    if (!config) {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    LOG_INFO("Creating complexity landscape with mode %d", config->mode);

    // Copy configuration
    current_config = *config;

    // Load file data
    CQError result = load_file_data();
    if (result != CQ_SUCCESS) {
        return result;
    }

    // Position files based on mode
    switch (current_config.mode) {
        case LANDSCAPE_MODE_GRID:
            position_files_grid();
            break;
        case LANDSCAPE_MODE_CIRCULAR:
            position_files_circular();
            break;
        case LANDSCAPE_MODE_HIERARCHICAL:
            position_files_hierarchical();
            break;
        case LANDSCAPE_MODE_SCATTER:
            position_files_scatter();
            break;
        default:
            LOG_ERROR("Unknown landscape mode: %d", current_config.mode);
            return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Assign colors
    assign_colors();

    // Generate geometry
    result = generate_geometry();
    if (result != CQ_SUCCESS) {
        return result;
    }

    // Create OpenGL buffers
    glGenVertexArrays(1, &landscape_vao);
    glGenBuffers(1, &landscape_vbo);
    glGenBuffers(1, &landscape_ebo);

    glBindVertexArray(landscape_vao);

    glBindBuffer(GL_ARRAY_BUFFER, landscape_vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * 9 * sizeof(float), vertex_data, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, landscape_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(unsigned int), index_data, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    LOG_INFO("Complexity landscape created successfully with %d files", num_files);
    return CQ_SUCCESS;
}

CQError complexity_landscape_update(const char *metric_name)
{
    if (!landscape_initialized) {
        return CQ_ERROR_UNKNOWN;
    }

    if (strcmp(current_config.metric_name, metric_name) != 0) {
        strcpy(current_config.metric_name, metric_name);
    }

    // Reload data and recreate landscape
    return complexity_landscape_create(&current_config);
}

void complexity_landscape_render(void)
{
    if (!landscape_initialized || num_vertices == 0) {
        return;
    }

    shader_use(&landscape_shader);

    // Set lighting uniforms
    shader_set_vec3(&landscape_shader, "lightPos", 10.0f, 10.0f, 10.0f);
    shader_set_vec3(&landscape_shader, "lightColor", 1.0f, 1.0f, 1.0f);
    shader_set_vec3(&landscape_shader, "viewPos", 0.0f, 5.0f, 10.0f);
    shader_set_float(&landscape_shader, "ambientStrength", 0.3f);
    shader_set_float(&landscape_shader, "specularStrength", 0.5f);

    // Render the landscape
    glBindVertexArray(landscape_vao);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Render labels if enabled
    if (current_config.show_labels) {
        for (int i = 0; i < num_files; ++i) {
            // Extract filename from path
            const char *filename = strrchr(files[i].filepath, '/');
            if (!filename) filename = strrchr(files[i].filepath, '\\');
            if (!filename) filename = files[i].filepath;
            else filename++; // Skip the separator

            char label[64];
            snprintf(label, sizeof(label), "%.1f", files[i].height);

            renderer_draw_text_3d(label, files[i].x, files[i].y + 0.5f, files[i].z,
                                 0.5f, &files[i].color);
        }
    }

    // Render grid if enabled
    if (current_config.show_grid) {
        glDisable(GL_DEPTH_TEST);
        for (int i = 0; i < num_files; ++i) {
            // Draw grid lines
            renderer_draw_line_color(files[i].x - 0.6f, current_config.base_height, files[i].z,
                                   files[i].x + 0.6f, current_config.base_height, files[i].z,
                                   &(Color){0.5f, 0.5f, 0.5f, 0.3f});
            renderer_draw_line_color(files[i].x, current_config.base_height, files[i].z - 0.6f,
                                   files[i].x, current_config.base_height, files[i].z + 0.6f,
                                   &(Color){0.5f, 0.5f, 0.5f, 0.3f});
        }
        glEnable(GL_DEPTH_TEST);
    }

    // Render tooltip if hovering over a file
    render_tooltip();

    // Render legend and scale indicators
    render_legend();
}

CQError complexity_landscape_set_mode(LandscapeMode mode)
{
    current_config.mode = mode;
    return complexity_landscape_create(&current_config);
}

CQError complexity_landscape_set_metric(const char *metric_name)
{
    strcpy(current_config.metric_name, metric_name);
    return complexity_landscape_create(&current_config);
}

CQError complexity_landscape_set_color_metric(const char *metric_name)
{
    strcpy(current_config.color_metric, metric_name);
    return complexity_landscape_create(&current_config);
}

void complexity_landscape_set_scale(float scale)
{
    current_config.scale_factor = scale;
    // Update heights without recreating geometry
    for (int i = 0; i < num_files; ++i) {
        files[i].y = current_config.base_height + files[i].height * current_config.scale_factor;
    }
    // TODO: Update vertex buffer
}

CQError complexity_landscape_set_gradient(const char *gradient_name)
{
    if (!gradient_name) {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (strcmp(gradient_name, "complexity") == 0) {
        gradient_create(&current_config.gradient, complexity_colors,
                       sizeof(complexity_colors) / sizeof(Color));
    } else if (strcmp(gradient_name, "heatmap") == 0) {
        gradient_create(&current_config.gradient, heatmap_colors,
                       sizeof(heatmap_colors) / sizeof(Color));
    } else {
        LOG_WARNING("Unknown gradient '%s', using complexity", gradient_name);
        gradient_create(&current_config.gradient, complexity_colors,
                       sizeof(complexity_colors) / sizeof(Color));
    }

    // Update colors
    assign_colors();
    // TODO: Update vertex buffer

    LOG_INFO("Set landscape gradient to '%s'", gradient_name);
    return CQ_SUCCESS;
}

CQError complexity_landscape_set_custom_gradient(const Color *colors, int num_colors)
{
    if (!colors || num_colors < 2) {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    gradient_create(&current_config.gradient, colors, num_colors);

    // Update colors
    assign_colors();
    // TODO: Update vertex buffer

    LOG_INFO("Set custom landscape gradient with %d colors", num_colors);
    return CQ_SUCCESS;
}

void complexity_landscape_show_labels(bool enabled)
{
    current_config.show_labels = enabled;
}

void complexity_landscape_show_grid(bool enabled)
{
    current_config.show_grid = enabled;
}

void complexity_landscape_set_window_size(int width, int height)
{
    window_width = width;
    window_height = height;
}

static void render_tooltip(void)
{
    if (!show_tooltip || !text_renderer_initialized) {
        return;
    }

    // Extract filename from path
    const char *filename = strrchr(tooltip_filepath, '/');
    if (!filename) filename = strrchr(tooltip_filepath, '\\');
    if (!filename) filename = tooltip_filepath;
    else filename++; // Skip the separator

    // Create tooltip text
    char tooltip_text[256];
    snprintf(tooltip_text, sizeof(tooltip_text), "%s\nComplexity: %.2f",
             filename, tooltip_metric_value);

    // Render tooltip at mouse position (with some offset)
    Color text_color = {1.0f, 1.0f, 1.0f, 1.0f};
    renderer_draw_text(tooltip_text, mouse_x + 10, mouse_y - 10, 0.5f, &text_color);
}

static void render_legend(void)
{
    if (!text_renderer_initialized) {
        return;
    }

    // Position legend in bottom-right corner
    int legend_x = window_width - 200;
    int legend_y = window_height - 150;
    int legend_width = 180;
    int legend_height = 120;

    // Draw legend background (semi-transparent)
    // Note: This would require additional rendering functions for rectangles

    // Draw color gradient bar
    float bar_height = 20.0f;
    float bar_width = 150.0f;
    for (int i = 0; i < (int)bar_width; ++i) {
        float t = (float)i / bar_width;
        Color color = gradient_get_color(&current_config.gradient, t);
        renderer_draw_line_color(legend_x + i, legend_y + 10,
                               legend_x + i, legend_y + 10 + bar_height,
                               &color);
    }

    // Draw scale labels
    Color text_color = {1.0f, 1.0f, 1.0f, 1.0f};
    char min_text[32];
    char max_text[32];
    char metric_text[64];

    snprintf(min_text, sizeof(min_text), "%.1f", color_metric_min);
    snprintf(max_text, sizeof(max_text), "%.1f", color_metric_max);
    snprintf(metric_text, sizeof(metric_text), "%s", current_config.color_metric);

    renderer_draw_text(min_text, legend_x, legend_y + 35, 0.4f, &text_color);
    renderer_draw_text(max_text, legend_x + bar_width - 30, legend_y + 35, 0.4f, &text_color);
    renderer_draw_text(metric_text, legend_x, legend_y + 55, 0.5f, &text_color);

    // Draw mode information
    char mode_text[32];
    switch (current_config.mode) {
        case LANDSCAPE_MODE_GRID:
            strcpy(mode_text, "Grid Mode");
            break;
        case LANDSCAPE_MODE_CIRCULAR:
            strcpy(mode_text, "Circular Mode");
            break;
        case LANDSCAPE_MODE_HIERARCHICAL:
            strcpy(mode_text, "Hierarchical Mode");
            break;
        case LANDSCAPE_MODE_SCATTER:
            strcpy(mode_text, "Scatter Mode");
            break;
        default:
            strcpy(mode_text, "Unknown Mode");
    }
    renderer_draw_text(mode_text, legend_x, legend_y + 75, 0.4f, &text_color);

    // Draw file count
    char count_text[32];
    snprintf(count_text, sizeof(count_text), "Files: %d", num_files);
    renderer_draw_text(count_text, legend_x, legend_y + 95, 0.4f, &text_color);
}

void complexity_landscape_update_mouse_position(int screen_x, int screen_y)
{
    mouse_x = screen_x;
    mouse_y = screen_y;

    // Update tooltip
    show_tooltip = complexity_landscape_get_tooltip(screen_x, screen_y,
                                                   tooltip_filepath, &tooltip_metric_value);
}

bool complexity_landscape_get_tooltip(int screen_x, int screen_y, char *filepath, double *metric_value)
{
    if (!landscape_initialized || !filepath || !metric_value) {
        return false;
    }

    // Convert screen coordinates to normalized device coordinates
    float x = (2.0f * screen_x) / window_width - 1.0f;
    float y = 1.0f - (2.0f * screen_y) / window_height;

    // Create ray from camera through the screen point
    // This is a simplified implementation - in a real system you'd use proper ray casting
    float aspect = (float)window_width / (float)window_height;

    // Camera position and direction (simplified)
    float cam_x = 0.0f, cam_y = 5.0f, cam_z = 10.0f;
    float ray_dir_x = x * aspect;
    float ray_dir_y = y;
    float ray_dir_z = -1.0f; // Looking towards negative Z

    // Normalize ray direction
    float length = sqrtf(ray_dir_x * ray_dir_x + ray_dir_y * ray_dir_y + ray_dir_z * ray_dir_z);
    ray_dir_x /= length;
    ray_dir_y /= length;
    ray_dir_z /= length;

    // Find closest file intersection
    float closest_distance = FLT_MAX;
    int closest_file = -1;

    for (int i = 0; i < num_files; ++i) {
        // Simple distance-based intersection (not accurate ray-box intersection)
        float dx = files[i].x - cam_x;
        float dy = files[i].y - cam_y;
        float dz = files[i].z - cam_z;

        // Project point onto ray
        float t = (dx * ray_dir_x + dy * ray_dir_y + dz * ray_dir_z);
        if (t < 0) continue; // Behind camera

        float proj_x = cam_x + t * ray_dir_x;
        float proj_y = cam_y + t * ray_dir_y;
        float proj_z = cam_z + t * ray_dir_z;

        // Distance from projected point to file position
        float dist_x = proj_x - files[i].x;
        float dist_y = proj_y - files[i].y;
        float dist_z = proj_z - files[i].z;
        float distance = sqrtf(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);

        // Check if within file "bounds" (1.0 unit cube)
        if (distance < 1.0f && t < closest_distance) {
            closest_distance = t;
            closest_file = i;
        }
    }

    if (closest_file >= 0) {
        strcpy(filepath, files[closest_file].filepath);
        *metric_value = files[closest_file].height;
        return true;
    }

    return false;
}

CQError complexity_landscape_get_legend(double *min_value, double *max_value, Gradient *gradient)
{
    if (!min_value || !max_value || !gradient) {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    *min_value = metric_min;
    *max_value = metric_max;
    *gradient = current_config.gradient;

    return CQ_SUCCESS;
}