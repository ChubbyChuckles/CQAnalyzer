#include "visualizer/gradient.h"

Gradient gradient_create(const Color *start_color, const Color *end_color)
{
    Gradient gradient;
    gradient.start_color = *start_color;
    gradient.end_color = *end_color;
    return gradient;
}

Color gradient_get_color(const Gradient *gradient, float t)
{
    // Clamp t to [0, 1]
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    return color_lerp(&gradient->start_color, &gradient->end_color, t);
}

Gradient gradient_rainbow(void)
{
    // Red to violet (rainbow)
    Color start = color_from_hsv(0.0f, 1.0f, 1.0f, 1.0f);   // Red
    Color end = color_from_hsv(270.0f, 1.0f, 1.0f, 1.0f);   // Violet
    return gradient_create(&start, &end);
}

Gradient gradient_heatmap(void)
{
    // Blue to red (heatmap)
    Color start = color_from_hsv(240.0f, 1.0f, 1.0f, 1.0f);  // Blue
    Color end = color_from_hsv(0.0f, 1.0f, 1.0f, 1.0f);     // Red
    return gradient_create(&start, &end);
}

Gradient gradient_grayscale(void)
{
    // Black to white
    return gradient_create(&COLOR_BLACK, &COLOR_WHITE);
}

Gradient gradient_from_hsv(float start_hue, float end_hue, float saturation, float value)
{
    Color start = color_from_hsv(start_hue, saturation, value, 1.0f);
    Color end = color_from_hsv(end_hue, saturation, value, 1.0f);
    return gradient_create(&start, &end);
}