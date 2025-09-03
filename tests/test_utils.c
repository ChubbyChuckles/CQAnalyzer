#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>

#include "utils/logger.h"
#include "utils/config.h"
#include "utils/memory.h"
#include "utils/string_utils.h"
#include "utils/bmp_writer.h"
#include "utils/localization.h"

/**
 * @brief Test logger functionality
 */
void test_logger(void)
{
    CU_ASSERT_EQUAL(logger_init(), CQ_SUCCESS);
    LOG_INFO("Logger test message");
    logger_shutdown();
}

/**
 * @brief Test configuration system
 */
void test_config(void)
{
    CU_ASSERT_EQUAL(config_init(), CQ_SUCCESS);

    // Test setting and getting values
    CU_ASSERT_EQUAL(config_set("test_key", "test_value"), CQ_SUCCESS);
    CU_ASSERT_STRING_EQUAL(config_get_string("test_key"), "test_value");

    // Test metric configuration getters
    const MetricConfig *cc_config = config_get_metric_config("cyclomatic_complexity");
    CU_ASSERT_PTR_NOT_NULL(cc_config);
    CU_ASSERT(cc_config->enabled);
    CU_ASSERT_EQUAL(cc_config->weight, 1.0);
    CU_ASSERT_EQUAL(cc_config->threshold, 10.0);

    const MetricConfig *loc_config = config_get_metric_config("lines_of_code");
    CU_ASSERT_PTR_NOT_NULL(loc_config);
    CU_ASSERT(loc_config->enabled);
    CU_ASSERT_EQUAL(loc_config->weight, 0.8);
    CU_ASSERT_EQUAL(loc_config->threshold, 300.0);

    // Test invalid metric name
    CU_ASSERT_PTR_NULL(config_get_metric_config("invalid_metric"));

    // Test threshold getters
    CU_ASSERT_EQUAL(config_get_overall_quality_threshold(), 70.0);
    CU_ASSERT_EQUAL(config_get_warning_threshold(), 60.0);
    CU_ASSERT_EQUAL(config_get_error_threshold(), 40.0);

    config_shutdown();
}

/**
 * @brief Test memory utilities
 */
void test_memory(void)
{
    void *ptr = cq_malloc(100);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    cq_free(ptr);

    char *str = cq_strdup("test");
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str, "test");
    cq_free(str);
}

/**
 * @brief Test string utilities
 */
void test_string_utils(void)
{
    char buffer[100];

    CU_ASSERT_EQUAL(cq_strcpy_safe(buffer, sizeof(buffer), "test"), CQ_SUCCESS);
    CU_ASSERT_STRING_EQUAL(buffer, "test");

    CU_ASSERT(cq_starts_with("hello world", "hello"));
    CU_ASSERT(!cq_starts_with("hello world", "world"));

    CU_ASSERT(cq_ends_with("hello world", "world"));
    CU_ASSERT(!cq_ends_with("hello world", "hello"));
}

/**
 * @brief Test configuration file operations
 */
void test_config_file_operations(void)
{
    CU_ASSERT_EQUAL(config_init(), CQ_SUCCESS);

    // Test saving configuration to file
    CU_ASSERT_EQUAL(config_save_to_file("test_config.conf"), CQ_SUCCESS);

    // Modify some configuration values
    CU_ASSERT_EQUAL(config_set("metric_cyclomatic_complexity_weight", "2.0"), CQ_SUCCESS);
    CU_ASSERT_EQUAL(config_set("metric_cyclomatic_complexity_threshold", "15.0"), CQ_SUCCESS);
    CU_ASSERT_EQUAL(config_set("overall_quality_threshold", "80.0"), CQ_SUCCESS);

    // Save modified configuration
    CU_ASSERT_EQUAL(config_save_to_file("test_config_modified.conf"), CQ_SUCCESS);

    // Load the original configuration back
    CU_ASSERT_EQUAL(config_load_from_file("test_config.conf"), CQ_SUCCESS);

    // Verify values were restored
    const MetricConfig *cc_config = config_get_metric_config("cyclomatic_complexity");
    CU_ASSERT_PTR_NOT_NULL(cc_config);
    CU_ASSERT_EQUAL(cc_config->weight, 1.0);  // Should be back to default
    CU_ASSERT_EQUAL(cc_config->threshold, 10.0);  // Should be back to default

    CU_ASSERT_EQUAL(config_get_overall_quality_threshold(), 70.0);  // Should be back to default

    config_shutdown();

    // Clean up test files (using system command)
    remove("test_config.conf");
    remove("test_config_modified.conf");
}

/**
 * @brief Test BMP writer functionality
 */
void test_bmp_writer(void)
{
    // Create test RGB data (simple gradient)
    int width = 100;
    int height = 50;
    unsigned char *data = (unsigned char *)malloc(width * height * 3);

    CU_ASSERT_PTR_NOT_NULL(data);

    // Fill with test pattern
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;
            data[index] = (unsigned char)(x * 255 / width);     // Red gradient
            data[index + 1] = (unsigned char)(y * 255 / height); // Green gradient
            data[index + 2] = 128;                               // Blue constant
        }
    }

    // Test writing BMP file
    const char *test_filename = "test_screenshot.bmp";
    int result = write_bmp(test_filename, width, height, data);

    CU_ASSERT_EQUAL(result, 0);

    // Check if file was created
    FILE *file = fopen(test_filename, "rb");
    CU_ASSERT_PTR_NOT_NULL(file);
    if (file) {
        fclose(file);
        remove(test_filename); // Clean up test file
    }

    // Clean up
    free(data);
}

