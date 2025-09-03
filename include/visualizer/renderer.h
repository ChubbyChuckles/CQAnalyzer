#ifndef RENDERER_H
#define RENDERER_H

#include "cqanalyzer.h"
#include "visualizer/color.h"
#include "visualizer/gradient.h"
#include "visualizer/text_renderer.h"
#include "visualizer/lighting.h"

/**
 * @file renderer.h
 * @brief 3D rendering functionality
 *
 * Provides functions to initialize and manage 3D rendering
 * using OpenGL and GLFW.
 */

/**
 * @brief Initialize 3D renderer
 *
 * @param width Window width
 * @param height Window height
 * @param title Window title
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError renderer_init(int width, int height, const char *title);

/**
 * @brief Shutdown 3D renderer
 */
void renderer_shutdown(void);

/**
 * @brief Check if renderer is running
 *
 * @return true if running, false otherwise
 */
bool renderer_is_running(void);

/**
 * @brief Update renderer (handle events, etc.)
 */
void renderer_update(void);

/**
 * @brief Render current scene
 */
void renderer_render(void);

/**
 * @brief Swap buffers and present frame
 */
void renderer_present(void);

/**
 * @brief Handle mouse scroll for camera zoom
 *
 * @param x_offset Scroll X offset
 * @param y_offset Scroll Y offset
 */
void renderer_handle_scroll(double x_offset, double y_offset);

/**
 * @brief Draw a 3D cube
 *
 * @param x X position
 * @param y Y position
 * @param z Z position
 * @param size Cube size
 * @param r Red color component (0.0-1.0)
 * @param g Green color component (0.0-1.0)
 * @param b Blue color component (0.0-1.0)
 */
void renderer_draw_cube(float x, float y, float z, float size, float r, float g, float b);

/**
 * @brief Draw a 3D cube with Color
 *
 * @param x X position
 * @param y Y position
 * @param z Z position
 * @param size Cube size
 * @param color Color structure
 */
void renderer_draw_cube_color(float x, float y, float z, float size, const Color *color);

/**
 * @brief Draw a 3D sphere
 *
 * @param x X position
 * @param y Y position
 * @param z Z position
 * @param radius Sphere radius
 * @param r Red color component (0.0-1.0)
 * @param g Green color component (0.0-1.0)
 * @param b Blue color component (0.0-1.0)
 */
void renderer_draw_sphere(float x, float y, float z, float radius, float r, float g, float b);

/**
 * @brief Draw a 3D sphere with Color
 *
 * @param x X position
 * @param y Y position
 * @param z Z position
 * @param radius Sphere radius
 * @param color Color structure
 */
void renderer_draw_sphere_color(float x, float y, float z, float radius, const Color *color);

/**
 * @brief Draw a 3D line
 *
 * @param x1 Start X position
 * @param y1 Start Y position
 * @param z1 Start Z position
 * @param x2 End X position
 * @param y2 End Y position
 * @param z2 End Z position
 * @param r Red color component (0.0-1.0)
 * @param g Green color component (0.0-1.0)
 * @param b Blue color component (0.0-1.0)
 */
void renderer_draw_line(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b);

/**
 * @brief Draw a 3D line with Color
 *
 * @param x1 Start X position
 * @param y1 Start Y position
 * @param z1 Start Z position
 * @param x2 End X position
 * @param y2 End Y position
 * @param z2 End Z position
 * @param color Color structure
 */
void renderer_draw_line_color(float x1, float y1, float z1, float x2, float y2, float z2, const Color *color);

/**
 * @brief Draw a gradient line
 *
 * @param x1 Start X position
 * @param y1 Start Y position
 * @param z1 Start Z position
 * @param x2 End X position
 * @param y2 End Y position
 * @param z2 End Z position
 * @param gradient Gradient to use for coloring
 */
void renderer_draw_line_gradient(float x1, float y1, float z1, float x2, float y2, float z2, const Gradient *gradient);

/**
 * @brief Initialize text rendering system
 *
 * @param font_path Path to font file
 * @param font_size Font size in pixels
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError renderer_init_text(const char *font_path, int font_size);

/**
 * @brief Shutdown text rendering system
 */
void renderer_shutdown_text(void);

/**
 * @brief Render text at screen coordinates
 *
 * @param text Text to render
 * @param x X position (screen coordinates)
 * @param y Y position (screen coordinates)
 * @param scale Text scale
 * @param color Text color
 */
void renderer_draw_text(const char *text, float x, float y, float scale, const Color *color);

/**
 * @brief Render text in 3D world space
 *
 * @param text Text to render
 * @param x X position (world coordinates)
 * @param y Y position (world coordinates)
 * @param z Z position (world coordinates)
 * @param scale Text scale
 * @param color Text color
 */
void renderer_draw_text_3d(const char *text, float x, float y, float z, float scale, const Color *color);

/**
 * @brief Draw a 3D cube with lighting and materials
 *
 * @param x X position
 * @param y Y position
 * @param z Z position
 * @param size Cube size
 * @param material Material properties
 * @param light Light source
 */
void renderer_draw_cube_lit(float x, float y, float z, float size, const Material *material, const Light *light);

/**
 * @brief Draw a 3D sphere with lighting and materials
 *
 * @param x X position
 * @param y Y position
 * @param z Z position
 * @param radius Sphere radius
 * @param material Material properties
 * @param light Light source
 */
void renderer_draw_sphere_lit(float x, float y, float z, float radius, const Material *material, const Light *light);

/**
 * @brief Take a screenshot of the current frame
 *
 * @param filename Output filename for the screenshot
 */
void renderer_take_screenshot(const char *filename);

/**
 * @brief Start video recording
 *
 * @param filename_pattern Pattern for frame filenames (e.g., "frame_%04d.bmp")
 */
void renderer_start_video_recording(const char *filename_pattern);

/**
 * @brief Stop video recording
 */
void renderer_stop_video_recording(void);

#endif // RENDERER_H
