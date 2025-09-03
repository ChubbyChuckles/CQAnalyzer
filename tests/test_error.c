#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils/error.h"
#include "utils/logger.h"

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

// Test helper macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            tests_passed++; \
            printf("✓ %s\n", message); \
        } else { \
            tests_failed++; \
            printf("✗ %s\n", message); \
        } \
    } while (0)

#define TEST_EQUAL(a, b, message) TEST_ASSERT((a) == (b), message)
#define TEST_STR_EQUAL(a, b, message) TEST_ASSERT(strcmp(a, b) == 0, message)
#define TEST_NOT_NULL(ptr, message) TEST_ASSERT((ptr) != NULL, message)

// Error handler for testing
static int error_handler_called = 0;
static CQErrorContext last_error_context;

static void test_error_handler(const CQErrorContext *error)
{
    error_handler_called++;
    memcpy(&last_error_context, error, sizeof(CQErrorContext));
}

// Test functions
void test_error_initialization()
{
    printf("\n--- Testing Error System Initialization ---\n");

    // Test initialization
    CQError result = cq_error_init();
    TEST_EQUAL(result, CQ_SUCCESS, "Error system initialization should succeed");

    // Test shutdown
    cq_error_shutdown();
    TEST_ASSERT(1, "Error system shutdown should complete");
}

void test_error_code_conversion()
{
    printf("\n--- Testing Error Code Conversion ---\n");

    // Test new error codes
    const char *msg = cq_error_code_to_string(CQ_ERROR_FILE_NOT_FOUND);
    TEST_STR_EQUAL(msg, "File not found", "File not found error string");

    msg = cq_error_code_to_string(CQ_ERROR_PARSING_FAILED);
    TEST_STR_EQUAL(msg, "Code parsing failed", "Parsing failed error string");

    // Test legacy compatibility
    msg = cq_error_to_string(CQ_ERROR_FILE_NOT_FOUND);
    TEST_STR_EQUAL(msg, "File not found", "Legacy file not found error string");

    msg = cq_error_to_string(CQ_ERROR_PARSING_FAILED);
    TEST_STR_EQUAL(msg, "Code parsing failed", "Legacy parsing failed error string");
}

void test_error_categories()
{
    printf("\n--- Testing Error Categories ---\n");

    // Test category detection
    CQErrorCategory cat = cq_error_get_category(CQ_ERROR_PARSING_FAILED);
    TEST_EQUAL(cat, ERROR_CATEGORY_PARSING, "Parsing error should be in parsing category");

    cat = cq_error_get_category(CQ_ERROR_ANALYSIS_FAILED);
    TEST_EQUAL(cat, ERROR_CATEGORY_ANALYSIS, "Analysis error should be in analysis category");

    cat = cq_error_get_category(CQ_ERROR_RENDERING_FAILED);
    TEST_EQUAL(cat, ERROR_CATEGORY_VISUALIZATION, "Rendering error should be in visualization category");

    // Test category string conversion
    const char *cat_str = cq_error_category_to_string(ERROR_CATEGORY_PARSING);
    TEST_STR_EQUAL(cat_str, "Parsing", "Parsing category string");

    cat_str = cq_error_category_to_string(ERROR_CATEGORY_ANALYSIS);
    TEST_STR_EQUAL(cat_str, "Analysis", "Analysis category string");
}

void test_error_severity()
{
    printf("\n--- Testing Error Severity ---\n");

    // Test severity detection
    CQErrorSeverity sev = cq_error_get_severity(CQ_ERROR_OUT_OF_MEMORY);
    TEST_EQUAL(sev, ERROR_SEVERITY_CRITICAL, "Out of memory should be critical");

    sev = cq_error_get_severity(CQ_ERROR_FILE_NOT_FOUND);
    TEST_EQUAL(sev, ERROR_SEVERITY_ERROR, "File not found should be error");

    sev = cq_error_get_severity(CQ_ERROR_CONFIG_VALUE_INVALID);
    TEST_EQUAL(sev, ERROR_SEVERITY_WARNING, "Invalid config value should be warning");

    // Test severity string conversion
    const char *sev_str = cq_error_severity_to_string(ERROR_SEVERITY_CRITICAL);
    TEST_STR_EQUAL(sev_str, "Critical", "Critical severity string");

    sev_str = cq_error_severity_to_string(ERROR_SEVERITY_WARNING);
    TEST_STR_EQUAL(sev_str, "Warning", "Warning severity string");
}

void test_error_creation()
{
    printf("\n--- Testing Error Context Creation ---\n");

    // Test basic error creation
    CQErrorContext *error = CQ_ERROR_CREATE(CQ_ERROR_FILE_NOT_FOUND, ERROR_SEVERITY_ERROR, "Test error");
    TEST_NOT_NULL(error, "Error context creation should succeed");

    if (error)
    {
        TEST_EQUAL(error->code, CQ_ERROR_FILE_NOT_FOUND, "Error code should match");
        TEST_EQUAL(error->severity, ERROR_SEVERITY_ERROR, "Error severity should match");
        TEST_STR_EQUAL(error->message, "Test error", "Error message should match");
        TEST_EQUAL(error->category, ERROR_CATEGORY_IO, "Error category should be IO");

        cq_error_free(error);
    }

    // Test formatted error creation
    error = CQ_ERROR_CREATEF(CQ_ERROR_INVALID_ARGUMENT, ERROR_SEVERITY_WARNING,
                           "Invalid value: %d", 42);
    TEST_NOT_NULL(error, "Formatted error context creation should succeed");

    if (error)
    {
        TEST_STR_EQUAL(error->message, "Invalid value: 42", "Formatted error message should match");
        cq_error_free(error);
    }
}

