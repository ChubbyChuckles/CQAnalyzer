#ifndef TRANSFORMS_H
#define TRANSFORMS_H

#include <stdbool.h>

/**
 * @file transforms.h
 * @brief Coordinate system transformation functions
 *
 * Provides functions for 3D coordinate transformations, matrix operations,
 * and conversions between different coordinate systems.
 */

/**
 * @brief Multiply two 4x4 matrices
 *
 * @param result Output 4x4 matrix (column-major)
 * @param a First 4x4 matrix (column-major)
 * @param b Second 4x4 matrix (column-major)
 */
void matrix_multiply(float result[16], const float a[16], const float b[16]);

/**
 * @brief Transform a 3D vector by a 4x4 matrix
 *
 * @param result Output 3D vector
 * @param matrix 4x4 transformation matrix (column-major)
 * @param vector Input 3D vector
 */
void transform_vector(float result[3], const float matrix[16], const float vector[3]);

/**
 * @brief Transform a 4D homogeneous vector by a 4x4 matrix
 *
 * @param result Output 4D vector
 * @param matrix 4x4 transformation matrix (column-major)
 * @param vector Input 4D vector
 */
void transform_vector4(float result[4], const float matrix[16], const float vector[4]);

/**
 * @brief Create identity matrix
 *
 * @param matrix Output 4x4 identity matrix (column-major)
 */
void matrix_identity(float matrix[16]);

/**
 * @brief Create translation matrix
 *
 * @param matrix Output 4x4 translation matrix (column-major)
 * @param x Translation in X
 * @param y Translation in Y
 * @param z Translation in Z
 */
void matrix_translate(float matrix[16], float x, float y, float z);

/**
 * @brief Create rotation matrix around X axis
 *
 * @param matrix Output 4x4 rotation matrix (column-major)
 * @param angle Rotation angle in radians
 */
void matrix_rotate_x(float matrix[16], float angle);

/**
 * @brief Create rotation matrix around Y axis
 *
 * @param matrix Output 4x4 rotation matrix (column-major)
 * @param angle Rotation angle in radians
 */
void matrix_rotate_y(float matrix[16], float angle);

/**
 * @brief Create rotation matrix around Z axis
 *
 * @param matrix Output 4x4 rotation matrix (column-major)
 * @param angle Rotation angle in radians
 */
void matrix_rotate_z(float matrix[16], float angle);

/**
 * @brief Create scale matrix
 *
 * @param matrix Output 4x4 scale matrix (column-major)
 * @param x Scale in X
 * @param y Scale in Y
 * @param z Scale in Z
 */
void matrix_scale(float matrix[16], float x, float y, float z);

/**
 * @brief Create model matrix from translation, rotation, and scale
 *
 * @param matrix Output 4x4 model matrix (column-major)
 * @param translate_x Translation in X
 * @param translate_y Translation in Y
 * @param translate_z Translation in Z
 * @param rotate_x Rotation around X in radians
 * @param rotate_y Rotation around Y in radians
 * @param rotate_z Rotation around Z in radians
 * @param scale_x Scale in X
 * @param scale_y Scale in Y
 * @param scale_z Scale in Z
 */
void create_model_matrix(float matrix[16],
                        float translate_x, float translate_y, float translate_z,
                        float rotate_x, float rotate_y, float rotate_z,
                        float scale_x, float scale_y, float scale_z);

/**
 * @brief Transform world coordinates to screen coordinates
 *
 * @param screen_coords Output screen coordinates [x, y, z, w]
 * @param world_coords Input world coordinates [x, y, z]
 * @param model_matrix 4x4 model matrix (column-major)
 * @param view_matrix 4x4 view matrix (column-major)
 * @param projection_matrix 4x4 projection matrix (column-major)
 * @param viewport_width Viewport width
 * @param viewport_height Viewport height
 */
void world_to_screen(float screen_coords[4],
                    const float world_coords[3],
                    const float model_matrix[16],
                    const float view_matrix[16],
                    const float projection_matrix[16],
                    int viewport_width, int viewport_height);

/**
 * @brief Transform screen coordinates to world coordinates
 *
 * @param world_coords Output world coordinates [x, y, z]
 * @param screen_coords Input screen coordinates [x, y, depth] (depth 0-1)
 * @param inv_mvp_matrix Inverse of model-view-projection matrix (column-major)
 * @param viewport_width Viewport width
 * @param viewport_height Viewport height
 */
void screen_to_world(float world_coords[3],
                    const float screen_coords[3],
                    const float inv_mvp_matrix[16],
                    int viewport_width, int viewport_height);

/**
 * @brief Calculate inverse of a 4x4 matrix
 *
 * @param result Output inverse matrix (column-major)
 * @param matrix Input 4x4 matrix (column-major)
 * @return true if matrix is invertible, false otherwise
 */
bool matrix_inverse(float result[16], const float matrix[16]);

/**
 * @brief Transpose a 4x4 matrix
 *
 * @param result Output transposed matrix (column-major)
 * @param matrix Input 4x4 matrix (column-major)
 */
void matrix_transpose(float result[16], const float matrix[16]);

#endif // TRANSFORMS_H