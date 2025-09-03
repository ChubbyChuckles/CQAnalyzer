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
    CU_ASSERT_EQUAL(analyze_file_complexity("test.c", &complexity), CQ_SUCCESS);
    CU_ASSERT(complexity >= 0);
}

/**
 * @brief Test duplication detection
 */
void test_duplication_detector(void)
{
    double ratio;
    CU_ASSERT_EQUAL(detect_file_duplication("test.c", &ratio), CQ_SUCCESS);
    CU_ASSERT(ratio >= 0.0 && ratio <= 1.0);
}

/**
 * @brief Add analyzer tests to suite
 */
void add_analyzer_tests(CU_pSuite suite)
{
    CU_add_test(suite, "Metric Calculator Test", test_metric_calculator);
    CU_add_test(suite, "Complexity Analyzer Test", test_complexity_analyzer);
    CU_add_test(suite, "Duplication Detector Test", test_duplication_detector);
}