void test_error_context()
{
    printf("\n--- Testing Error Context Management ---\n");

    CQErrorContext *error = CQ_ERROR_CREATE(CQ_ERROR_MEMORY_ALLOCATION, ERROR_SEVERITY_ERROR, "Memory error");

    // Test setting context info
    cq_error_set_context(error, "Additional context information");
    TEST_STR_EQUAL(error->context_info, "Additional context information", "Context info should be set");

    // Test setting recovery suggestion
    cq_error_set_recovery_suggestion(error, "Try freeing some memory");
    TEST_STR_EQUAL(error->recovery_suggestion, "Try freeing some memory", "Recovery suggestion should be set");

    cq_error_free(error);
}

void test_error_reporting()
{
    printf("\n--- Testing Error Reporting ---\n");

    // Set up test error handler
    cq_error_set_handler(test_error_handler);
    error_handler_called = 0;

    // Create and report error
    CQErrorContext *error = CQ_ERROR_CREATE(CQ_ERROR_PARSING_FAILED, ERROR_SEVERITY_ERROR, "Parse failed");
    cq_error_report(error);

    // Check that handler was called
    TEST_EQUAL(error_handler_called, 1, "Error handler should be called once");

    // Check that error context was passed correctly
    TEST_EQUAL(last_error_context.code, CQ_ERROR_PARSING_FAILED, "Reported error code should match");

    cq_error_free(error);
    cq_error_set_handler(NULL); // Reset handler
}

void test_error_formatting()
{
    printf("\n--- Testing Error Message Formatting ---\n");

    CQErrorContext *error = CQ_ERROR_CREATE(CQ_ERROR_FILE_NOT_FOUND, ERROR_SEVERITY_ERROR, "File missing");
    cq_error_set_context(error, "File: test.txt");
    cq_error_set_recovery_suggestion(error, "Check file path");

    char buffer[1024];
    int written = cq_error_format_message(error, buffer, sizeof(buffer));

    TEST_ASSERT(written > 0, "Error message formatting should succeed");
    TEST_ASSERT(strstr(buffer, "File missing") != NULL, "Formatted message should contain error text");
    TEST_ASSERT(strstr(buffer, "test.txt") != NULL, "Formatted message should contain context");
    TEST_ASSERT(strstr(buffer, "Check file path") != NULL, "Formatted message should contain suggestion");

    cq_error_free(error);
}

void test_recovery_suggestions()
{
    printf("\n--- Testing Recovery Suggestions ---\n");

    // Test getting recovery suggestions
    const char *suggestion = cq_error_get_recovery_suggestion(CQ_ERROR_FILE_NOT_FOUND);
    TEST_STR_EQUAL(suggestion, "Verify file path and existence", "File not found suggestion should match");

    suggestion = cq_error_get_recovery_suggestion(CQ_ERROR_OUT_OF_MEMORY);
    TEST_STR_EQUAL(suggestion, "Reduce project size or increase system memory", "Out of memory suggestion should match");

    // Test recoverability
    bool recoverable = cq_error_is_recoverable(CQ_ERROR_TIMEOUT);
    TEST_ASSERT(recoverable, "Timeout error should be recoverable");

    recoverable = cq_error_is_recoverable(CQ_ERROR_OUT_OF_MEMORY);
    TEST_ASSERT(!recoverable, "Out of memory error should not be recoverable");
}

void test_error_macros()
{
    printf("\n--- Testing Error Macros ---\n");

    // Test convenience macros
    CQErrorContext *error1 = CQ_ERROR_CREATE(CQ_ERROR_INVALID_ARGUMENT, ERROR_SEVERITY_ERROR, "Test");
    TEST_NOT_NULL(error1, "CQ_ERROR_CREATE macro should work");

    CQErrorContext *error2 = CQ_ERROR_CREATEF(CQ_ERROR_MEMORY_ALLOCATION, ERROR_SEVERITY_CRITICAL,
                                            "Failed to allocate %d bytes", 1024);
    TEST_NOT_NULL(error2, "CQ_ERROR_CREATEF macro should work");
    TEST_STR_EQUAL(error2->message, "Failed to allocate 1024 bytes", "Macro formatting should work");

    cq_error_free(error1);
    cq_error_free(error2);
}

int main()
{
    printf("CQAnalyzer Error System Unit Tests\n");
    printf("==================================\n");

    // Initialize logger for testing (errors will be logged)
    logger_init();
    logger_set_level(LOG_LEVEL_ERROR); // Only show errors during testing

    // Run all tests
    test_error_initialization();
    test_error_code_conversion();
    test_error_categories();
    test_error_severity();
    test_error_creation();
    test_error_context();
    test_error_reporting();
    test_error_formatting();
    test_recovery_suggestions();
    test_error_macros();

    // Clean up
    cq_error_shutdown();
    logger_shutdown();

    // Print results
    printf("\n==================================\n");
    printf("Test Results: %d passed, %d failed\n", tests_passed, tests_failed);

    if (tests_failed == 0)
    {
        printf("✓ All tests passed!\n");
        return EXIT_SUCCESS;
    }
    else
    {
        printf("✗ Some tests failed!\n");
        return EXIT_FAILURE;
    }
}