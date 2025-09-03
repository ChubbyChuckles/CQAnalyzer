#include <stdio.h>
#include <string.h>

#include "ui/input_handler.h"
#include "utils/logger.h"

#define MAX_KEYS 512
#define MAX_MOUSE_BUTTONS 8

static bool key_states[MAX_KEYS];
static bool mouse_button_states[MAX_MOUSE_BUTTONS];
static double mouse_x = 0.0;
static double mouse_y = 0.0;

CQError input_handler_init(void)
{
    memset(key_states, 0, sizeof(key_states));
    memset(mouse_button_states, 0, sizeof(mouse_button_states));
    mouse_x = 0.0;
    mouse_y = 0.0;

    LOG_INFO("Input handler initialized");
    return CQ_SUCCESS;
}

void input_handler_shutdown(void)
{
    LOG_INFO("Input handler shutdown");
}

void input_handle_key(int key, int action, int mods)
{
    if (key < 0 || key >= MAX_KEYS)
    {
        LOG_WARNING("Invalid key code: %d", key);
        return;
    }

    key_states[key] = (action != 0); // 0 = release, non-zero = press

    LOG_DEBUG("Key %d %s", key, key_states[key] ? "pressed" : "released");
}

void input_handle_mouse_button(int button, int action, int mods)
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS)
    {
        LOG_WARNING("Invalid mouse button: %d", button);
        return;
    }

    mouse_button_states[button] = (action != 0);

    LOG_DEBUG("Mouse button %d %s", button, mouse_button_states[button] ? "pressed" : "released");
}

void input_handle_mouse_move(double x, double y)
{
    mouse_x = x;
    mouse_y = y;

    // Only log occasionally to avoid spam
    static int log_counter = 0;
    if (++log_counter % 100 == 0)
    {
        LOG_DEBUG("Mouse moved to (%.1f, %.1f)", x, y);
    }
}

void input_handle_scroll(double x_offset, double y_offset)
{
    LOG_DEBUG("Mouse scroll: x=%.2f, y=%.2f", x_offset, y_offset);

    // TODO: Handle scroll events for zooming, etc.
}

bool input_is_key_pressed(int key)
{
    if (key < 0 || key >= MAX_KEYS)
    {
        return false;
    }

    return key_states[key];
}

bool input_is_mouse_button_pressed(int button)
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS)
    {
        return false;
    }

    return mouse_button_states[button];
}
