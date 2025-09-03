#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "visualizer/text_renderer.h"
#include "visualizer/camera.h"
#include "utils/logger.h"

static Camera *text_camera = NULL;  // Reference to main camera for 3D text

CQError text_renderer_init(TextRenderer *renderer, const char *font_path, int font_size)
{
    if (!renderer || !font_path)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    LOG_INFO("Initializing text renderer with font: %s, size: %d", font_path, font_size);

    // Initialize FreeType
    if (FT_Init_FreeType(&renderer->ft))
    {
        LOG_ERROR("Failed to initialize FreeType library");
        return CQ_ERROR_UNKNOWN;
    }

    // Load font face
    if (FT_New_Face(renderer->ft, font_path, 0, &renderer->face))
    {
        LOG_ERROR("Failed to load font: %s", font_path);
        FT_Done_FreeType(renderer->ft);
        return CQ_ERROR_UNKNOWN;
    }

    // Set font size
    FT_Set_Pixel_Sizes(renderer->face, 0, font_size);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 ASCII characters
    for (unsigned char c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(renderer->face, c, FT_LOAD_RENDER))
        {
            LOG_WARNING("Failed to load glyph for character: %c", c);
            continue;
        }

        // Generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            renderer->face->glyph->bitmap.width,
            renderer->face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            renderer->face->glyph->bitmap.buffer
        );

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Store character info
        Character character = {
            .texture_id = texture,
            .width = renderer->face->glyph->bitmap.width,
            .height = renderer->face->glyph->bitmap.rows,
            .bearing_x = renderer->face->glyph->bitmap_left,
            .bearing_y = renderer->face->glyph->bitmap_top,
            .advance = renderer->face->glyph->advance.x
        };
        renderer->characters[c] = character;
    }

    // Create shader program for text rendering
    const char *vertex_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
        "out vec2 TexCoords;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
        "    TexCoords = vertex.zw;\n"
        "}\0";

    const char *fragment_shader_source =
        "#version 330 core\n"
        "in vec2 TexCoords;\n"
        "out vec4 color;\n"
        "uniform sampler2D text;\n"
        "uniform vec3 textColor;\n"
        "void main()\n"
        "{\n"
        "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
        "    color = vec4(textColor, 1.0) * sampled;\n"
        "}\0";

    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    renderer->shader_program = glCreateProgram();
    glAttachShader(renderer->shader_program, vertex_shader);
    glAttachShader(renderer->shader_program, fragment_shader);
    glLinkProgram(renderer->shader_program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Create VAO/VBO for text rendering
    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo);
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    LOG_INFO("Text renderer initialized successfully");
    return CQ_SUCCESS;
}

void text_renderer_shutdown(TextRenderer *renderer)
{
    if (!renderer)
    {
        return;
    }

    LOG_INFO("Shutting down text renderer");

    // Clean up textures
    for (int i = 0; i < 128; i++)
    {
        if (renderer->characters[i].texture_id != 0)
        {
            glDeleteTextures(1, &renderer->characters[i].texture_id);
        }
    }

    // Clean up OpenGL objects
    if (renderer->vao != 0)
    {
        glDeleteVertexArrays(1, &renderer->vao);
    }
    if (renderer->vbo != 0)
    {
        glDeleteBuffers(1, &renderer->vbo);
    }
    if (renderer->shader_program != 0)
    {
        glDeleteProgram(renderer->shader_program);
    }

    // Clean up FreeType
    if (renderer->face)
    {
        FT_Done_Face(renderer->face);
    }
    if (renderer->ft)
    {
        FT_Done_FreeType(renderer->ft);
    }

    LOG_INFO("Text renderer shutdown complete");
}

void text_renderer_render_text(TextRenderer *renderer, const char *text, float x, float y, float scale, const Color *color)
{
    if (!renderer || !text || !color)
    {
        return;
    }

    // Enable blending for text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use text shader
    glUseProgram(renderer->shader_program);
    glUniform3f(glGetUniformLocation(renderer->shader_program, "textColor"), color->r, color->g, color->b);

    // Set up orthographic projection for 2D text
    float projection[16] = {
        2.0f/800.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f/600.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(glGetUniformLocation(renderer->shader_program, "projection"), 1, GL_FALSE, projection);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(renderer->vao);

    // Iterate through all characters
    const char *c = text;
    while (*c)
    {
        Character ch = renderer->characters[(unsigned char)*c];

        float xpos = x + ch.bearing_x * scale;
        float ypos = y - (ch.height - ch.bearing_y) * scale;

        float w = ch.width * scale;
        float h = ch.height * scale;

        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.texture_id);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advance to next glyph
        x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels
        c++;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

void text_renderer_render_text_3d(TextRenderer *renderer, const char *text, float x, float y, float z, float scale, const Color *color)
{
    if (!renderer || !text || !color)
    {
        return;
    }

    // For 3D text, we need to project the 3D position to screen space
    // This is a simplified implementation - in a real application you'd want
    // more sophisticated billboarding and depth handling

    // For now, just render as 2D text at the projected position
    // TODO: Implement proper 3D text rendering with billboarding

    // Simple projection (this should be improved)
    float screen_x = x * 100.0f + 400.0f;  // Simple scaling and offset
    float screen_y = y * 100.0f + 300.0f;

    text_renderer_render_text(renderer, text, screen_x, screen_y, scale, color);
}