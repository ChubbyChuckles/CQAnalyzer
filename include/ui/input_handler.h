#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <stdbool.h>
#include "cqanalyzer.h"

/**
 * @file input_handler.h
 * @brief Input handling for user interactions
 *
 * Provides functions to handle keyboard and mouse input
 * for the 3D visualization interface.
 */

/**
 * @brief Initialize input handler
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError input_handler_init(void);

/**
 * @brief Shutdown input handler
 */
void input_handler_shutdown(void);

/**
 * @brief Process keyboard input
 *
 * @param key Key code
 * @param action Key action (press/release)
 * @param mods Modifier keys
 */
void input_handle_key(int key, int action, int mods);

/**
 * @brief Process mouse button input
 *
 * @param button Mouse button
 * @param action Button action (press/release)
 * @param mods Modifier keys
 */
void input_handle_mouse_button(int button, int action, int mods);

/**
 * @brief Process mouse movement
 *
 * @param x Mouse X position
 * @param y Mouse Y position
 */
void input_handle_mouse_move(double x, double y);

/**
 * @brief Process mouse scroll
 *
 * @param x_offset Scroll X offset
 * @param y_offset Scroll Y offset
 */
void input_handle_scroll(double x_offset, double y_offset);

/**
 * @brief Check if key is pressed
 *
 * @param key Key code
 * @return true if pressed, false otherwise
 */
bool input_is_key_pressed(int key);

/**
 * @brief Check if mouse button is pressed
 *
 * @param button Mouse button
 * @return true if pressed, false otherwise
 */
bool input_is_mouse_button_pressed(int button);

/**
 * @brief Get current mouse position
 *
 * @param x Pointer to store X position
 * @param y Pointer to store Y position
 */
void input_get_mouse_position(double *x, double *y);

/**
 * @brief Get accumulated scroll deltas and reset them
 *
 * @param x Pointer to store X scroll delta
 * @param y Pointer to store Y scroll delta
 */
void input_get_scroll_delta(double *x, double *y);

#endif // INPUT_HANDLER_H
