#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

#include "visualizer/profiler.h"
#include "visualizer/renderer.h"
#include "visualizer/color.h"
#include "utils/logger.h"

static Profiler g_profiler = {0};

/**
 * @brief Get current time in seconds with high precision
 */
static double get_current_time_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

/**
 * @brief Calculate time difference in milliseconds
 */
static double calculate_time_diff_ms(struct timespec start, struct timespec end)
{
    double start_sec = (double)start.tv_sec + (double)start.tv_nsec / 1000000000.0;
    double end_sec = (double)end.tv_sec + (double)end.tv_nsec / 1000000000.0;
    return (end_sec - start_sec) * 1000.0;
}

CQError profiler_init(void)
{
    LOG_INFO("Initializing performance profiler");

    memset(&g_profiler, 0, sizeof(Profiler));
    g_profiler.enabled = true;
    g_profiler.overlay_visible = false;

    // Initialize timing
    clock_gettime(CLOCK_MONOTONIC, &g_profiler.start_time);
    g_profiler.last_frame_time = g_profiler.start_time;

    // Initialize metrics
    g_profiler.metrics.fps = 0.0;
    g_profiler.metrics.frame_time_ms = 0.0;
    g_profiler.metrics.render_time_ms = 0.0;
    g_profiler.metrics.update_time_ms = 0.0;
    g_profiler.metrics.min_frame_time_ms = INFINITY;
    g_profiler.metrics.max_frame_time_ms = 0.0;
    g_profiler.metrics.avg_frame_time_ms = 0.0;
    g_profiler.metrics.frame_count = 0;
    g_profiler.metrics.total_time_sec = 0.0;

    LOG_INFO("Performance profiler initialized successfully");
    return CQ_SUCCESS;
}

void profiler_shutdown(void)
{
    LOG_INFO("Shutting down performance profiler");
    memset(&g_profiler, 0, sizeof(Profiler));
}

void profiler_start_frame(void)
{
    if (!g_profiler.enabled)
        return;

    clock_gettime(CLOCK_MONOTONIC, &g_profiler.last_frame_time);
}

void profiler_end_frame(void)
{
    if (!g_profiler.enabled)
        return;

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    double frame_time_ms = calculate_time_diff_ms(g_profiler.last_frame_time, current_time);
    double total_time_sec = calculate_time_diff_ms(g_profiler.start_time, current_time) / 1000.0;

    // Update metrics
    g_profiler.metrics.frame_time_ms = frame_time_ms;
    g_profiler.metrics.fps = 1000.0 / frame_time_ms;
    g_profiler.metrics.total_time_sec = total_time_sec;
    g_profiler.metrics.frame_count++;

    // Update min/max
    if (frame_time_ms < g_profiler.metrics.min_frame_time_ms)
        g_profiler.metrics.min_frame_time_ms = frame_time_ms;
    if (frame_time_ms > g_profiler.metrics.max_frame_time_ms)
        g_profiler.metrics.max_frame_time_ms = frame_time_ms;

    // Update rolling average
    g_profiler.frame_accumulator += frame_time_ms;
    g_profiler.frame_accumulator_count++;
    if (g_profiler.frame_accumulator_count >= 60) // Update average every 60 frames
    {
        g_profiler.metrics.avg_frame_time_ms = g_profiler.frame_accumulator / g_profiler.frame_accumulator_count;
        g_profiler.frame_accumulator = 0.0;
        g_profiler.frame_accumulator_count = 0;
    }
}

void profiler_start_render(void)
{
    if (!g_profiler.enabled)
        return;

    g_profiler.render_start_time = get_current_time_seconds();
}

void profiler_end_render(void)
{
    if (!g_profiler.enabled)
        return;

    double end_time = get_current_time_seconds();
    g_profiler.metrics.render_time_ms = (end_time - g_profiler.render_start_time) * 1000.0;
}

void profiler_start_update(void)
{
    if (!g_profiler.enabled)
        return;

    g_profiler.update_start_time = get_current_time_seconds();
}

void profiler_end_update(void)
{
    if (!g_profiler.enabled)
        return;

    double end_time = get_current_time_seconds();
    g_profiler.metrics.update_time_ms = (end_time - g_profiler.update_start_time) * 1000.0;
}

