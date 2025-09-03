#include <math.h>
#include "visualizer/color.h"

// Predefined colors
const Color COLOR_RED = {1.0f, 0.0f, 0.0f, 1.0f};
const Color COLOR_GREEN = {0.0f, 1.0f, 0.0f, 1.0f};
const Color COLOR_BLUE = {0.0f, 0.0f, 1.0f, 1.0f};
const Color COLOR_WHITE = {1.0f, 1.0f, 1.0f, 1.0f};
const Color COLOR_BLACK = {0.0f, 0.0f, 0.0f, 1.0f};
const Color COLOR_GRAY = {0.5f, 0.5f, 0.5f, 1.0f};
const Color COLOR_YELLOW = {1.0f, 1.0f, 0.0f, 1.0f};
const Color COLOR_CYAN = {0.0f, 1.0f, 1.0f, 1.0f};
const Color COLOR_MAGENTA = {1.0f, 0.0f, 1.0f, 1.0f};
const Color COLOR_ORANGE = {1.0f, 0.5f, 0.0f, 1.0f};
const Color COLOR_PURPLE = {0.5f, 0.0f, 0.5f, 1.0f};

Color color_create(float r, float g, float b, float a)
{
    Color color = {r, g, b, a};
    return color;
}

Color color_from_hsv(float h, float s, float v, float a)
{
    Color color;
    color.a = a;

    if (s == 0.0f)
    {
        // Achromatic (gray)
        color.r = color.g = color.b = v;
    }
    else
    {
        h /= 60.0f; // sector 0 to 5
        int i = (int)floorf(h);
        float f = h - i; // factorial part of h
        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));

        switch (i)
        {
        case 0:
            color.r = v;
            color.g = t;
            color.b = p;
            break;
        case 1:
            color.r = q;
            color.g = v;
            color.b = p;
            break;
        case 2:
            color.r = p;
            color.g = v;
            color.b = t;
            break;
        case 3:
            color.r = p;
            color.g = q;
            color.b = v;
            break;
        case 4:
            color.r = t;
            color.g = p;
            color.b = v;
            break;
        default: // case 5:
            color.r = v;
            color.g = p;
            color.b = q;
            break;
        }
    }

    return color;
}

void color_to_hsv(const Color *color, float *h, float *s, float *v)
{
    float max = fmaxf(fmaxf(color->r, color->g), color->b);
    float min = fminf(fminf(color->r, color->g), color->b);
    float delta = max - min;

    *v = max;

    if (max != 0.0f)
    {
        *s = delta / max;
    }
    else
    {
        *s = 0.0f;
        *h = 0.0f;
        return;
    }

    if (delta == 0.0f)
    {
        *h = 0.0f;
    }
    else if (max == color->r)
    {
        *h = (color->g - color->b) / delta;
    }
    else if (max == color->g)
    {
        *h = 2.0f + (color->b - color->r) / delta;
    }
    else
    {
        *h = 4.0f + (color->r - color->g) / delta;
    }

    *h *= 60.0f;
    if (*h < 0.0f)
    {
        *h += 360.0f;
    }
}

Color color_blend(const Color *c1, const Color *c2, float factor)
{
    Color result;
    result.r = c1->r + (c2->r - c1->r) * factor;
    result.g = c1->g + (c2->g - c1->g) * factor;
    result.b = c1->b + (c2->b - c1->b) * factor;
    result.a = c1->a + (c2->a - c1->a) * factor;
    return result;
}

Color color_lerp(const Color *c1, const Color *c2, float t)
{
    return color_blend(c1, c2, t);
}

void color_to_array(const Color *color, float *array)
{
    array[0] = color->r;
    array[1] = color->g;
    array[2] = color->b;
    array[3] = color->a;
}

Color color_from_array(const float *array)
{
    return color_create(array[0], array[1], array[2], array[3]);
}