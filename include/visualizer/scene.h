#ifndef SCENE_H
#define SCENE_H

#include "cqanalyzer.h"

/**
 * @file scene.h
 * @brief 3D scene management
 *
 * Provides functions to manage 3D scene objects and rendering.
 */

typedef struct {
    float position[3];
    float color[4];
    float scale[3];
} SceneObject;

/**
 * @brief Initialize 3D scene
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_init(void);

/**
 * @brief Shutdown 3D scene
 */
void scene_shutdown(void);

/**
 * @brief Add object to scene
 *
 * @param object Object to add
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_add_object(const SceneObject* object);

/**
 * @brief Remove object from scene
 *
 * @param index Index of object to remove
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError scene_remove_object(int index);

/**
 * @brief Update scene objects
 *
 * @param delta_time Time elapsed since last update
 */
void scene_update(float delta_time);

/**
 * @brief Render scene
 */
void scene_render(void);

/**
 * @brief Clear all objects from scene
 */
void scene_clear(void);

#endif // SCENE_H
