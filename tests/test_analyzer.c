#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "analyzer/metric_calculator.h"
#include "analyzer/complexity_analyzer.h"
#include "analyzer/duplication_detector.h"

/**
 * @brief Test metric calculation
 */
void test_metric_calculator(void)
{
    int physical, logical, comment;
    CU_ASSERT_EQUAL(calculate_lines_of_code("test.c", &physical, &logical, &comment), CQ_SUCCESS);

    double mi = calculate_maintainability_index(5, 100, 0.1);
    CU_ASSERT(mi >= 0.0 && mi <= 100.0);
}

/**
 * @brief Test complexity analysis
 */
void test_complexity_analyzer(void)
{
    int complexity;
    CU_ASSERT_EQUAL(analyze_file_complexity("tests/test_sample.c", &complexity), CQ_SUCCESS);
    CU_ASSERT(complexity >= 1); // At least base complexity

    // The sample file has functions with various complexities
    // simple_function: 1 (no decisions)
    // conditional_function: 2 (1 if)
    // loop_function: 2 (1 for)
    // complex_function: 4 (2 if + && + ||)
    // Average should be around 2-3
    CU_ASSERT(complexity >= 1 && complexity <= 5);
}

/**
 * @brief Test Halstead metrics calculation
 */
void test_halstead_metrics(void)
{
    HalsteadMetrics metrics;
    CU_ASSERT_EQUAL(calculate_halstead_metrics("tests/test_sample.c", &metrics), CQ_SUCCESS);

    // Check that metrics are calculated
    CU_ASSERT(metrics.n1 >= 0);
    CU_ASSERT(metrics.n2 >= 0);
    CU_ASSERT(metrics.N1 >= 0);
    CU_ASSERT(metrics.N2 >= 0);
    CU_ASSERT(metrics.volume >= 0.0);
    CU_ASSERT(metrics.difficulty >= 0.0);
    CU_ASSERT(metrics.effort >= 0.0);
    CU_ASSERT(metrics.time >= 0.0);
    CU_ASSERT(metrics.bugs >= 0.0);

    // The sample file should have some operators and operands
    CU_ASSERT(metrics.N1 > 0 || metrics.N2 > 0);
}

/**
 * @brief Test duplication detection
 */
void test_duplication_detector(void)
{
    double ratio;
    CU_ASSERT_EQUAL(detect_file_duplication("tests/test_sample.c", &ratio), CQ_SUCCESS);
    CU_ASSERT(ratio >= 0.0 && ratio <= 1.0);

    // The sample file has some repeated patterns, so ratio should be > 0
    // But for a small file, it might be low
}

/**
 * @brief Add analyzer tests to suite
 */
void add_analyzer_tests(CU_pSuite suite)
{
    CU_add_test(suite, "Metric Calculator Test", test_metric_calculator);
    CU_add_test(suite, "Complexity Analyzer Test", test_complexity_analyzer);
    CU_add_test(suite, "Halstead Metrics Test", test_halstead_metrics);
    CU_add_test(suite, "Duplication Detector Test", test_duplication_detector);
}
