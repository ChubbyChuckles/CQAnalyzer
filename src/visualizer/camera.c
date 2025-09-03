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
    if (!camera)
    {
        return;
    }

    // Calculate current distance from camera to target
    float dx = camera->position[0] - camera->target[0];
    float dy = camera->position[1] - camera->target[1];
    float dz = camera->position[2] - camera->target[2];
    float distance = sqrtf(dx * dx + dy * dy + dz * dz);

    // Convert to spherical coordinates
    float current_yaw = atan2f(dx, dz);
    float current_pitch = asinf(dy / distance);

    // Apply rotation
    current_yaw += yaw;
    current_pitch += pitch;

    // Clamp pitch to avoid gimbal lock
    if (current_pitch > M_PI_2 - 0.1f) current_pitch = M_PI_2 - 0.1f;
    if (current_pitch < -M_PI_2 + 0.1f) current_pitch = -M_PI_2 + 0.1f;

    // Convert back to Cartesian coordinates
    camera->position[0] = camera->target[0] + distance * sinf(current_yaw) * cosf(current_pitch);
    camera->position[1] = camera->target[1] + distance * sinf(current_pitch);
    camera->position[2] = camera->target[2] + distance * cosf(current_yaw) * cosf(current_pitch);

    LOG_DEBUG("Camera rotated (yaw: %.2f, pitch: %.2f)", yaw, pitch);
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

void camera_get_view_matrix(const Camera *camera, float view_matrix[16])
{
    if (!camera || !view_matrix)
    {
        return;
    }

    // Calculate forward vector
    float forward[3];
    forward[0] = camera->target[0] - camera->position[0];
    forward[1] = camera->target[1] - camera->position[1];
    forward[2] = camera->target[2] - camera->position[2];

    // Normalize forward vector
    float forward_length = sqrtf(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
    if (forward_length > 0.0f)
    {
        forward[0] /= forward_length;
        forward[1] /= forward_length;
        forward[2] /= forward_length;
    }

    // Calculate right vector (cross product of forward and up)
    float right[3];
    right[0] = forward[1] * camera->up[2] - forward[2] * camera->up[1];
    right[1] = forward[2] * camera->up[0] - forward[0] * camera->up[2];
    right[2] = forward[0] * camera->up[1] - forward[1] * camera->up[0];

    // Normalize right vector
    float right_length = sqrtf(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
    if (right_length > 0.0f)
    {
        right[0] /= right_length;
        right[1] /= right_length;
        right[2] /= right_length;
    }

    // Recalculate up vector (cross product of right and forward)
    float up[3];
    up[0] = right[1] * forward[2] - right[2] * forward[1];
    up[1] = right[2] * forward[0] - right[0] * forward[2];
    up[2] = right[0] * forward[1] - right[1] * forward[0];

    // Build view matrix (column-major)
    view_matrix[0] = right[0];
    view_matrix[1] = up[0];
    view_matrix[2] = -forward[0];
    view_matrix[3] = 0.0f;

    view_matrix[4] = right[1];
    view_matrix[5] = up[1];
    view_matrix[6] = -forward[1];
    view_matrix[7] = 0.0f;

    view_matrix[8] = right[2];
    view_matrix[9] = up[2];
    view_matrix[10] = -forward[2];
    view_matrix[11] = 0.0f;

    view_matrix[12] = -(right[0] * camera->position[0] + right[1] * camera->position[1] + right[2] * camera->position[2]);
    view_matrix[13] = -(up[0] * camera->position[0] + up[1] * camera->position[1] + up[2] * camera->position[2]);
    view_matrix[14] = -(-forward[0] * camera->position[0] - forward[1] * camera->position[1] - forward[2] * camera->position[2]);
    view_matrix[15] = 1.0f;
}

void camera_get_projection_matrix(const Camera *camera, float aspect_ratio, float projection_matrix[16])
{
    if (!camera || !projection_matrix || aspect_ratio <= 0.0f)
    {
        return;
    }

    float fov_rad = camera->fov * M_PI / 180.0f;
    float tan_half_fov = tanf(fov_rad / 2.0f);

    float range = camera->near_plane - camera->far_plane;

    // Build perspective projection matrix (column-major)
    projection_matrix[0] = 1.0f / (aspect_ratio * tan_half_fov);
    projection_matrix[1] = 0.0f;
    projection_matrix[2] = 0.0f;
    projection_matrix[3] = 0.0f;

    projection_matrix[4] = 0.0f;
    projection_matrix[5] = 1.0f / tan_half_fov;
    projection_matrix[6] = 0.0f;
    projection_matrix[7] = 0.0f;

    projection_matrix[8] = 0.0f;
    projection_matrix[9] = 0.0f;
    projection_matrix[10] = (-camera->near_plane - camera->far_plane) / range;
    projection_matrix[11] = 2.0f * camera->far_plane * camera->near_plane / range;

    projection_matrix[12] = 0.0f;
    projection_matrix[13] = 0.0f;
    projection_matrix[14] = 1.0f;
    projection_matrix[15] = 0.0f;
}
