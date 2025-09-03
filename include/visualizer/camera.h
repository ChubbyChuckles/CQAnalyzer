#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>

/**
 * @file camera.h
 * @brief 3D camera management
 *
 * Provides functions to control the 3D camera for visualization.
 */

typedef struct
{
    float position[3];
    float target[3];
    float up[3];
    float fov;
    float near_plane;
    float far_plane;
} Camera;

/**
 * @brief Initialize camera with default values
 *
 * @param camera Camera to initialize
 */
void camera_init(Camera *camera);

/**
 * @brief Set camera position
 *
 * @param camera Camera to modify
 * @param x X position
 * @param y Y position
 * @param z Z position
 */
void camera_set_position(Camera *camera, float x, float y, float z);

/**
 * @brief Set camera target
 *
 * @param camera Camera to modify
 * @param x X target
 * @param y Y target
 * @param z Z target
 */
void camera_set_target(Camera *camera, float x, float y, float z);

/**
 * @brief Move camera
 *
 * @param camera Camera to modify
 * @param dx X movement
 * @param dy Y movement
 * @param dz Z movement
 */
void camera_move(Camera *camera, float dx, float dy, float dz);

/**
 * @brief Rotate camera around target
 *
 * @param camera Camera to modify
 * @param yaw Yaw angle in radians
 * @param pitch Pitch angle in radians
 */
void camera_rotate(Camera *camera, float yaw, float pitch);

/**
 * @brief Zoom camera
 *
 * @param camera Camera to modify
 * @param factor Zoom factor
 */
void camera_zoom(Camera *camera, float factor);

#endif // CAMERA_H
