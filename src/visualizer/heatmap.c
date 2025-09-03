#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "visualizer/heatmap.h"
#include "visualizer/shader.h"
#include "visualizer/color.h"
#include "visualizer/gradient.h"
#include "data/data_store.h"
#include "data/metric_aggregator.h"
#include "utils/logger.h"

#define MAX_HEATMAP_RESOLUTION 256
#define MAX_FILES 10000

static bool heatmap_initialized = false;
static Shader heatmap_shader;
static GLuint heatmap_vao, heatmap_vbo, heatmap_ebo;
static int current_resolution = 64;
static float *height_data = NULL;
static float *vertex_data = NULL;
static unsigned int *index_data = NULL;
static int num_vertices = 0;
static int num_indices = 0;
static char current_metric[256] = "";
static char current_surface[32] = "plane";
static Gradient current_gradient;
static float heatmap_opacity = 1.0f;
static float heatmap_scale = 1.0f;
static double metric_min = 0.0;
static double metric_max = 1.0;

// Predefined gradients
static const Color viridis_colors[] = {
    {0.267, 0.004, 0.329, 1.0}, {0.278, 0.016, 0.420, 1.0}, {0.285, 0.032, 0.512, 1.0},
    {0.286, 0.059, 0.600, 1.0}, {0.282, 0.086, 0.686, 1.0}, {0.268, 0.114, 0.765, 1.0},
    {0.244, 0.143, 0.831, 1.0}, {0.208, 0.173, 0.878, 1.0}, {0.161, 0.204, 0.894, 1.0},
    {0.106, 0.237, 0.882, 1.0}, {0.055, 0.271, 0.843, 1.0}, {0.016, 0.306, 0.781, 1.0},
    {0.004, 0.337, 0.702, 1.0}, {0.020, 0.364, 0.612, 1.0}, {0.055, 0.388, 0.518, 1.0},
    {0.102, 0.408, 0.427, 1.0}, {0.161, 0.427, 0.341, 1.0}, {0.227, 0.447, 0.259, 1.0},
    {0.298, 0.467, 0.184, 1.0}, {0.374, 0.490, 0.118, 1.0}, {0.455, 0.514, 0.061, 1.0},
    {0.541, 0.541, 0.020, 1.0}, {0.631, 0.569, 0.012, 1.0}, {0.725, 0.600, 0.043, 1.0},
    {0.820, 0.635, 0.102, 1.0}, {0.914, 0.675, 0.184, 1.0}, {0.992, 0.722, 0.388, 1.0}
};

