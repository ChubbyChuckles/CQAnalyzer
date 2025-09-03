#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "visualizer/renderer.h"

/**
 * @brief Test renderer initialization
 */
void test_renderer_init(void)
{
    // Test successful initialization
    CU_ASSERT_EQUAL(renderer_init(800, 600, "Test Window"), CQ_SUCCESS);

    // Test that renderer is running after initialization
    CU_ASSERT_TRUE(renderer_is_running());

    // Shutdown renderer
    renderer_shutdown();
}

/**
 * @brief Test renderer initialization with invalid parameters
 */
void test_renderer_init_invalid_params(void)
{
    // Test with zero width
    CU_ASSERT_EQUAL(renderer_init(0, 600, "Test Window"), CQ_ERROR_UNKNOWN);

    // Test with zero height
    CU_ASSERT_EQUAL(renderer_init(800, 0, "Test Window"), CQ_ERROR_UNKNOWN);

    // Test with NULL title
    CU_ASSERT_EQUAL(renderer_init(800, 600, NULL), CQ_ERROR_UNKNOWN);
}

/**
 * @brief Test renderer shutdown
 */
void test_renderer_shutdown(void)
{
    // Initialize renderer
    CU_ASSERT_EQUAL(renderer_init(800, 600, "Test Window"), CQ_SUCCESS);

    // Shutdown renderer
    renderer_shutdown();

    // Test that renderer is not running after shutdown
    CU_ASSERT_FALSE(renderer_is_running());
}

/**
 * @brief Test multiple initialization/shutdown cycles
 */
void test_renderer_multiple_init_shutdown(void)
{
    // First cycle
    CU_ASSERT_EQUAL(renderer_init(800, 600, "Test Window"), CQ_SUCCESS);
    CU_ASSERT_TRUE(renderer_is_running());
    renderer_shutdown();
    CU_ASSERT_FALSE(renderer_is_running());

    // Second cycle
    CU_ASSERT_EQUAL(renderer_init(1024, 768, "Test Window 2"), CQ_SUCCESS);
    CU_ASSERT_TRUE(renderer_is_running());
    renderer_shutdown();
    CU_ASSERT_FALSE(renderer_is_running());
}

/**
 * @brief Test renderer functions when not initialized
 */
void test_renderer_not_initialized(void)
{
    // Ensure renderer is not initialized
    renderer_shutdown();

    // Test that functions handle uninitialized state gracefully
    CU_ASSERT_FALSE(renderer_is_running());

    // These should not crash but may not do anything meaningful
    renderer_update();
    renderer_render();
    renderer_present();
}

/**
 * @brief Test basic drawing functions
 */
void test_renderer_drawing_functions(void)
{
    // Initialize renderer
    CU_ASSERT_EQUAL(renderer_init(800, 600, "Test Window"), CQ_SUCCESS);

    // Test drawing functions (these should not crash)
    renderer_draw_cube(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);
    renderer_draw_sphere(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
    renderer_draw_line(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);

    // Shutdown renderer
    renderer_shutdown();
}

/**
 * @brief Test screenshot functionality
 */
void test_renderer_screenshot(void)
{
    // Initialize renderer
    CU_ASSERT_EQUAL(renderer_init(800, 600, "Test Window"), CQ_SUCCESS);

    // Test screenshot (this may fail in headless environment but shouldn't crash)
    renderer_take_screenshot("test_screenshot.bmp");

    // Shutdown renderer
    renderer_shutdown();
}

/**
 * @brief Test video recording functionality
 */
void test_renderer_video_recording(void)
{
    // Initialize renderer
    CU_ASSERT_EQUAL(renderer_init(800, 600, "Test Window"), CQ_SUCCESS);

    // Test video recording start/stop
    renderer_start_video_recording("test_frame_%04d.bmp");
    renderer_stop_video_recording();

    // Shutdown renderer
    renderer_shutdown();
}

/**
 * @brief Add renderer tests to suite
 */
void add_renderer_tests(CU_pSuite suite)
{
    CU_add_test(suite, "Renderer Init Test", test_renderer_init);
    CU_add_test(suite, "Renderer Init Invalid Params Test", test_renderer_init_invalid_params);
    CU_add_test(suite, "Renderer Shutdown Test", test_renderer_shutdown);
    CU_add_test(suite, "Renderer Multiple Init/Shutdown Test", test_renderer_multiple_init_shutdown);
    CU_add_test(suite, "Renderer Not Initialized Test", test_renderer_not_initialized);
    CU_add_test(suite, "Renderer Drawing Functions Test", test_renderer_drawing_functions);
    CU_add_test(suite, "Renderer Screenshot Test", test_renderer_screenshot);
    CU_add_test(suite, "Renderer Video Recording Test", test_renderer_video_recording);
}