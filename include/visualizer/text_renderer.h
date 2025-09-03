#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include "cqanalyzer.h"
#include "visualizer/color.h"

/**
 * @file text_renderer.h
 * @brief Text rendering functionality using FreeType
 *
 * Provides functions to render text in 3D space using FreeType
 * for font loading and OpenGL for rendering.
 */

/**
 * @brief Character glyph information
 */
typedef struct
{
    unsigned int texture_id;  // OpenGL texture ID
    int width;               // Glyph width
    int height;              // Glyph height
    int bearing_x;           // Left side bearing
    int bearing_y;           // Top side bearing
    long advance;            // Advance to next glyph
} Character;

/**
 * @brief Text renderer context
 */
typedef struct
{
    FT_Library ft;                    // FreeType library
    FT_Face face;                     // FreeType face
    Character characters[128];        // ASCII character cache
    unsigned int vao;                 // Vertex array object
    unsigned int vbo;                 // Vertex buffer object
    unsigned int shader_program;      // Text shader program
} TextRenderer;

/**
 * @brief Initialize text renderer
 *
 * @param renderer Text renderer instance
 * @param font_path Path to font file
 * @param font_size Font size in pixels
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError text_renderer_init(TextRenderer *renderer, const char *font_path, int font_size);

/**
 * @brief Shutdown text renderer
 *
 * @param renderer Text renderer instance
 */
void text_renderer_shutdown(TextRenderer *renderer);

/**
 * @brief Render text at screen coordinates
 *
 * @param renderer Text renderer instance
 * @param text Text to render
 * @param x X position (screen coordinates)
 * @param y Y position (screen coordinates)
 * @param scale Text scale
 * @param color Text color
 */
void text_renderer_render_text(TextRenderer *renderer, const char *text, float x, float y, float scale, const Color *color);

/**
 * @brief Render text in 3D world space
 *
 * @param renderer Text renderer instance
 * @param text Text to render
 * @param x X position (world coordinates)
 * @param y Y position (world coordinates)
 * @param z Z position (world coordinates)
 * @param scale Text scale
 * @param color Text color
 */
void text_renderer_render_text_3d(TextRenderer *renderer, const char *text, float x, float y, float z, float scale, const Color *color);

#endif // TEXT_RENDERER_H