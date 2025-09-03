#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>

#include "utils/logger.h"
#include "utils/config.h"
#include "utils/memory.h"
#include "utils/string_utils.h"

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
 * @brief Add utils tests to suite
 */
void add_utils_tests(CU_pSuite suite)
{
    CU_add_test(suite, "Logger Test", test_logger);
    CU_add_test(suite, "Config Test", test_config);
    CU_add_test(suite, "Config File Operations Test", test_config_file_operations);
    CU_add_test(suite, "Memory Test", test_memory);
    CU_add_test(suite, "String Utils Test", test_string_utils);
}
