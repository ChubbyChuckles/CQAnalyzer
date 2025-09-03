#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h> // for usleep

#include "visualizer/profiler.h"
#include "utils/logger.h"

/**
 * @brief Test profiler initialization and shutdown
 */
void test_profiler_init_shutdown(void)
{
    printf("Testing profiler initialization and shutdown...\n");

    // Test initialization
    CQError result = profiler_init();
    assert(result == CQ_SUCCESS);
    assert(profiler_is_overlay_visible() == false);

    // Test shutdown
    profiler_shutdown();

    printf("✓ Profiler init/shutdown test passed\n");
}

/**
 * @brief Test profiler timing functions
 */
void test_profiler_timing(void)
{
    printf("Testing profiler timing functions...\n");

    // Initialize profiler
    CQError result = profiler_init();
    assert(result == CQ_SUCCESS);

    // Test frame timing
    profiler_start_frame();
    usleep(10000); // Sleep for 10ms
    profiler_end_frame();

    const PerformanceMetrics* metrics = profiler_get_metrics();
    assert(metrics != NULL);
    assert(metrics->frame_count == 1);
    assert(metrics->frame_time_ms >= 10.0); // Should be at least 10ms
    assert(metrics->fps > 0.0);

    // Test render timing
    profiler_start_render();
    usleep(5000); // Sleep for 5ms
    profiler_end_render();

    assert(metrics->render_time_ms >= 5.0);

    // Test update timing
    profiler_start_update();
    usleep(3000); // Sleep for 3ms
    profiler_end_update();

    assert(metrics->update_time_ms >= 3.0);

    // Test multiple frames
    for (int i = 0; i < 5; i++)
    {
        profiler_start_frame();
        usleep(8000); // Sleep for 8ms
        profiler_end_frame();
    }

    assert(metrics->frame_count == 6); // 1 + 5
    assert(metrics->min_frame_time_ms <= metrics->max_frame_time_ms);

    profiler_shutdown();

    printf("✓ Profiler timing test passed\n");
}

/**
 * @brief Test profiler overlay toggle
 */
void test_profiler_overlay_toggle(void)
{
    printf("Testing profiler overlay toggle...\n");

    // Initialize profiler
    CQError result = profiler_init();
    assert(result == CQ_SUCCESS);

    // Initially overlay should be hidden
    assert(profiler_is_overlay_visible() == false);

    // Toggle overlay on
    profiler_toggle_overlay();
    assert(profiler_is_overlay_visible() == true);

    // Toggle overlay off
    profiler_toggle_overlay();
    assert(profiler_is_overlay_visible() == false);

    // Test direct setting
    profiler_set_overlay_visible(true);
    assert(profiler_is_overlay_visible() == true);

    profiler_set_overlay_visible(false);
    assert(profiler_is_overlay_visible() == false);

    profiler_shutdown();

    printf("✓ Profiler overlay toggle test passed\n");
}

/**
 * @brief Test profiler metrics calculation
 */
void test_profiler_metrics(void)
{
    printf("Testing profiler metrics calculation...\n");

    // Initialize profiler
    CQError result = profiler_init();
    assert(result == CQ_SUCCESS);

    // Simulate some frames with known timing
    for (int i = 0; i < 10; i++)
    {
        profiler_start_frame();
        usleep(16667); // Sleep for ~16.67ms (60 FPS)
        profiler_end_frame();
    }

    const PerformanceMetrics* metrics = profiler_get_metrics();
    assert(metrics != NULL);
    assert(metrics->frame_count == 10);
    assert(metrics->fps >= 50.0 && metrics->fps <= 70.0); // Should be around 60 FPS
    assert(metrics->frame_time_ms >= 15.0 && metrics->frame_time_ms <= 20.0);

    // Test min/max tracking
    assert(metrics->min_frame_time_ms > 0.0);
    assert(metrics->max_frame_time_ms >= metrics->min_frame_time_ms);

    profiler_shutdown();

    printf("✓ Profiler metrics test passed\n");
}

/**
 * @brief Main test function
 */
int main(int argc, char *argv[])
{
    printf("Running profiler unit tests...\n\n");

    // Disable logging for tests
    // Note: In a real implementation, you'd want to mock or redirect logging

    test_profiler_init_shutdown();
    test_profiler_timing();
    test_profiler_overlay_toggle();
    test_profiler_metrics();

    printf("\n✓ All profiler tests passed!\n");
    return 0;
}