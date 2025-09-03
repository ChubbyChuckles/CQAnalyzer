#ifndef GRADIENT_H
#define GRADIENT_H

#include "visualizer/color.h"

/**
 * @file gradient.h
 * @brief Gradient system for smooth color transitions
 *
 * Provides gradient structures and functions for creating
 * smooth color transitions between multiple colors.
 */

typedef struct
{
    Color start_color;
    Color end_color;
} Gradient;

/**
 * @brief Create a gradient from two colors
 *
 * @param start_color Starting color
 * @param end_color Ending color
 * @return Gradient structure
 */
Gradient gradient_create(const Color *start_color, const Color *end_color);

/**
 * @brief Get color at specific position in gradient
 *
 * @param gradient Input gradient
 * @param t Position in gradient (0.0 = start, 1.0 = end)
 * @return Color at position t
 */
Color gradient_get_color(const Gradient *gradient, float t);

/**
 * @brief Create a rainbow gradient
 *
 * @return Rainbow gradient (red to violet)
 */
Gradient gradient_rainbow(void);

/**
 * @brief Create a heatmap gradient
 *
 * @return Heatmap gradient (blue to red)
 */
Gradient gradient_heatmap(void);

/**
 * @brief Create a grayscale gradient
 *
 * @return Grayscale gradient (black to white)
 */
Gradient gradient_grayscale(void);

/**
 * @brief Create a custom gradient from HSV range
 *
 * @param start_hue Start hue (0.0-360.0)
 * @param end_hue End hue (0.0-360.0)
 * @param saturation Saturation (0.0-1.0)
 * @param value Value (0.0-1.0)
 * @return Custom HSV gradient
 */
Gradient gradient_from_hsv(float start_hue, float end_hue, float saturation, float value);

#endif // GRADIENT_H