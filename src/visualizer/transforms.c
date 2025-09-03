#include <math.h>
#include <string.h>

#include "visualizer/transforms.h"
#include "utils/logger.h"

void matrix_multiply(float result[16], const float a[16], const float b[16])
{
    if (!result || !a || !b)
    {
        LOG_ERROR("Invalid matrix pointers");
        return;
    }

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result[i + j * 4] = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                result[i + j * 4] += a[i + k * 4] * b[k + j * 4];
            }
        }
    }
}

void transform_vector(float result[3], const float matrix[16], const float vector[3])
{
    if (!result || !matrix || !vector)
    {
        LOG_ERROR("Invalid pointers");
        return;
    }

    float temp[4] = {vector[0], vector[1], vector[2], 1.0f};
    float transformed[4];

    transform_vector4(transformed, matrix, temp);

    // Perspective divide
    if (transformed[3] != 0.0f)
    {
        result[0] = transformed[0] / transformed[3];
        result[1] = transformed[1] / transformed[3];
        result[2] = transformed[2] / transformed[3];
    }
    else
    {
        result[0] = transformed[0];
        result[1] = transformed[1];
        result[2] = transformed[2];
    }
}

void transform_vector4(float result[4], const float matrix[16], const float vector[4])
{
    if (!result || !matrix || !vector)
    {
        LOG_ERROR("Invalid pointers");
        return;
    }

    for (int i = 0; i < 4; ++i)
    {
        result[i] = 0.0f;
        for (int j = 0; j < 4; ++j)
        {
            result[i] += matrix[i + j * 4] * vector[j];
        }
    }
}

void matrix_identity(float matrix[16])
{
    if (!matrix)
    {
        LOG_ERROR("Invalid matrix pointer");
        return;
    }

    memset(matrix, 0, 16 * sizeof(float));
    matrix[0] = 1.0f;
    matrix[5] = 1.0f;
    matrix[10] = 1.0f;
    matrix[15] = 1.0f;
}

void matrix_translate(float matrix[16], float x, float y, float z)
{
    if (!matrix)
    {
        LOG_ERROR("Invalid matrix pointer");
        return;
    }

    matrix_identity(matrix);
    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;
}

void matrix_rotate_x(float matrix[16], float angle)
{
    if (!matrix)
    {
        LOG_ERROR("Invalid matrix pointer");
        return;
    }

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    matrix_identity(matrix);
    matrix[5] = cos_a;
    matrix[6] = sin_a;
    matrix[9] = -sin_a;
    matrix[10] = cos_a;
}

void matrix_rotate_y(float matrix[16], float angle)
{
    if (!matrix)
    {
        LOG_ERROR("Invalid matrix pointer");
        return;
    }

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    matrix_identity(matrix);
    matrix[0] = cos_a;
    matrix[2] = -sin_a;
    matrix[8] = sin_a;
    matrix[10] = cos_a;
}

void matrix_rotate_z(float matrix[16], float angle)
{
    if (!matrix)
    {
        LOG_ERROR("Invalid matrix pointer");
        return;
    }

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    matrix_identity(matrix);
    matrix[0] = cos_a;
    matrix[1] = sin_a;
    matrix[4] = -sin_a;
    matrix[5] = cos_a;
}

void matrix_scale(float matrix[16], float x, float y, float z)
{
    if (!matrix)
    {
        LOG_ERROR("Invalid matrix pointer");
        return;
    }

    matrix_identity(matrix);
    matrix[0] = x;
    matrix[5] = y;
    matrix[10] = z;
}

void create_model_matrix(float matrix[16],
                        float translate_x, float translate_y, float translate_z,
                        float rotate_x, float rotate_y, float rotate_z,
                        float scale_x, float scale_y, float scale_z)
{
    if (!matrix)
    {
        LOG_ERROR("Invalid matrix pointer");
        return;
    }

    float temp[16];
    float result[16];

    // Start with identity
    matrix_identity(result);

    // Apply scale
    matrix_scale(temp, scale_x, scale_y, scale_z);
    matrix_multiply(result, result, temp);

    // Apply rotations
    matrix_rotate_x(temp, rotate_x);
    matrix_multiply(result, result, temp);

    matrix_rotate_y(temp, rotate_y);
    matrix_multiply(result, result, temp);

    matrix_rotate_z(temp, rotate_z);
    matrix_multiply(result, result, temp);

    // Apply translation
    matrix_translate(temp, translate_x, translate_y, translate_z);
    matrix_multiply(result, result, temp);

    memcpy(matrix, result, 16 * sizeof(float));
}

