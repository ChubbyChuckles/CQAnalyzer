#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "visualizer/picking.h"
#include "visualizer/camera.h"
#include "visualizer/renderer.h"
#include "utils/logger.h"

#define MAX_PICKABLE_OBJECTS 1000

static PickableObject *pickable_objects = NULL;
static int num_pickable_objects = 0;
static int max_pickable_objects = 0;

static SelectedObject selected_objects[MAX_SELECTED_OBJECTS];
static int num_selected_objects = 0;

static Color highlight_color = {1.0f, 1.0f, 0.0f, 1.0f}; // Yellow highlight

// Forward declarations
static bool ray_sphere_intersection(float ray_origin[3], float ray_direction[3],
                                   float sphere_center[3], float sphere_radius);
static bool ray_cube_intersection(float ray_origin[3], float ray_direction[3],
                                 float cube_center[3], float cube_size[3]);
static void screen_to_world_ray(float screen_x, float screen_y, int screen_width, int screen_height,
                               float ray_origin[3], float ray_direction[3]);
static float vector_length(float v[3]);
static void vector_normalize(float v[3]);
static void vector_subtract(float result[3], float a[3], float b[3]);
static void vector_cross(float result[3], float a[3], float b[3]);
static float vector_dot(float a[3], float b[3]);

