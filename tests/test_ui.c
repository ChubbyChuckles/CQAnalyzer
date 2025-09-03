#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui/progress_display.h"

/**
 * @brief Test progress display initialization
 */
void test_progress_display_init(void)
{
    CU_ASSERT_EQUAL(progress_display_init(), CQ_SUCCESS);
    progress_display_shutdown();
}

/**
 * @brief Test progress display functions
 */
void test_progress_display_functions(void)
{
    CU_ASSERT_EQUAL(progress_display_init(), CQ_SUCCESS);

    // Test progress start
    progress_start("Test Progress", 10);

    // Test progress updates
    for (int i = 1; i <= 10; i++)
    {
        char status[50];
        sprintf(status, "Processing item %d", i);
        progress_update(i, status);
    }

    // Test progress completion
    progress_complete("Test completed successfully");

    progress_display_shutdown();
}

/**
 * @brief Test progress display error/warning/info functions
 */
void test_progress_display_messages(void)
{
    CU_ASSERT_EQUAL(progress_display_init(), CQ_SUCCESS);

    // Test error message
    progress_display_error("Test error message");

    // Test warning message
    progress_display_warning("Test warning message");

    // Test info message
    progress_display_info("Test info message");

    progress_display_shutdown();
}

/**
 * @brief Test progress display with NULL parameters
 */
void test_progress_display_null_params(void)
{
    CU_ASSERT_EQUAL(progress_display_init(), CQ_SUCCESS);

    // Test with NULL title (should not crash)
    progress_start(NULL, 5);
    progress_update(1, "Test");
    progress_complete("Done");

    // Test with NULL status
    progress_start("Test", 3);
    progress_update(1, NULL);
    progress_update(2, NULL);
    progress_complete(NULL);

    progress_display_shutdown();
}

/**
 * @brief Add UI tests to suite
 */
void add_ui_tests(CU_pSuite suite)
{
    CU_add_test(suite, "Progress Display Init Test", test_progress_display_init);
    CU_add_test(suite, "Progress Display Functions Test", test_progress_display_functions);
    CU_add_test(suite, "Progress Display Messages Test", test_progress_display_messages);
    CU_add_test(suite, "Progress Display Null Params Test", test_progress_display_null_params);
}