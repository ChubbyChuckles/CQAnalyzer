#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include "cqanalyzer.h"

/**
 * @file shader.h
 * @brief OpenGL shader management
 *
 * Provides functions to load, compile, and manage OpenGL shaders.
 */

typedef struct
{
    unsigned int program_id;
    bool is_loaded;
} Shader;

/**
 * @brief Load shader from source code
 *
 * @param vertex_source Vertex shader source code
 * @param fragment_source Fragment shader source code
 * @param shader Output shader structure
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError shader_load_from_source(const char *vertex_source, const char *fragment_source, Shader *shader);

/**
 * @brief Load shader from files
 *
 * @param vertex_file Vertex shader file path
 * @param fragment_file Fragment shader file path
 * @param shader Output shader structure
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError shader_load_from_files(const char *vertex_file, const char *fragment_file, Shader *shader);

/**
 * @brief Use shader program
 *
 * @param shader Shader to use
 */
void shader_use(const Shader *shader);

/**
 * @brief Delete shader program
 *
 * @param shader Shader to delete
 */
void shader_delete(Shader *shader);

/**
 * @brief Set uniform float value
 *
 * @param shader Shader program
 * @param name Uniform name
 * @param value Float value
 */
void shader_set_float(const Shader *shader, const char *name, float value);

/**
 * @brief Set uniform vec3 value
 *
 * @param shader Shader program
 * @param name Uniform name
 * @param x X component
 * @param y Y component
 * @param z Z component
 */
void shader_set_vec3(const Shader *shader, const char *name, float x, float y, float z);

/**
 * @brief Set uniform vec4 value
 *
 * @param shader Shader program
 * @param name Uniform name
 * @param x X component
 * @param y Y component
 * @param z Z component
 * @param w W component
 */
void shader_set_vec4(const Shader *shader, const char *name, float x, float y, float z, float w);

/**
 * @brief Set uniform matrix 4x4 value
 *
 * @param shader Shader program
 * @param name Uniform name
 * @param matrix Matrix data (column-major)
 */
void shader_set_mat4(const Shader *shader, const char *name, const float matrix[16]);

/**
 * @brief Set uniform int value
 *
 * @param shader Shader program
 * @param name Uniform name
 * @param value Integer value
 */
void shader_set_int(const Shader *shader, const char *name, int value);

#endif // SHADER_H