void profiler_toggle_overlay(void)
{
    g_profiler.overlay_visible = !g_profiler.overlay_visible;
    LOG_INFO("Profiler overlay %s", g_profiler.overlay_visible ? "enabled" : "disabled");
}

void profiler_set_overlay_visible(bool visible)
{
    g_profiler.overlay_visible = visible;
}

bool profiler_is_overlay_visible(void)
{
    return g_profiler.overlay_visible;
}

const PerformanceMetrics* profiler_get_metrics(void)
{
    return &g_profiler.metrics;
}

void profiler_render_overlay(void)
{
    if (!g_profiler.overlay_visible || !g_profiler.enabled)
        return;

    const PerformanceMetrics* metrics = &g_profiler.metrics;

    // Define colors
    Color white = COLOR_WHITE;
    Color green = COLOR_GREEN;
    Color yellow = COLOR_YELLOW;
    Color red = COLOR_RED;

    // Background rectangle (semi-transparent black)
    Color bg_color = color_create(0.0f, 0.0f, 0.0f, 0.7f);

    // Position overlay in top-left corner
    float x = 10.0f;
    float y = 10.0f;
    float line_height = 20.0f;
    float scale = 0.5f;

    char buffer[256];

    // Title
    renderer_draw_text("Performance Profiler", x, y, scale, &white);
    y += line_height * 1.5f;

    // FPS
    Color fps_color = (metrics->fps >= 60.0f) ? green :
                     (metrics->fps >= 30.0f) ? yellow : red;
    snprintf(buffer, sizeof(buffer), "FPS: %.1f", metrics->fps);
    renderer_draw_text(buffer, x, y, scale, &fps_color);
    y += line_height;

    // Frame time
    Color frame_color = (metrics->frame_time_ms <= 16.67f) ? green :
                       (metrics->frame_time_ms <= 33.33f) ? yellow : red;
    snprintf(buffer, sizeof(buffer), "Frame Time: %.2f ms", metrics->frame_time_ms);
    renderer_draw_text(buffer, x, y, scale, &frame_color);
    y += line_height;

    // Render time
    snprintf(buffer, sizeof(buffer), "Render Time: %.2f ms", metrics->render_time_ms);
    renderer_draw_text(buffer, x, y, scale, &white);
    y += line_height;

    // Update time
    snprintf(buffer, sizeof(buffer), "Update Time: %.2f ms", metrics->update_time_ms);
    renderer_draw_text(buffer, x, y, scale, &white);
    y += line_height;

    // Min/Max/Avg frame times
    if (metrics->frame_count > 0)
    {
        snprintf(buffer, sizeof(buffer), "Min Frame: %.2f ms", metrics->min_frame_time_ms);
        renderer_draw_text(buffer, x, y, scale, &white);
        y += line_height;

        snprintf(buffer, sizeof(buffer), "Max Frame: %.2f ms", metrics->max_frame_time_ms);
        renderer_draw_text(buffer, x, y, scale, &white);
        y += line_height;

        if (g_profiler.frame_accumulator_count > 0)
        {
            snprintf(buffer, sizeof(buffer), "Avg Frame: %.2f ms", g_profiler.metrics.avg_frame_time_ms);
            renderer_draw_text(buffer, x, y, scale, &white);
            y += line_height;
        }
    }

    // Frame count and total time
    snprintf(buffer, sizeof(buffer), "Frames: %d", metrics->frame_count);
    renderer_draw_text(buffer, x, y, scale, &white);
    y += line_height;

    int minutes = (int)(metrics->total_time_sec / 60.0);
    double seconds = fmod(metrics->total_time_sec, 60.0);
    snprintf(buffer, sizeof(buffer), "Time: %02d:%05.2f", minutes, seconds);
    renderer_draw_text(buffer, x, y, scale, &white);
    y += line_height;

    // Memory usage (placeholder - would need system-specific implementation)
    snprintf(buffer, sizeof(buffer), "Memory: N/A");
    renderer_draw_text(buffer, x, y, scale, &white);
    y += line_height;

    // Instructions
    y += line_height * 0.5f;
    renderer_draw_text("Press 'P' to toggle overlay", x, y, scale * 0.8f, &yellow);
}