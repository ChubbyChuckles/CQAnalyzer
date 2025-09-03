#ifndef PICKING_H
#define PICKING_H

#include <stdbool.h>
#include "cqanalyzer.h"
#include "visualizer/color.h"

/**
 * @file picking.h
 * @brief Object picking and selection system for 3D visualization
 *
 * Provides ray casting and object selection functionality for
 * interactive 3D visualization.
 */

#define MAX_SELECTED_OBJECTS 100

typedef enum
{
    OBJECT_TYPE_SPHERE,
    OBJECT_TYPE_CUBE,
    OBJECT_TYPE_POINT,
    OBJECT_TYPE_LINE
} ObjectType;

typedef struct
{
    int object_id;
    ObjectType type;
    float position[3];
    float radius;  // For spheres
    float size[3]; // For cubes
    char label[256];
} PickableObject;

typedef struct
{
    int object_id;
    char label[256];
    Color original_color;
} SelectedObject;

/**
 * @brief Initialize picking system
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError picking_init(void);

/**
 * @brief Shutdown picking system
 */
void picking_shutdown(void);

/**
 * @brief Register an object for picking
 *
 * @param object Object to register
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError picking_register_object(const PickableObject *object);

/**
 * @brief Unregister an object from picking
 *
 * @param object_id ID of object to unregister
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError picking_unregister_object(int object_id);

/**
 * @brief Clear all registered objects
 */
void picking_clear_objects(void);

/**
 * @brief Perform picking at screen coordinates
 *
 * @param screen_x Screen X coordinate
 * @param screen_y Screen Y coordinate
 * @param screen_width Screen width
 * @param screen_height Screen height
 * @return ID of picked object, or -1 if no object picked
 */
int picking_pick_object(float screen_x, float screen_y, int screen_width, int screen_height);

/**
 * @brief Select an object
 *
 * @param object_id ID of object to select
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError picking_select_object(int object_id);

/**
 * @brief Deselect an object
 *
 * @param object_id ID of object to deselect
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError picking_deselect_object(int object_id);

/**
 * @brief Clear all selections
 */
void picking_clear_selection(void);

/**
 * @brief Check if object is selected
 *
 * @param object_id ID of object to check
 * @return true if selected, false otherwise
 */
bool picking_is_selected(int object_id);

/**
 * @brief Get number of selected objects
 *
 * @return Number of selected objects
 */
int picking_get_selected_count(void);

/**
 * @brief Get selected object by index
 *
 * @param index Index in selection list
 * @param object_id Pointer to store object ID
 * @param label Pointer to store object label
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError picking_get_selected_object(int index, int *object_id, char *label);

/**
 * @brief Set selection highlight color
 *
 * @param color Highlight color
 */
void picking_set_highlight_color(const Color *color);

/**
 * @brief Get selection highlight color
 *
 * @param color Pointer to store highlight color
 */
void picking_get_highlight_color(Color *color);

#endif // PICKING_H