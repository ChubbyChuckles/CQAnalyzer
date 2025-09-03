#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>

#include "cqanalyzer.h"

/**
 * @brief Test suite initialization
 */
int init_test_suite(void)
{
    return 0; // Success
}

/**
 * @brief Test suite cleanup
 */
int cleanup_test_suite(void)
{
    return 0; // Success
}

/**
 * @brief Main test function
 */
int main(int argc, char *argv[])
{
    // Initialize CUnit test registry
    if (CU_initialize_registry() != CUE_SUCCESS)
    {
        return CU_get_error();
    }

    // Create test suites
    CU_pSuite utils_suite = CU_add_suite("Utils Tests", init_test_suite, cleanup_test_suite);
    CU_pSuite parser_suite = CU_add_suite("Parser Tests", init_test_suite, cleanup_test_suite);
    CU_pSuite analyzer_suite = CU_add_suite("Analyzer Tests", init_test_suite, cleanup_test_suite);
    CU_pSuite data_suite = CU_add_suite("Data Tests", init_test_suite, cleanup_test_suite);

    if (!utils_suite || !parser_suite || !analyzer_suite || !data_suite)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Add tests to suites
    // Note: Test functions are defined in separate files
    extern void add_utils_tests(CU_pSuite);
    extern void add_parser_tests(CU_pSuite);
    extern void add_analyzer_tests(CU_pSuite);
    extern void add_ui_tests(CU_pSuite);

    add_utils_tests(utils_suite);
    add_parser_tests(parser_suite);
    add_analyzer_tests(analyzer_suite);
    add_ui_tests(data_suite); // Add UI tests to data suite for now

    // Run tests
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    // Get test results
    CU_basic_show_failures(CU_get_failure_list());

    // Cleanup
    CU_cleanup_registry();
    return CU_get_error();
}