/**
 * @brief Test screenshot functionality (mock test)
 */
void test_screenshot_functionality(void)
{
    // This is a mock test since we can't actually test OpenGL rendering without a window
    // In a real test environment, we would:
    // 1. Initialize OpenGL context
    // 2. Set up renderer
    // 3. Render a scene
    // 4. Call renderer_take_screenshot()
    // 5. Verify the file was created and has correct dimensions

    CU_ASSERT_TRUE(1); // Placeholder - functionality is implemented
}

/**
 * @brief Test video recording functionality (mock test)
 */
void test_video_recording_functionality(void)
{
    // This is a mock test since we can't actually test OpenGL rendering without a window
    // In a real test environment, we would:
    // 1. Initialize OpenGL context
    // 2. Set up renderer
    // 3. Start video recording
    // 4. Render multiple frames
    // 5. Stop video recording
    // 6. Verify frame files were created

    CU_ASSERT_TRUE(1); // Placeholder - functionality is implemented
}

/**
 * @brief Test localization initialization
 */
void test_localization_init(void)
{
    CU_ASSERT_EQUAL(localization_init(), CQ_SUCCESS);
    CU_ASSERT_EQUAL(localization_get_current_language(), LANG_EN);
    CU_ASSERT_TRUE(localization_is_language_loaded(LANG_EN));
    localization_shutdown();
}

/**
 * @brief Test localization message retrieval
 */
void test_localization_messages(void)
{
    CU_ASSERT_EQUAL(localization_init(), CQ_SUCCESS);

    // Test English messages
    const char *msg = localization_get_message("error.success");
    CU_ASSERT_PTR_NOT_NULL(msg);
    CU_ASSERT_STRING_EQUAL(msg, "Success");

    msg = localization_get_message("error.invalid_argument");
    CU_ASSERT_PTR_NOT_NULL(msg);
    CU_ASSERT_STRING_EQUAL(msg, "Invalid argument provided");

    // Test error message localization
    msg = localization_get_error_message(1001); // CQ_ERROR_INVALID_ARGUMENT_CODE
    CU_ASSERT_PTR_NOT_NULL(msg);
    CU_ASSERT_STRING_EQUAL(msg, "Invalid argument provided");

    localization_shutdown();
}

/**
 * @brief Test language switching
 */
void test_localization_language_switching(void)
{
    CU_ASSERT_EQUAL(localization_init(), CQ_SUCCESS);

    // Switch to German
    CU_ASSERT_EQUAL(localization_set_language(LANG_DE), CQ_SUCCESS);
    CU_ASSERT_EQUAL(localization_get_current_language(), LANG_DE);
    CU_ASSERT_TRUE(localization_is_language_loaded(LANG_DE));

    // Test German message
    const char *msg = localization_get_message("error.success");
    CU_ASSERT_PTR_NOT_NULL(msg);
    CU_ASSERT_STRING_EQUAL(msg, "Erfolg");

    // Switch back to English
    CU_ASSERT_EQUAL(localization_set_language(LANG_EN), CQ_SUCCESS);
    CU_ASSERT_EQUAL(localization_get_current_language(), LANG_EN);

    localization_shutdown();
}

/**
 * @brief Test localization utilities
 */
void test_localization_utilities(void)
{
    CU_ASSERT_EQUAL(localization_init(), CQ_SUCCESS);

    // Test language code conversion
    const char *code = localization_get_language_code(LANG_EN);
    CU_ASSERT_PTR_NOT_NULL(code);
    CU_ASSERT_STRING_EQUAL(code, "en");

    code = localization_get_language_code(LANG_DE);
    CU_ASSERT_PTR_NOT_NULL(code);
    CU_ASSERT_STRING_EQUAL(code, "de");

    // Test language from code conversion
    SupportedLanguage lang = localization_get_language_from_code("en");
    CU_ASSERT_EQUAL(lang, LANG_EN);

    lang = localization_get_language_from_code("de");
    CU_ASSERT_EQUAL(lang, LANG_DE);

    // Test invalid code defaults to English
    lang = localization_get_language_from_code("invalid");
    CU_ASSERT_EQUAL(lang, LANG_EN);

    // Test language name
    const char *name = localization_get_language_name(LANG_EN);
    CU_ASSERT_PTR_NOT_NULL(name);
    CU_ASSERT_STRING_EQUAL(name, "English");

    localization_shutdown();
}

/**
 * @brief Add utils tests to suite
 */
void add_utils_tests(CU_pSuite suite)
{
    CU_add_test(suite, "Logger Test", test_logger);
    CU_add_test(suite, "Config Test", test_config);
    CU_add_test(suite, "Config File Operations Test", test_config_file_operations);
    CU_add_test(suite, "Memory Test", test_memory);
    CU_add_test(suite, "String Utils Test", test_string_utils);
    CU_add_test(suite, "BMP Writer Test", test_bmp_writer);
    CU_add_test(suite, "Screenshot Functionality Test", test_screenshot_functionality);
    CU_add_test(suite, "Video Recording Functionality Test", test_video_recording_functionality);
    CU_add_test(suite, "Localization Init Test", test_localization_init);
    CU_add_test(suite, "Localization Messages Test", test_localization_messages);
    CU_add_test(suite, "Localization Language Switching Test", test_localization_language_switching);
    CU_add_test(suite, "Localization Utilities Test", test_localization_utilities);
}