CQError heatmap_init(void)
{
    if (heatmap_initialized)
    {
        return CQ_SUCCESS;
    }

    LOG_INFO("Initializing heatmap visualization system");

    // Create heatmap shader
    const char *vertex_shader =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aNormal;\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 FragPos;\n"
        "out vec3 Normal;\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
        "    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
        "    TexCoord = aTexCoord;\n"
        "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
        "}\0";

    const char *fragment_shader =
        "#version 330 core\n"
        "in vec3 FragPos;\n"
        "in vec3 Normal;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D heightMap;\n"
        "uniform vec3 gradientColors[32];\n"
        "uniform int numGradientColors;\n"
        "uniform float opacity;\n"
        "uniform float minValue;\n"
        "uniform float maxValue;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    float height = texture(heightMap, TexCoord).r;\n"
        "    float normalizedValue = (height - minValue) / (maxValue - minValue);\n"
        "    normalizedValue = clamp(normalizedValue, 0.0, 1.0);\n"
        "    \n"
        "    // Interpolate gradient color\n"
        "    int index = int(normalizedValue * (numGradientColors - 1));\n"
        "    index = clamp(index, 0, numGradientColors - 2);\n"
        "    float t = fract(normalizedValue * (numGradientColors - 1));\n"
        "    vec3 color1 = gradientColors[index];\n"
        "    vec3 color2 = gradientColors[index + 1];\n"
        "    vec3 finalColor = mix(color1, color2, t);\n"
        "    \n"
        "    FragColor = vec4(finalColor, opacity);\n"
        "}\0";

    if (shader_load_from_source(vertex_shader, fragment_shader, &heatmap_shader) != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to load heatmap shader");
        return CQ_ERROR_UNKNOWN;
    }

    // Set default gradient
    heatmap_set_gradient("viridis");

    // Allocate data arrays
    height_data = (float *)malloc(MAX_HEATMAP_RESOLUTION * MAX_HEATMAP_RESOLUTION * sizeof(float));
    if (!height_data)
    {
        LOG_ERROR("Failed to allocate height data");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    heatmap_initialized = true;
    LOG_INFO("Heatmap visualization system initialized successfully");
    return CQ_SUCCESS;
}

void heatmap_shutdown(void)
{
    if (!heatmap_initialized)
    {
        return;
    }

    LOG_INFO("Shutting down heatmap visualization system");

    if (height_data)
    {
        free(height_data);
        height_data = NULL;
    }

    if (vertex_data)
    {
        free(vertex_data);
        vertex_data = NULL;
    }

    if (index_data)
    {
        free(index_data);
        index_data = NULL;
    }

    // Clean up OpenGL resources
    glDeleteVertexArrays(1, &heatmap_vao);
    glDeleteBuffers(1, &heatmap_vbo);
    glDeleteBuffers(1, &heatmap_ebo);

    heatmap_initialized = false;
}

static CQError generate_plane_surface(int resolution)
{
    int vertices_per_side = resolution + 1;
    num_vertices = vertices_per_side * vertices_per_side;
    num_indices = resolution * resolution * 6;

    vertex_data = (float *)realloc(vertex_data, num_vertices * 8 * sizeof(float)); // pos(3) + normal(3) + texcoord(2)
    index_data = (unsigned int *)realloc(index_data, num_indices * sizeof(unsigned int));

    if (!vertex_data || !index_data)
    {
        LOG_ERROR("Failed to allocate surface data");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    float size = 10.0f;
    float step = size / resolution;

    int vertex_index = 0;
    for (int i = 0; i <= resolution; ++i)
    {
        for (int j = 0; j <= resolution; ++j)
        {
            float x = -size/2 + j * step;
            float z = -size/2 + i * step;
            float y = 0.0f;

            // Position
            vertex_data[vertex_index++] = x;
            vertex_data[vertex_index++] = y;
            vertex_data[vertex_index++] = z;

            // Normal (up)
            vertex_data[vertex_index++] = 0.0f;
            vertex_data[vertex_index++] = 1.0f;
            vertex_data[vertex_index++] = 0.0f;

            // Texture coordinates
            vertex_data[vertex_index++] = (float)j / resolution;
            vertex_data[vertex_index++] = (float)i / resolution;
        }
    }

    int index_index = 0;
    for (int i = 0; i < resolution; ++i)
    {
        for (int j = 0; j < resolution; ++j)
        {
            int top_left = i * (resolution + 1) + j;
            int top_right = top_left + 1;
            int bottom_left = (i + 1) * (resolution + 1) + j;
            int bottom_right = bottom_left + 1;

            // First triangle
            index_data[index_index++] = top_left;
            index_data[index_index++] = bottom_left;
            index_data[index_index++] = top_right;

            // Second triangle
            index_data[index_index++] = top_right;
            index_data[index_index++] = bottom_left;
            index_data[index_index++] = bottom_right;
        }
    }

    return CQ_SUCCESS;
}

static CQError generate_height_map(const char *metric_name)
{
    // Get all metric values
    double values[MAX_FILES];
    int num_values = data_store_get_all_metric_values(metric_name, values, MAX_FILES);

    if (num_values == 0)
    {
        LOG_WARNING("No metric values found for %s", metric_name);
        return CQ_ERROR_UNKNOWN;
    }

    // Calculate min/max
    metric_min = values[0];
    metric_max = values[0];
    for (int i = 1; i < num_values; ++i)
    {
        if (values[i] < metric_min) metric_min = values[i];
        if (values[i] > metric_max) metric_max = values[i];
    }

    // Create height map from metric data
    int data_index = 0;
    for (int i = 0; i < current_resolution; ++i)
    {
        for (int j = 0; j < current_resolution; ++j)
        {
            if (data_index < num_values)
            {
                // Normalize to 0-1 range
                height_data[i * current_resolution + j] = (values[data_index] - metric_min) / (metric_max - metric_min);
            }
            else
            {
                height_data[i * current_resolution + j] = 0.0f;
            }
            data_index++;
        }
    }

    LOG_INFO("Generated height map for %s with %d values (min: %.2f, max: %.2f)",
             metric_name, num_values, metric_min, metric_max);
    return CQ_SUCCESS;
}

CQError heatmap_create(const char *metric_name, const char *surface_type, int resolution)
{
    if (!heatmap_initialized)
    {
        return CQ_ERROR_UNKNOWN;
    }

    if (!metric_name || !surface_type)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (resolution < 2 || resolution > MAX_HEATMAP_RESOLUTION)
    {
        LOG_ERROR("Invalid resolution: %d (must be 2-%d)", resolution, MAX_HEATMAP_RESOLUTION);
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    LOG_INFO("Creating heatmap for metric '%s' on %s surface with resolution %dx%d",
             metric_name, surface_type, resolution, resolution);

    current_resolution = resolution;
    strcpy(current_metric, metric_name);
    strcpy(current_surface, surface_type);

    // Generate surface geometry
    CQError result = generate_plane_surface(resolution);
    if (result != CQ_SUCCESS)
    {
        return result;
    }

    // Generate height map data
    result = generate_height_map(metric_name);
    if (result != CQ_SUCCESS)
    {
        return result;
    }

    // Create OpenGL buffers
    glGenVertexArrays(1, &heatmap_vao);
    glGenBuffers(1, &heatmap_vbo);
    glGenBuffers(1, &heatmap_ebo);

    glBindVertexArray(heatmap_vao);

    glBindBuffer(GL_ARRAY_BUFFER, heatmap_vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * 8 * sizeof(float), vertex_data, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, heatmap_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(unsigned int), index_data, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    LOG_INFO("Heatmap created successfully");
    return CQ_SUCCESS;
}

CQError heatmap_update(const char *metric_name)
{
    if (!heatmap_initialized)
    {
        return CQ_ERROR_UNKNOWN;
    }

    if (strcmp(current_metric, metric_name) != 0)
    {
        strcpy(current_metric, metric_name);
    }

    return generate_height_map(metric_name);
}

void heatmap_render(void)
{
    if (!heatmap_initialized || num_vertices == 0)
    {
        return;
    }

    shader_use(&heatmap_shader);

    // Set uniforms
    shader_set_float(&heatmap_shader, "opacity", heatmap_opacity);
    shader_set_float(&heatmap_shader, "minValue", metric_min);
    shader_set_float(&heatmap_shader, "maxValue", metric_max);

    // Set gradient colors
    for (int i = 0; i < current_gradient.num_colors; ++i)
    {
        char uniform_name[32];
        sprintf(uniform_name, "gradientColors[%d]", i);
        glUniform3f(glGetUniformLocation(heatmap_shader.program_id, uniform_name),
                   current_gradient.colors[i].r,
                   current_gradient.colors[i].g,
                   current_gradient.colors[i].b);
    }
    shader_set_int(&heatmap_shader, "numGradientColors", current_gradient.num_colors);

    // Create and bind height map texture
    GLuint height_texture;
    glGenTextures(1, &height_texture);
    glBindTexture(GL_TEXTURE_2D, height_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, current_resolution, current_resolution, 0,
                 GL_RED, GL_FLOAT, height_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, height_texture);
    shader_set_int(&heatmap_shader, "heightMap", 0);

    // Render the surface
    glBindVertexArray(heatmap_vao);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Clean up texture
    glDeleteTextures(1, &height_texture);
}

CQError heatmap_set_gradient(const char *gradient_name)
{
    if (!gradient_name)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (strcmp(gradient_name, "viridis") == 0)
    {
        gradient_create(&current_gradient, viridis_colors, sizeof(viridis_colors) / sizeof(Color));
    }
    else
    {
        LOG_WARNING("Unknown gradient '%s', using viridis", gradient_name);
        gradient_create(&current_gradient, viridis_colors, sizeof(viridis_colors) / sizeof(Color));
    }

    LOG_INFO("Set heatmap gradient to '%s'", gradient_name);
    return CQ_SUCCESS;
}

CQError heatmap_set_custom_gradient(const Color *colors, int num_colors)
{
    if (!colors || num_colors < 2)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    gradient_create(&current_gradient, colors, num_colors);
    LOG_INFO("Set custom heatmap gradient with %d colors", num_colors);
    return CQ_SUCCESS;
}

void heatmap_set_opacity(float opacity)
{
    heatmap_opacity = opacity;
    if (heatmap_opacity < 0.0f) heatmap_opacity = 0.0f;
    if (heatmap_opacity > 1.0f) heatmap_opacity = 1.0f;
}

void heatmap_set_scale(float scale)
{
    heatmap_scale = scale;
}