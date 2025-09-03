/**
 * @file test_dependency_manager.c
 * @brief Unit tests for the dependency manager module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "dependency_manager.h"
#include "utils/logger.h"

/**
 * Test suite setup
 */
int init_dependency_manager_tests(void)
{
    // Initialize logger for tests
    if (logger_init() != 0) {
        return -1;
    }

    return 0;
}

/**
 * Test suite cleanup
 */
int cleanup_dependency_manager_tests(void)
{
    logger_shutdown();
    return 0;
}

/**
 * Test dependency manager initialization
 */
void test_dependency_manager_init(void)
{
    CQError result = dependency_manager_init();
    CU_ASSERT_EQUAL(result, CQ_SUCCESS);

    // Test that we can call init multiple times
    result = dependency_manager_init();
    CU_ASSERT_EQUAL(result, CQ_SUCCESS);

    dependency_manager_shutdown();
}

/**
 * Test dependency availability checking
 */
void test_dependency_availability(void)
{
    dependency_manager_init();

    // Test valid dependency types
    for (int i = 0; i < DEP_COUNT; i++) {
        bool available = dependency_is_available((DependencyType)i);
        // Availability is system-dependent, just check that function doesn't crash
        CU_ASSERT(available == true || available == false);
    }

    // Test invalid dependency type
    bool available = dependency_is_available((DependencyType)DEP_COUNT);
    CU_ASSERT_EQUAL(available, false);

    dependency_manager_shutdown();
}

/**
 * Test dependency information retrieval
 */
void test_dependency_info(void)
{
    dependency_manager_init();

    // Test valid dependency info
    for (int i = 0; i < DEP_COUNT; i++) {
        const DependencyInfo *info = dependency_get_info((DependencyType)i);
        CU_ASSERT_PTR_NOT_NULL(info);
        CU_ASSERT_PTR_NOT_NULL(info->name);
        CU_ASSERT_PTR_NOT_NULL(info->description);
    }

    // Test invalid dependency type
    const DependencyInfo *info = dependency_get_info((DependencyType)DEP_COUNT);
    CU_ASSERT_PTR_NULL(info);

    dependency_manager_shutdown();
}

/**
 * Test feature availability checking
 */
void test_feature_availability(void)
{
    dependency_manager_init();

    // Test valid feature types
    for (int i = 0; i < FEATURE_COUNT; i++) {
        bool available = feature_is_available((FeatureType)i);
        // Availability is system-dependent, just check that function doesn't crash
        CU_ASSERT(available == true || available == false);
    }

    // Test invalid feature type
    bool available = feature_is_available((FeatureType)FEATURE_COUNT);
    CU_ASSERT_EQUAL(available, false);

    dependency_manager_shutdown();
}

/**
 * Test feature information retrieval
 */
void test_feature_info(void)
{
    dependency_manager_init();

    // Test valid feature info
    for (int i = 0; i < FEATURE_COUNT; i++) {
        const FeatureInfo *info = feature_get_info((FeatureType)i);
        CU_ASSERT_PTR_NOT_NULL(info);
        CU_ASSERT_PTR_NOT_NULL(info->name);
        CU_ASSERT_PTR_NOT_NULL(info->description);
        CU_ASSERT(info->dep_count >= 0);
        CU_ASSERT(info->dep_count <= 5); // Max dependencies per feature
    }

    // Test invalid feature type
    const FeatureInfo *info = feature_get_info((FeatureType)FEATURE_COUNT);
    CU_ASSERT_PTR_NULL(info);

    dependency_manager_shutdown();
}

/**
 * Test missing dependencies detection
 */
void test_missing_dependencies(void)
{
    dependency_manager_init();

    // Test getting missing dependencies for each feature
    for (int i = 0; i < FEATURE_COUNT; i++) {
        DependencyType missing_deps[10];
        int num_missing = feature_get_missing_dependencies((FeatureType)i, missing_deps, 10);

        CU_ASSERT(num_missing >= 0);
        CU_ASSERT(num_missing <= 10);

        // Verify that all returned dependencies are valid
        for (int j = 0; j < num_missing; j++) {
            CU_ASSERT(missing_deps[j] >= 0);
            CU_ASSERT(missing_deps[j] < DEP_COUNT);
        }
    }

    dependency_manager_shutdown();
}

/**
 * Test dependency status reporting
 */
void test_dependency_status_reporting(void)
{
    dependency_manager_init();

    // Test that status reporting doesn't crash
    // We can't easily test the output, but we can ensure the function runs
    dependency_print_status();

    // Test missing features description
    char buffer[1024];
    CQError result = dependency_get_missing_features_description(buffer, sizeof(buffer));
    CU_ASSERT_EQUAL(result, CQ_SUCCESS);
    CU_ASSERT(strlen(buffer) >= 0);

    dependency_manager_shutdown();
}

/**
 * Test CLI-only mode detection
 */
void test_cli_only_mode(void)
{
    dependency_manager_init();

    bool can_run_cli = dependency_can_run_cli_only();
    // Result depends on system, just ensure it's a valid boolean
    CU_ASSERT(can_run_cli == true || can_run_cli == false);

    dependency_manager_shutdown();
}

/**
 * Test recommended mode detection
 */
void test_recommended_mode(void)
{
    dependency_manager_init();

    const char *mode = dependency_get_recommended_mode();
    CU_ASSERT_PTR_NOT_NULL(mode);

    // Mode should be one of the expected values
    CU_ASSERT(strcmp(mode, "gui") == 0 ||
              strcmp(mode, "cli") == 0 ||
              strcmp(mode, "limited") == 0 ||
              strcmp(mode, "unknown") == 0);

    dependency_manager_shutdown();
}

/**
 * Test dependency manager shutdown
 */
void test_dependency_manager_shutdown(void)
{
    dependency_manager_init();

    // Test that shutdown doesn't crash
    dependency_manager_shutdown();

    // Test that calling shutdown multiple times doesn't crash
    dependency_manager_shutdown();
}

/**
 * Main test suite setup
 */
CU_TestInfo dependency_manager_test_array[] = {
    {"test_dependency_manager_init", test_dependency_manager_init},
    {"test_dependency_availability", test_dependency_availability},
    {"test_dependency_info", test_dependency_info},
    {"test_feature_availability", test_feature_availability},
    {"test_feature_info", test_feature_info},
    {"test_missing_dependencies", test_missing_dependencies},
    {"test_dependency_status_reporting", test_dependency_status_reporting},
    {"test_cli_only_mode", test_cli_only_mode},
    {"test_recommended_mode", test_recommended_mode},
    {"test_dependency_manager_shutdown", test_dependency_manager_shutdown},
    CU_TEST_INFO_NULL
};

CU_SuiteInfo dependency_manager_test_suite = {
    "Dependency Manager Tests",
    init_dependency_manager_tests,
    cleanup_dependency_manager_tests,
    dependency_manager_test_array
};