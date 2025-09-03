#include <math.h>

#include "visualizer/camera.h"
#include "utils/logger.h"

void camera_init(Camera *camera)
{
    if (!camera)
    {
        LOG_ERROR("Invalid camera pointer");
        return;
    }

    // Default camera position
    camera->position[0] = 0.0f;
    camera->position[1] = 0.0f;
    camera->position[2] = 10.0f;

    // Default target (looking at origin)
    camera->target[0] = 0.0f;
    camera->target[1] = 0.0f;
    camera->target[2] = 0.0f;

    // Default up vector
    camera->up[0] = 0.0f;
    camera->up[1] = 1.0f;
    camera->up[2] = 0.0f;

    // Default field of view
    camera->fov = 45.0f;
    camera->near_plane = 0.1f;
    camera->far_plane = 1000.0f;

    LOG_DEBUG("Camera initialized");
}

void camera_set_position(Camera *camera, float x, float y, float z)
{
    if (!camera)
    {
        return;
    }

    camera->position[0] = x;
    camera->position[1] = y;
    camera->position[2] = z;

    LOG_DEBUG("Camera position set to (%.2f, %.2f, %.2f)", x, y, z);
}

void camera_set_target(Camera *camera, float x, float y, float z)
{
    if (!camera)
    {
        return;
    }

    camera->target[0] = x;
    camera->target[1] = y;
    camera->target[2] = z;

    LOG_DEBUG("Camera target set to (%.2f, %.2f, %.2f)", x, y, z);
}

void camera_move(Camera *camera, float dx, float dy, float dz)
{
    if (!camera)
    {
        return;
    }

    camera->position[0] += dx;
    camera->position[1] += dy;
    camera->position[2] += dz;

    LOG_DEBUG("Camera moved by (%.2f, %.2f, %.2f)", dx, dy, dz);
}

void camera_rotate(Camera *camera, float yaw, float pitch)
{
    // TODO: Implement camera rotation
    // TODO: Calculate new position based on yaw and pitch
    // TODO: Keep camera at constant distance from target

    LOG_WARNING("Camera rotation not yet implemented");
}

void camera_zoom(Camera *camera, float factor)
{
    if (!camera)
    {
        return;
    }

    // Calculate direction vector from position to target
    float dir[3];
    dir[0] = camera->target[0] - camera->position[0];
    dir[1] = camera->target[1] - camera->position[1];
    dir[2] = camera->target[2] - camera->position[2];

    // Move camera along the direction vector
    camera->position[0] += dir[0] * factor;
    camera->position[1] += dir[1] * factor;
    camera->position[2] += dir[2] * factor;

    LOG_DEBUG("Camera zoomed by factor %.2f", factor);
}