void world_to_screen(float screen_coords[4],
                    const float world_coords[3],
                    const float model_matrix[16],
                    const float view_matrix[16],
                    const float projection_matrix[16],
                    int viewport_width, int viewport_height)
{
    if (!screen_coords || !world_coords || !model_matrix || !view_matrix || !projection_matrix)
    {
        LOG_ERROR("Invalid pointers");
        return;
    }

    float temp[16];
    float mvp[16];
    float clip_coords[4];

    // Model * View
    matrix_multiply(temp, model_matrix, view_matrix);
    // Model * View * Projection
    matrix_multiply(mvp, temp, projection_matrix);

    // Transform to clip space
    float world_homogeneous[4] = {world_coords[0], world_coords[1], world_coords[2], 1.0f};
    transform_vector4(clip_coords, mvp, world_homogeneous);

    // Perspective divide
    if (clip_coords[3] != 0.0f)
    {
        clip_coords[0] /= clip_coords[3];
        clip_coords[1] /= clip_coords[3];
        clip_coords[2] /= clip_coords[3];
    }

    // Viewport transformation
    screen_coords[0] = (clip_coords[0] + 1.0f) * 0.5f * viewport_width;
    screen_coords[1] = (1.0f - clip_coords[1]) * 0.5f * viewport_height; // Flip Y
    screen_coords[2] = clip_coords[2];
    screen_coords[3] = clip_coords[3];
}

void screen_to_world(float world_coords[3],
                    const float screen_coords[3],
                    const float inv_mvp_matrix[16],
                    int viewport_width, int viewport_height)
{
    if (!world_coords || !screen_coords || !inv_mvp_matrix)
    {
        LOG_ERROR("Invalid pointers");
        return;
    }

    // Convert screen to NDC
    float ndc_x = (2.0f * screen_coords[0]) / viewport_width - 1.0f;
    float ndc_y = 1.0f - (2.0f * screen_coords[1]) / viewport_height; // Flip Y
    float ndc_z = screen_coords[2]; // Depth 0-1

    float clip_coords[4] = {ndc_x, ndc_y, ndc_z, 1.0f};

    // Transform back to world space
    float world_homogeneous[4];
    transform_vector4(world_homogeneous, inv_mvp_matrix, clip_coords);

    // Perspective divide
    if (world_homogeneous[3] != 0.0f)
    {
        world_coords[0] = world_homogeneous[0] / world_homogeneous[3];
        world_coords[1] = world_homogeneous[1] / world_homogeneous[3];
        world_coords[2] = world_homogeneous[2] / world_homogeneous[3];
    }
    else
    {
        world_coords[0] = world_homogeneous[0];
        world_coords[1] = world_homogeneous[1];
        world_coords[2] = world_homogeneous[2];
    }
}

bool matrix_inverse(float result[16], const float matrix[16])
{
    if (!result || !matrix)
    {
        LOG_ERROR("Invalid matrix pointers");
        return false;
    }

    // Simple 4x4 matrix inversion using Gaussian elimination
    float m[16];
    memcpy(m, matrix, 16 * sizeof(float));

    float inv[16];
    matrix_identity(inv);

    // Forward elimination
    for (int i = 0; i < 4; ++i)
    {
        // Find pivot
        int pivot = i;
        for (int j = i + 1; j < 4; ++j)
        {
            if (fabsf(m[j + i * 4]) > fabsf(m[pivot + i * 4]))
            {
                pivot = j;
            }
        }

        // Swap rows
        if (pivot != i)
        {
            for (int k = 0; k < 4; ++k)
            {
                float temp = m[i + k * 4];
                m[i + k * 4] = m[pivot + k * 4];
                m[pivot + k * 4] = temp;

                temp = inv[i + k * 4];
                inv[i + k * 4] = inv[pivot + k * 4];
                inv[pivot + k * 4] = temp;
            }
        }

        // Check for singular matrix
        if (fabsf(m[i + i * 4]) < 1e-6f)
        {
            LOG_ERROR("Matrix is singular, cannot invert");
            return false;
        }

        // Eliminate
        for (int j = 0; j < 4; ++j)
        {
            if (j != i)
            {
                float factor = m[j + i * 4] / m[i + i * 4];
                for (int k = 0; k < 4; ++k)
                {
                    m[j + k * 4] -= factor * m[i + k * 4];
                    inv[j + k * 4] -= factor * inv[i + k * 4];
                }
            }
        }

        // Normalize pivot row
        float factor = m[i + i * 4];
        for (int k = 0; k < 4; ++k)
        {
            m[i + k * 4] /= factor;
            inv[i + k * 4] /= factor;
        }
    }

    memcpy(result, inv, 16 * sizeof(float));
    return true;
}

void matrix_transpose(float result[16], const float matrix[16])
{
    if (!result || !matrix)
    {
        LOG_ERROR("Invalid matrix pointers");
        return;
    }

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result[j + i * 4] = matrix[i + j * 4];
        }
    }
}