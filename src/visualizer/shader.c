#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "visualizer/shader.h"
#include "utils/logger.h"

CQError shader_load_from_source(const char *vertex_source, const char *fragment_source, Shader *shader)
{
    if (!vertex_source || !fragment_source || !shader)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    GLuint vertex_shader, fragment_shader;
    GLint success;
    GLchar info_log[512];

    // Create vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source, NULL);
    glCompileShader(vertex_shader);

    // Check vertex shader compilation
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        LOG_ERROR("Vertex shader compilation failed: %s", info_log);
        glDeleteShader(vertex_shader);
        return CQ_ERROR_UNKNOWN;
    }

    // Create fragment shader
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader);

    // Check fragment shader compilation
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        LOG_ERROR("Fragment shader compilation failed: %s", info_log);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return CQ_ERROR_UNKNOWN;
    }

    // Create shader program
    shader->program_id = glCreateProgram();
    glAttachShader(shader->program_id, vertex_shader);
    glAttachShader(shader->program_id, fragment_shader);
    glLinkProgram(shader->program_id);

    // Check program linking
    glGetProgramiv(shader->program_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader->program_id, 512, NULL, info_log);
        LOG_ERROR("Shader program linking failed: %s", info_log);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(shader->program_id);
        shader->program_id = 0;
        return CQ_ERROR_UNKNOWN;
    }

    // Clean up shaders (they're linked into the program now)
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    shader->is_loaded = true;
    LOG_INFO("Shader program loaded successfully (ID: %u)", shader->program_id);
    return CQ_SUCCESS;
}

CQError shader_load_from_files(const char *vertex_file, const char *fragment_file, Shader *shader)
{
    if (!vertex_file || !fragment_file || !shader)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    FILE *vertex_fp = NULL;
    FILE *fragment_fp = NULL;
    char *vertex_source = NULL;
    char *fragment_source = NULL;
    long vertex_size = 0;
    long fragment_size = 0;
    CQError result = CQ_ERROR_UNKNOWN;

    // Open vertex shader file
    vertex_fp = fopen(vertex_file, "r");
    if (!vertex_fp)
    {
        LOG_ERROR("Failed to open vertex shader file: %s", vertex_file);
        return CQ_ERROR_UNKNOWN;
    }

    // Open fragment shader file
    fragment_fp = fopen(fragment_file, "r");
    if (!fragment_fp)
    {
        LOG_ERROR("Failed to open fragment shader file: %s", fragment_file);
        fclose(vertex_fp);
        return CQ_ERROR_UNKNOWN;
    }

    // Get file sizes
    fseek(vertex_fp, 0, SEEK_END);
    vertex_size = ftell(vertex_fp);
    fseek(vertex_fp, 0, SEEK_SET);

    fseek(fragment_fp, 0, SEEK_END);
    fragment_size = ftell(fragment_fp);
    fseek(fragment_fp, 0, SEEK_SET);

    // Allocate memory for shader sources
    vertex_source = (char *)malloc(vertex_size + 1);
    fragment_source = (char *)malloc(fragment_size + 1);

    if (!vertex_source || !fragment_source)
    {
        LOG_ERROR("Failed to allocate memory for shader sources");
        result = CQ_ERROR_MEMORY_ALLOCATION;
        goto cleanup;
    }

    // Read shader files
    if (fread(vertex_source, 1, vertex_size, vertex_fp) != (size_t)vertex_size)
    {
        LOG_ERROR("Failed to read vertex shader file");
        goto cleanup;
    }
    vertex_source[vertex_size] = '\0';

    if (fread(fragment_source, 1, fragment_size, fragment_fp) != (size_t)fragment_size)
    {
        LOG_ERROR("Failed to read fragment shader file");
        goto cleanup;
    }
    fragment_source[fragment_size] = '\0';

    // Load shader from source
    result = shader_load_from_source(vertex_source, fragment_source, shader);

cleanup:
    if (vertex_source) free(vertex_source);
    if (fragment_source) free(fragment_source);
    if (vertex_fp) fclose(vertex_fp);
    if (fragment_fp) fclose(fragment_fp);

    return result;
}

void shader_use(const Shader *shader)
{
    if (!shader || !shader->is_loaded)
    {
        LOG_WARNING("Cannot use invalid or unloaded shader");
        return;
    }

    glUseProgram(shader->program_id);
}

void shader_delete(Shader *shader)
{
    if (!shader)
    {
        return;
    }

    if (shader->is_loaded)
    {
        glDeleteProgram(shader->program_id);
        LOG_INFO("Shader program deleted (ID: %u)", shader->program_id);
    }

    shader->program_id = 0;
    shader->is_loaded = false;
}

void shader_set_float(const Shader *shader, const char *name, float value)
{
    if (!shader || !shader->is_loaded || !name)
    {
        return;
    }

    GLint location = glGetUniformLocation(shader->program_id, name);
    if (location != -1)
    {
        glUniform1f(location, value);
    }
}

void shader_set_vec3(const Shader *shader, const char *name, float x, float y, float z)
{
    if (!shader || !shader->is_loaded || !name)
    {
        return;
    }

    GLint location = glGetUniformLocation(shader->program_id, name);
    if (location != -1)
    {
        glUniform3f(location, x, y, z);
    }
}

void shader_set_vec4(const Shader *shader, const char *name, float x, float y, float z, float w)
{
    if (!shader || !shader->is_loaded || !name)
    {
        return;
    }

    GLint location = glGetUniformLocation(shader->program_id, name);
    if (location != -1)
    {
        glUniform4f(location, x, y, z, w);
    }
}

void shader_set_mat4(const Shader *shader, const char *name, const float matrix[16])
{
    if (!shader || !shader->is_loaded || !name || !matrix)
    {
        return;
    }

    GLint location = glGetUniformLocation(shader->program_id, name);
    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
    }
}

void shader_set_int(const Shader *shader, const char *name, int value)
{
    if (!shader || !shader->is_loaded || !name)
    {
        return;
    }

    GLint location = glGetUniformLocation(shader->program_id, name);
    if (location != -1)
    {
        glUniform1i(location, value);
    }
}
