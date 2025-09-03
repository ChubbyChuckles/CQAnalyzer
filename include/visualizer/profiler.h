#ifndef PROFILER_H
#define PROFILER_H

#include <time.h>
#include <stdbool.h>
#include "cqanalyzer.h"

/**
 * @file profiler.h
 * @brief Performance profiler for debugging 3D visualization system
 *
 * Provides FPS counter, timing measurements, and overlay display
 * for performance monitoring during development and debugging.
 */

/**
 * @brief Performance metrics structure
 */
typedef struct
{
    double fps;                    // Current FPS
    double frame_time_ms;          // Time per frame in milliseconds
    double render_time_ms;         // Rendering time in milliseconds
    double update_time_ms;         // Update time in milliseconds
    double min_frame_time_ms;      // Minimum frame time
    double max_frame_time_ms;      // Maximum frame time
    double avg_frame_time_ms;      // Average frame time
    int frame_count;               // Total frame count
    double total_time_sec;         // Total running time
} PerformanceMetrics;

/**
 * @brief Profiler context
 */
typedef struct
{
    bool enabled;                  // Whether profiler is enabled
    bool overlay_visible;          // Whether overlay is visible
    PerformanceMetrics metrics;    // Current performance metrics
    struct timespec last_frame_time; // Time of last frame
    struct timespec start_time;    // Profiler start time
    double frame_accumulator;      // Accumulator for averaging
    int frame_accumulator_count;   // Number of frames in accumulator
    double render_start_time;      // Start time of current render
    double update_start_time;      // Start time of current update
} Profiler;

/**
 * @brief Initialize performance profiler
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError profiler_init(void);

/**
 * @brief Shutdown performance profiler
 */
void profiler_shutdown(void);

/**
 * @brief Start frame timing
 * Call this at the beginning of each frame
 */
void profiler_start_frame(void);

/**
 * @brief End frame timing
 * Call this at the end of each frame
 */
void profiler_end_frame(void);

/**
 * @brief Start render timing
 * Call this before rendering operations
 */
void profiler_start_render(void);

/**
 * @brief End render timing
 * Call this after rendering operations
 */
void profiler_end_render(void);

/**
 * @brief Start update timing
 * Call this before update operations
 */
void profiler_start_update(void);

/**
 * @brief End update timing
 * Call this after update operations
 */
void profiler_end_update(void);

/**
 * @brief Toggle profiler overlay visibility
 */
void profiler_toggle_overlay(void);

/**
 * @brief Set profiler overlay visibility
 *
 * @param visible Whether overlay should be visible
 */
void profiler_set_overlay_visible(bool visible);

/**
 * @brief Check if profiler overlay is visible
 *
 * @return true if overlay is visible, false otherwise
 */
bool profiler_is_overlay_visible(void);

/**
 * @brief Get current performance metrics
 *
 * @return Pointer to current performance metrics
 */
const PerformanceMetrics* profiler_get_metrics(void);

/**
 * @brief Render profiler overlay
 * Call this after main scene rendering but before buffer swap
 */
void profiler_render_overlay(void);

#endif // PROFILER_H