CQError picking_init(void)
{
    pickable_objects = (PickableObject *)malloc(MAX_PICKABLE_OBJECTS * sizeof(PickableObject));
    if (!pickable_objects)
    {
        LOG_ERROR("Failed to allocate memory for pickable objects");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    max_pickable_objects = MAX_PICKABLE_OBJECTS;
    num_pickable_objects = 0;
    num_selected_objects = 0;

    LOG_INFO("Picking system initialized with capacity for %d objects", max_pickable_objects);
    return CQ_SUCCESS;
}

void picking_shutdown(void)
{
    if (pickable_objects)
    {
        free(pickable_objects);
        pickable_objects = NULL;
    }

    num_pickable_objects = 0;
    max_pickable_objects = 0;
    num_selected_objects = 0;

    LOG_INFO("Picking system shutdown");
}

CQError picking_register_object(const PickableObject *object)
{
    if (!object)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (num_pickable_objects >= max_pickable_objects)
    {
        LOG_ERROR("Picking system is full (%d objects)", max_pickable_objects);
        return CQ_ERROR_UNKNOWN;
    }

    pickable_objects[num_pickable_objects] = *object;
    num_pickable_objects++;

    LOG_DEBUG("Registered pickable object (ID: %d, type: %d)", object->object_id, object->type);
    return CQ_SUCCESS;
}

CQError picking_unregister_object(int object_id)
{
    for (int i = 0; i < num_pickable_objects; i++)
    {
        if (pickable_objects[i].object_id == object_id)
        {
            // Shift remaining objects
            for (int j = i; j < num_pickable_objects - 1; j++)
            {
                pickable_objects[j] = pickable_objects[j + 1];
            }
            num_pickable_objects--;

            // Also remove from selection if selected
            picking_deselect_object(object_id);

            LOG_DEBUG("Unregistered pickable object (ID: %d)", object_id);
            return CQ_SUCCESS;
        }
    }

    LOG_WARNING("Object ID %d not found for unregistration", object_id);
    return CQ_ERROR_INVALID_ARGUMENT;
}

void picking_clear_objects(void)
{
    num_pickable_objects = 0;
    num_selected_objects = 0;
    LOG_DEBUG("Cleared all pickable objects and selections");
}

int picking_pick_object(float screen_x, float screen_y, int screen_width, int screen_height)
{
    float ray_origin[3];
    float ray_direction[3];

    // Convert screen coordinates to world ray
    screen_to_world_ray(screen_x, screen_y, screen_width, screen_height, ray_origin, ray_direction);

    int closest_object_id = -1;
    float closest_distance = FLT_MAX;

    // Test intersection with all pickable objects
    for (int i = 0; i < num_pickable_objects; i++)
    {
        PickableObject *obj = &pickable_objects[i];
        bool intersects = false;

        switch (obj->type)
        {
        case OBJECT_TYPE_SPHERE:
            intersects = ray_sphere_intersection(ray_origin, ray_direction,
                                               obj->position, obj->radius);
            break;

        case OBJECT_TYPE_CUBE:
            intersects = ray_cube_intersection(ray_origin, ray_direction,
                                             obj->position, obj->size);
            break;

        case OBJECT_TYPE_POINT:
            // For points, use a small sphere intersection
            intersects = ray_sphere_intersection(ray_origin, ray_direction,
                                               obj->position, 0.1f);
            break;

        case OBJECT_TYPE_LINE:
            // Line picking not implemented yet
            intersects = false;
            break;

        default:
            intersects = false;
            break;
        }

        if (intersects)
        {
            // Calculate distance from ray origin to object
            float distance[3];
            vector_subtract(distance, obj->position, ray_origin);
            float dist = vector_length(distance);

            if (dist < closest_distance)
            {
                closest_distance = dist;
                closest_object_id = obj->object_id;
            }
        }
    }

    if (closest_object_id != -1)
    {
        LOG_DEBUG("Picked object ID: %d", closest_object_id);
    }

    return closest_object_id;
}

CQError picking_select_object(int object_id)
{
    if (num_selected_objects >= MAX_SELECTED_OBJECTS)
    {
        LOG_WARNING("Maximum number of selected objects reached");
        return CQ_ERROR_UNKNOWN;
    }

    // Check if already selected
    if (picking_is_selected(object_id))
    {
        return CQ_SUCCESS;
    }

    // Find the object to get its label
    for (int i = 0; i < num_pickable_objects; i++)
    {
        if (pickable_objects[i].object_id == object_id)
        {
            selected_objects[num_selected_objects].object_id = object_id;
            strcpy(selected_objects[num_selected_objects].label, pickable_objects[i].label);
            // Note: original_color would need to be retrieved from the actual object
            // For now, we'll set a default
            selected_objects[num_selected_objects].original_color.r = 0.5f;
            selected_objects[num_selected_objects].original_color.g = 0.5f;
            selected_objects[num_selected_objects].original_color.b = 0.5f;
            selected_objects[num_selected_objects].original_color.a = 1.0f;

            num_selected_objects++;
            LOG_DEBUG("Selected object ID: %d (%s)", object_id, pickable_objects[i].label);
            return CQ_SUCCESS;
        }
    }

    LOG_WARNING("Object ID %d not found for selection", object_id);
    return CQ_ERROR_INVALID_ARGUMENT;
}

CQError picking_deselect_object(int object_id)
{
    for (int i = 0; i < num_selected_objects; i++)
    {
        if (selected_objects[i].object_id == object_id)
        {
            // Shift remaining selections
            for (int j = i; j < num_selected_objects - 1; j++)
            {
                selected_objects[j] = selected_objects[j + 1];
            }
            num_selected_objects--;

            LOG_DEBUG("Deselected object ID: %d", object_id);
            return CQ_SUCCESS;
        }
    }

    return CQ_SUCCESS; // Not an error if object wasn't selected
}

void picking_clear_selection(void)
{
    num_selected_objects = 0;
    LOG_DEBUG("Cleared all selections");
}

bool picking_is_selected(int object_id)
{
    for (int i = 0; i < num_selected_objects; i++)
    {
        if (selected_objects[i].object_id == object_id)
        {
            return true;
        }
    }
    return false;
}

int picking_get_selected_count(void)
{
    return num_selected_objects;
}

CQError picking_get_selected_object(int index, int *object_id, char *label)
{
    if (index < 0 || index >= num_selected_objects)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (object_id)
    {
        *object_id = selected_objects[index].object_id;
    }

    if (label)
    {
        strcpy(label, selected_objects[index].label);
    }

    return CQ_SUCCESS;
}

void picking_set_highlight_color(const Color *color)
{
    if (color)
    {
        highlight_color = *color;
    }
}

void picking_get_highlight_color(Color *color)
{
    if (color)
    {
        *color = highlight_color;
    }
}

// Helper function: Ray-sphere intersection
static bool ray_sphere_intersection(float ray_origin[3], float ray_direction[3],
                                   float sphere_center[3], float sphere_radius)
{
    float oc[3];
    vector_subtract(oc, ray_origin, sphere_center);

    float a = vector_dot(ray_direction, ray_direction);
    float b = 2.0f * vector_dot(oc, ray_direction);
    float c = vector_dot(oc, oc) - sphere_radius * sphere_radius;

    float discriminant = b * b - 4 * a * c;
    return discriminant >= 0;
}

// Helper function: Ray-cube intersection (AABB)
static bool ray_cube_intersection(float ray_origin[3], float ray_direction[3],
                                 float cube_center[3], float cube_size[3])
{
    float half_size[3] = {cube_size[0] * 0.5f, cube_size[1] * 0.5f, cube_size[2] * 0.5f};

    float min[3] = {cube_center[0] - half_size[0], cube_center[1] - half_size[1], cube_center[2] - half_size[2]};
    float max[3] = {cube_center[0] + half_size[0], cube_center[1] + half_size[1], cube_center[2] + half_size[2]};

    float tmin = (min[0] - ray_origin[0]) / ray_direction[0];
    float tmax = (max[0] - ray_origin[0]) / ray_direction[0];

    if (tmin > tmax)
    {
        float temp = tmin;
        tmin = tmax;
        tmax = temp;
    }

    float tymin = (min[1] - ray_origin[1]) / ray_direction[1];
    float tymax = (max[1] - ray_origin[1]) / ray_direction[1];

    if (tymin > tymax)
    {
        float temp = tymin;
        tymin = tymax;
        tymax = temp;
    }

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin = (min[2] - ray_origin[2]) / ray_direction[2];
    float tzmax = (max[2] - ray_origin[2]) / ray_direction[2];

    if (tzmin > tzmax)
    {
        float temp = tzmin;
        tzmin = tzmax;
        tzmax = temp;
    }

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    return true;
}

// Helper function: Convert screen coordinates to world ray
static void screen_to_world_ray(float screen_x, float screen_y, int screen_width, int screen_height,
                               float ray_origin[3], float ray_direction[3])
{
    // Get camera matrices (this would need to be implemented in camera.c)
    // For now, we'll use a simplified approach

    // Normalize screen coordinates to [-1, 1]
    float x = (2.0f * screen_x) / screen_width - 1.0f;
    float y = 1.0f - (2.0f * screen_y) / screen_height;

    // Assume perspective projection
    float aspect = (float)screen_width / (float)screen_height;
    float fov = 45.0f * M_PI / 180.0f; // 45 degrees
    float tan_half_fov = tanf(fov * 0.5f);

    // Ray in camera space
    float ray_cam[3] = {x * tan_half_fov * aspect, y * tan_half_fov, -1.0f};
    vector_normalize(ray_cam);

    // For simplicity, assume camera at origin looking down -Z
    // In a real implementation, this would transform by inverse view matrix
    ray_origin[0] = 0.0f;
    ray_origin[1] = 0.0f;
    ray_origin[2] = 0.0f;

    ray_direction[0] = ray_cam[0];
    ray_direction[1] = ray_cam[1];
    ray_direction[2] = ray_cam[2];
}

// Vector math helper functions
static float vector_length(float v[3])
{
    return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

static void vector_normalize(float v[3])
{
    float len = vector_length(v);
    if (len > 0.0f)
    {
        v[0] /= len;
        v[1] /= len;
        v[2] /= len;
    }
}

static void vector_subtract(float result[3], float a[3], float b[3])
{
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
}

static void vector_cross(float result[3], float a[3], float b[3])
{
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
}

static float vector_dot(float a[3], float b[3])
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}