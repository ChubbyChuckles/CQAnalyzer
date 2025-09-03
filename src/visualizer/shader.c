#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "visualizer/shader.h"
#include "utils/logger.h"

CQError shader_load_from_source(const char *vertex_source, const char *fragment_source, Shader *shader)
{
    if (!vertex_source || !fragment_source || !shader)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // TODO: Compile vertex shader
    // TODO: Compile fragment shader
    // TODO: Link shader program
    // TODO: Check for compilation/linking errors

    LOG_WARNING("Shader loading from source not yet implemented");
    shader->program_id = 0;
    shader->is_loaded = false;

    return CQ_ERROR_UNKNOWN;
}

CQError shader_load_from_files(const char *vertex_file, const char *fragment_file, Shader *shader)
{
    if (!vertex_file || !fragment_file || !shader)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // TODO: Read vertex shader file
    // TODO: Read fragment shader file
    // TODO: Call shader_load_from_source

    LOG_WARNING("Shader loading from files not yet implemented");
    shader->program_id = 0;
    shader->is_loaded = false;

    return CQ_ERROR_UNKNOWN;
}

void shader_use(const Shader *shader)
{
    if (!shader || !shader->is_loaded)
    {
        LOG_WARNING("Cannot use invalid or unloaded shader");
        return;
    }

    // TODO: Use OpenGL glUseProgram
    LOG_WARNING("Shader use not yet implemented");
}

void shader_delete(Shader *shader)
{
    if (!shader)
    {
        return;
    }

    if (shader->is_loaded)
    {
        // TODO: Delete OpenGL shader program
        LOG_WARNING("Shader deletion not yet implemented");
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

    // TODO: Set OpenGL uniform float value
    LOG_WARNING("Shader set float not yet implemented");
}

void shader_set_vec3(const Shader *shader, const char *name, float x, float y, float z)
{
    if (!shader || !shader->is_loaded || !name)
    {
        return;
    }

    // TODO: Set OpenGL uniform vec3 value
    LOG_WARNING("Shader set vec3 not yet implemented");
}
