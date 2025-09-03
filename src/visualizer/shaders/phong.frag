#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

out vec4 FragColor;

uniform vec3 viewPos;

// Material properties
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

// Light properties
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;
uniform vec3 lightDirection;
uniform vec3 lightPosition;
uniform int lightType; // 0 = directional, 1 = point, 2 = spot
uniform float lightConstant;
uniform float lightLinear;
uniform float lightQuadratic;
uniform float lightCutoff;
uniform float lightOuterCutoff;
uniform bool lightEnabled;

vec3 calculateDirectionalLight(vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-lightDirection);

    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);

    // Combine results
    vec3 ambient = lightAmbient * materialAmbient;
    vec3 diffuse = lightDiffuse * diff * materialDiffuse;
    vec3 specular = lightSpecular * spec * materialSpecular;

    return ambient + diffuse + specular;
}

vec3 calculatePointLight(vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(lightPosition - fragPos);

    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);

    // Attenuation
    float distance = length(lightPosition - fragPos);
    float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));

    // Combine results
    vec3 ambient = lightAmbient * materialAmbient;
    vec3 diffuse = lightDiffuse * diff * materialDiffuse;
    vec3 specular = lightSpecular * spec * materialSpecular;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

vec3 calculateSpotLight(vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(lightPosition - fragPos);

    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);

    // Attenuation
    float distance = length(lightPosition - fragPos);
    float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * (distance * distance));

    // Spotlight intensity
    float theta = dot(lightDir, normalize(-lightDirection));
    float epsilon = lightCutoff - lightOuterCutoff;
    float intensity = clamp((theta - lightOuterCutoff) / epsilon, 0.0, 1.0);

    // Combine results
    vec3 ambient = lightAmbient * materialAmbient;
    vec3 diffuse = lightDiffuse * diff * materialDiffuse;
    vec3 specular = lightSpecular * spec * materialSpecular;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ambient + diffuse + specular;
}

void main()
{
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);

    if (lightEnabled)
    {
        if (lightType == 0) // Directional light
        {
            result = calculateDirectionalLight(norm, viewDir);
        }
        else if (lightType == 1) // Point light
        {
            result = calculatePointLight(norm, FragPos, viewDir);
        }
        else if (lightType == 2) // Spot light
        {
            result = calculateSpotLight(norm, FragPos, viewDir);
        }
    }
    else
    {
        // No lighting, just use material diffuse color
        result = materialDiffuse * Color;
    }

    FragColor = vec4(result, 1.0);
}