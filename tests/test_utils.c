#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "utils/logger.h"
#include "utils/config.h"
#include "utils/memory.h"
#include "utils/string_utils.h"

/**
 * @brief Test logger functionality
 */
void test_logger(void) {
    CU_ASSERT_EQUAL(logger_init(), CQ_SUCCESS);
    LOG_INFO("Logger test message");
    logger_shutdown();
}

/**
 * @brief Test configuration system
 */
void test_config(void) {
    CU_ASSERT_EQUAL(config_init(), CQ_SUCCESS);

    // Test setting and getting values
    CU_ASSERT_EQUAL(config_set("test_key", "test_value"), CQ_SUCCESS);
    CU_ASSERT_STRING_EQUAL(config_get_string("test_key"), "test_value");

    config_shutdown();
}

/**
 * @brief Test memory utilities
 */
void test_memory(void) {
    void* ptr = cq_malloc(100);
    CU_ASSERT_PTR_NOT_NULL(ptr);
    cq_free(ptr);

    char* str = cq_strdup("test");
    CU_ASSERT_PTR_NOT_NULL(str);
    CU_ASSERT_STRING_EQUAL(str, "test");
    cq_free(str);
}

/**
 * @brief Test string utilities
 */
void test_string_utils(void) {
    char buffer[100];

    CU_ASSERT_EQUAL(cq_strcpy_safe(buffer, sizeof(buffer), "test"), CQ_SUCCESS);
    CU_ASSERT_STRING_EQUAL(buffer, "test");

    CU_ASSERT(cq_starts_with("hello world", "hello"));
    CU_ASSERT(!cq_starts_with("hello world", "world"));

    CU_ASSERT(cq_ends_with("hello world", "world"));
    CU_ASSERT(!cq_ends_with("hello world", "hello"));
}

/**
 * @brief Add utils tests to suite
 */
void add_utils_tests(CU_pSuite suite) {
    CU_add_test(suite, "Logger Test", test_logger);
    CU_add_test(suite, "Config Test", test_config);
    CU_add_test(suite, "Memory Test", test_memory);
    CU_add_test(suite, "String Utils Test", test_string_utils);
}
