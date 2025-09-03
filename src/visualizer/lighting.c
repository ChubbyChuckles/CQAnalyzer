#include <math.h>
#include <string.h>
#include "visualizer/lighting.h"

void light_init_directional(Light *light, float direction[3],
                           const Color *ambient, const Color *diffuse, const Color *specular)
{
    if (!light || !direction || !ambient || !diffuse || !specular)
    {
        return;
    }

    light->type = LIGHT_DIRECTIONAL;
    light->ambient = *ambient;
    light->diffuse = *diffuse;
    light->specular = *specular;

    // Normalize direction vector
    float length = sqrtf(direction[0] * direction[0] +
                        direction[1] * direction[1] +
                        direction[2] * direction[2]);
    if (length > 0.0f)
    {
        light->direction[0] = direction[0] / length;
        light->direction[1] = direction[1] / length;
        light->direction[2] = direction[2] / length;
    }
    else
    {
        // Default direction if zero vector provided
        light->direction[0] = 0.0f;
        light->direction[1] = -1.0f;
        light->direction[2] = 0.0f;
    }

    light->enabled = true;
}

void light_init_point(Light *light, float position[3],
                     const Color *ambient, const Color *diffuse, const Color *specular,
                     float constant, float linear, float quadratic)
{
    if (!light || !position || !ambient || !diffuse || !specular)
    {
        return;
    }

    light->type = LIGHT_POINT;
    light->ambient = *ambient;
    light->diffuse = *diffuse;
    light->specular = *specular;

    light->position[0] = position[0];
    light->position[1] = position[1];
    light->position[2] = position[2];

    light->constant = constant;
    light->linear = linear;
    light->quadratic = quadratic;

    light->enabled = true;
}

void light_init_spot(Light *light, float position[3], float direction[3],
                    const Color *ambient, const Color *diffuse, const Color *specular,
                    float constant, float linear, float quadratic,
                    float cutoff, float outer_cutoff)
{
    if (!light || !position || !direction || !ambient || !diffuse || !specular)
    {
        return;
    }

    light->type = LIGHT_SPOT;
    light->ambient = *ambient;
    light->diffuse = *diffuse;
    light->specular = *specular;

    light->position[0] = position[0];
    light->position[1] = position[1];
    light->position[2] = position[2];

    // Normalize direction vector
    float length = sqrtf(direction[0] * direction[0] +
                        direction[1] * direction[1] +
                        direction[2] * direction[2]);
    if (length > 0.0f)
    {
        light->direction[0] = direction[0] / length;
        light->direction[1] = direction[1] / length;
        light->direction[2] = direction[2] / length;
    }
    else
    {
        // Default direction if zero vector provided
        light->direction[0] = 0.0f;
        light->direction[1] = -1.0f;
        light->direction[2] = 0.0f;
    }

    light->constant = constant;
    light->linear = linear;
    light->quadratic = quadratic;

    // Convert degrees to cosine values for cutoff angles
    light->cutoff = cosf(cutoff * M_PI / 180.0f);
    light->outer_cutoff = cosf(outer_cutoff * M_PI / 180.0f);

    light->enabled = true;
}

void light_set_enabled(Light *light, bool enabled)
{
    if (!light)
    {
        return;
    }

    light->enabled = enabled;
}

void material_init(Material *material,
                  const Color *ambient, const Color *diffuse, const Color *specular,
                  float shininess)
{
    if (!material || !ambient || !diffuse || !specular)
    {
        return;
    }

    material->ambient = *ambient;
    material->diffuse = *diffuse;
    material->specular = *specular;
    material->shininess = shininess;
}

void material_init_diffuse(Material *material, const Color *color)
{
    if (!material || !color)
    {
        return;
    }

    // For diffuse materials, ambient is usually darker than diffuse
    Color ambient = {color->r * 0.3f, color->g * 0.3f, color->b * 0.3f, color->a};
    Color diffuse = *color;
    Color specular = {0.1f, 0.1f, 0.1f, 1.0f}; // Low specular for diffuse materials

    material_init(material, &ambient, &diffuse, &specular, 32.0f);
}

void material_init_metallic(Material *material, const Color *color, float shininess)
{
    if (!material || !color)
    {
        return;
    }

    // For metallic materials, ambient and diffuse are similar, high specular
    Color ambient = {color->r * 0.2f, color->g * 0.2f, color->b * 0.2f, color->a};
    Color diffuse = {color->r * 0.8f, color->g * 0.8f, color->b * 0.8f, color->a};
    Color specular = {0.8f, 0.8f, 0.8f, 1.0f}; // High specular for metallic materials

    material_init(material, &ambient, &diffuse, &specular, shininess);
}

void material_init_plastic(Material *material, const Color *color, float shininess)
{
    if (!material || !color)
    {
        return;
    }

    // For plastic materials, moderate specular
    Color ambient = {color->r * 0.1f, color->g * 0.1f, color->b * 0.1f, color->a};
    Color diffuse = *color;
    Color specular = {0.5f, 0.5f, 0.5f, 1.0f}; // Moderate specular for plastic materials

    material_init(material, &ambient, &diffuse, &specular, shininess);
}