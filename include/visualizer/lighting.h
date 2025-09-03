#ifndef LIGHTING_H
#define LIGHTING_H

#include "visualizer/color.h"

/**
 * @file lighting.h
 * @brief Lighting system for 3D visualization
 *
 * Provides structures and functions for managing lights and materials
 * to create depth perception through shading.
 */

typedef enum
{
    LIGHT_DIRECTIONAL,
    LIGHT_POINT,
    LIGHT_SPOT
} LightType;

typedef struct
{
    LightType type;
    Color ambient;
    Color diffuse;
    Color specular;
    float position[3];    // For point and spot lights
    float direction[3];   // For directional and spot lights
    float constant;       // For point and spot lights
    float linear;         // For point and spot lights
    float quadratic;      // For point and spot lights
    float cutoff;         // For spot lights (cosine of angle)
    float outer_cutoff;   // For spot lights (cosine of outer angle)
    bool enabled;
} Light;

typedef struct
{
    Color ambient;
    Color diffuse;
    Color specular;
    float shininess;      // Specular exponent
} Material;

/**
 * @brief Initialize a directional light
 *
 * @param light Light to initialize
 * @param direction Light direction vector
 * @param ambient Ambient color
 * @param diffuse Diffuse color
 * @param specular Specular color
 */
void light_init_directional(Light *light, float direction[3],
                           const Color *ambient, const Color *diffuse, const Color *specular);

/**
 * @brief Initialize a point light
 *
 * @param light Light to initialize
 * @param position Light position
 * @param ambient Ambient color
 * @param diffuse Diffuse color
 * @param specular Specular color
 * @param constant Attenuation constant factor
 * @param linear Attenuation linear factor
 * @param quadratic Attenuation quadratic factor
 */
void light_init_point(Light *light, float position[3],
                     const Color *ambient, const Color *diffuse, const Color *specular,
                     float constant, float linear, float quadratic);

/**
 * @brief Initialize a spot light
 *
 * @param light Light to initialize
 * @param position Light position
 * @param direction Light direction
 * @param ambient Ambient color
 * @param diffuse Diffuse color
 * @param specular Specular color
 * @param constant Attenuation constant factor
 * @param linear Attenuation linear factor
 * @param quadratic Attenuation quadratic factor
 * @param cutoff Inner cutoff angle (degrees)
 * @param outer_cutoff Outer cutoff angle (degrees)
 */
void light_init_spot(Light *light, float position[3], float direction[3],
                    const Color *ambient, const Color *diffuse, const Color *specular,
                    float constant, float linear, float quadratic,
                    float cutoff, float outer_cutoff);

/**
 * @brief Enable or disable a light
 *
 * @param light Light to modify
 * @param enabled Whether the light should be enabled
 */
void light_set_enabled(Light *light, bool enabled);

/**
 * @brief Initialize a material
 *
 * @param material Material to initialize
 * @param ambient Ambient color
 * @param diffuse Diffuse color
 * @param specular Specular color
 * @param shininess Specular shininess factor
 */
void material_init(Material *material,
                  const Color *ambient, const Color *diffuse, const Color *specular,
                  float shininess);

/**
 * @brief Create a default diffuse material
 *
 * @param material Material to initialize
 * @param color Base color for the material
 */
void material_init_diffuse(Material *material, const Color *color);

/**
 * @brief Create a metallic material
 *
 * @param material Material to initialize
 * @param color Base color for the material
 * @param shininess Shininess factor
 */
void material_init_metallic(Material *material, const Color *color, float shininess);

/**
 * @brief Create a plastic material
 *
 * @param material Material to initialize
 * @param color Base color for the material
 * @param shininess Shininess factor
 */
void material_init_plastic(Material *material, const Color *color, float shininess);

#endif // LIGHTING_H