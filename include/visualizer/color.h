#ifndef COLOR_H
#define COLOR_H

#include "cqanalyzer.h"

/**
 * @file color.h
 * @brief Color management system for visualization
 *
 * Provides color structures and functions for RGB/HSV conversion,
 * color blending, and color manipulation.
 */

typedef struct
{
    float r, g, b, a; // Red, Green, Blue, Alpha (0.0-1.0)
} Color;

/**
 * @brief Create a color from RGB values
 *
 * @param r Red component (0.0-1.0)
 * @param g Green component (0.0-1.0)
 * @param b Blue component (0.0-1.0)
 * @param a Alpha component (0.0-1.0)
 * @return Color structure
 */
Color color_create(float r, float g, float b, float a);

/**
 * @brief Create a color from HSV values
 *
 * @param h Hue (0.0-360.0)
 * @param s Saturation (0.0-1.0)
 * @param v Value (0.0-1.0)
 * @param a Alpha (0.0-1.0)
 * @return Color structure
 */
Color color_from_hsv(float h, float s, float v, float a);

/**
 * @brief Convert color to HSV
 *
 * @param color Input color
 * @param h Output hue (0.0-360.0)
 * @param s Output saturation (0.0-1.0)
 * @param v Output value (0.0-1.0)
 */
void color_to_hsv(const Color *color, float *h, float *s, float *v);

/**
 * @brief Blend two colors
 *
 * @param c1 First color
 * @param c2 Second color
 * @param factor Blend factor (0.0 = c1, 1.0 = c2)
 * @return Blended color
 */
Color color_blend(const Color *c1, const Color *c2, float factor);

/**
 * @brief Linear interpolation between two colors
 *
 * @param c1 Start color
 * @param c2 End color
 * @param t Interpolation factor (0.0-1.0)
 * @return Interpolated color
 */
Color color_lerp(const Color *c1, const Color *c2, float t);

/**
 * @brief Get color as float array
 *
 * @param color Input color
 * @param array Output array (must be at least 4 floats)
 */
void color_to_array(const Color *color, float *array);

/**
 * @brief Create color from array
 *
 * @param array Input array (r, g, b, a)
 * @return Color structure
 */
Color color_from_array(const float *array);

// Predefined colors
extern const Color COLOR_RED;
extern const Color COLOR_GREEN;
extern const Color COLOR_BLUE;
extern const Color COLOR_WHITE;
extern const Color COLOR_BLACK;
extern const Color COLOR_GRAY;
extern const Color COLOR_YELLOW;
extern const Color COLOR_CYAN;
extern const Color COLOR_MAGENTA;
extern const Color COLOR_ORANGE;
extern const Color COLOR_PURPLE;

#endif // COLOR_H