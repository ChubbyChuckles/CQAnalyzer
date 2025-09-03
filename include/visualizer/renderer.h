#ifndef RENDERER_H
#define RENDERER_H

#include "cqanalyzer.h"

/**
 * @file renderer.h
 * @brief 3D rendering functionality
 *
 * Provides functions to initialize and manage 3D rendering
 * using OpenGL and GLFW.
 */

/**
 * @brief Initialize 3D renderer
 *
 * @param width Window width
 * @param height Window height
 * @param title Window title
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError renderer_init(int width, int height, const char* title);

/**
 * @brief Shutdown 3D renderer
 */
void renderer_shutdown(void);

/**
 * @brief Check if renderer is running
 *
 * @return true if running, false otherwise
 */
bool renderer_is_running(void);

/**
 * @brief Update renderer (handle events, etc.)
 */
void renderer_update(void);

/**
 * @brief Render current scene
 */
void renderer_render(void);

/**
 * @brief Swap buffers and present frame
 */
void renderer_present(void);

#endif // RENDERER_H
