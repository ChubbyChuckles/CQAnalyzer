#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "data/data_store.h"
#include "data/metric_aggregator.h"
#include "data/serialization.h"

/**
 * @brief Test data store
 */
void test_data_store(void) {
    CU_ASSERT_EQUAL(data_store_init(), CQ_SUCCESS);

    CU_ASSERT_EQUAL(data_store_add_file("test.c", LANG_C), CQ_SUCCESS);
    CU_ASSERT_EQUAL(data_store_add_metric("test.c", "complexity", 5.0), CQ_SUCCESS);

    double value = data_store_get_metric("test.c", "complexity");
    CU_ASSERT_EQUAL(value, 5.0);

    data_store_shutdown();
}

/**
 * @brief Test metric aggregation
 */
void test_metric_aggregator(void) {
    CU_ASSERT_EQUAL(aggregate_project_metrics("test_project"), CQ_SUCCESS);

    double mean, median, stddev;
    CU_ASSERT_EQUAL(calculate_metric_statistics("complexity", &mean, &median, &stddev), CQ_SUCCESS);
}

/**
 * @brief Test serialization
 */
void test_serialization(void) {
    // TODO: Implement serialization tests
    CU_PASS("Serialization test placeholder");
}

/**
 * @brief Add data tests to suite
 */
void add_data_tests(CU_pSuite suite) {
    CU_add_test(suite, "Data Store Test", test_data_store);
    CU_add_test(suite, "Metric Aggregator Test", test_metric_aggregator);
    CU_add_test(suite, "Serialization Test", test_serialization);
}
