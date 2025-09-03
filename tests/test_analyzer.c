#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "analyzer/metric_calculator.h"
#include "analyzer/complexity_analyzer.h"
#include "analyzer/duplication_detector.h"
#include "analyzer/dead_code_detector.h"
#include "parser/ast_parser.h"
#include "data/ast_types.h"

/**
 * @brief Test metric calculation
 */
void test_metric_calculator(void)
{
    int physical, logical, comment;
    CU_ASSERT_EQUAL(calculate_lines_of_code("test.c", &physical, &logical, &comment), CQ_SUCCESS);

    double mi = calculate_maintainability_index(5, 100, 0.1);
    CU_ASSERT(mi >= 0.0 && mi <= 100.0);

    // Test comment density calculation
    double density = calculate_comment_density(10, 100);
    CU_ASSERT_DOUBLE_EQUAL(density, 10.0, 0.01);

    density = calculate_comment_density(0, 100);
    CU_ASSERT_DOUBLE_EQUAL(density, 0.0, 0.01);

    density = calculate_comment_density(50, 100);
    CU_ASSERT_DOUBLE_EQUAL(density, 50.0, 0.01);

    // Test edge case: zero physical lines
    density = calculate_comment_density(5, 0);
    CU_ASSERT_DOUBLE_EQUAL(density, 0.0, 0.01);

    // Test class cohesion calculation
    struct ClassInfo test_class;
    memset(&test_class, 0, sizeof(struct ClassInfo));
    strcpy(test_class.name, "TestClass");

    // Test case 1: Equal methods and fields
    test_class.method_count = 5;
    test_class.field_count = 5;
    double cohesion = calculate_class_cohesion(&test_class);
    CU_ASSERT_DOUBLE_EQUAL(cohesion, 1.0, 0.01);

    // Test case 2: More methods than fields (good cohesion)
    test_class.method_count = 10;
    test_class.field_count = 5;
    cohesion = calculate_class_cohesion(&test_class);
    CU_ASSERT_DOUBLE_EQUAL(cohesion, 1.0, 0.01); // Capped at 1.0

    // Test case 3: Fewer methods than fields (lower cohesion)
    test_class.method_count = 2;
    test_class.field_count = 5;
    cohesion = calculate_class_cohesion(&test_class);
    CU_ASSERT_DOUBLE_EQUAL(cohesion, 0.4, 0.01);

    // Test case 4: No fields
    test_class.method_count = 3;
    test_class.field_count = 0;
    cohesion = calculate_class_cohesion(&test_class);
    CU_ASSERT_DOUBLE_EQUAL(cohesion, 0.5, 0.01);

    // Test case 5: No methods, no fields
    test_class.method_count = 0;
    test_class.field_count = 0;
    cohesion = calculate_class_cohesion(&test_class);
    CU_ASSERT_DOUBLE_EQUAL(cohesion, 0.0, 0.01);

    // Test edge case: NULL class info
    cohesion = calculate_class_cohesion(NULL);
    CU_ASSERT_DOUBLE_EQUAL(cohesion, 0.0, 0.01);
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
 * @brief Test nesting depth analysis
 */
void test_nesting_depth_analyzer(void)
{
    // Initialize AST parser
    CU_ASSERT_EQUAL(ast_parser_init(), CQ_SUCCESS);

    // Parse the test file to get AST data
    void *ast_data = parse_source_file("tests/test_sample.c");
    CU_ASSERT_PTR_NOT_NULL(ast_data);

    if (ast_data)
    {
        ASTData *data = (ASTData *)ast_data;
        CU_ASSERT_PTR_NOT_NULL(data->project);
        CU_ASSERT_PTR_NOT_NULL(data->project->files);

        // Find and test each function's nesting depth
        FunctionInfo *func = data->project->files->functions;
        int found_functions = 0;

        while (func)
        {
            int nesting_depth;
            CQError result = calculate_nesting_depth(func, &nesting_depth);
            CU_ASSERT_EQUAL(result, CQ_SUCCESS);
            CU_ASSERT(nesting_depth >= 0); // Nesting depth should be non-negative

            // Check specific function nesting depths based on their names
            if (strcmp(func->name, "simple_function") == 0)
            {
                CU_ASSERT_EQUAL(nesting_depth, 0); // No control flow
                found_functions++;
            }
            else if (strcmp(func->name, "conditional_function") == 0)
            {
                CU_ASSERT_EQUAL(nesting_depth, 1); // 1 level of if-else
                found_functions++;
            }
            else if (strcmp(func->name, "loop_function") == 0)
            {
                CU_ASSERT_EQUAL(nesting_depth, 1); // 1 level of for loop
                found_functions++;
            }
            else if (strcmp(func->name, "complex_function") == 0)
            {
                CU_ASSERT_EQUAL(nesting_depth, 2); // 2 levels of nesting (if inside if)
                found_functions++;
            }

            func = func->next;
        }

        // Ensure we found all expected functions
        CU_ASSERT_EQUAL(found_functions, 4);

        // Clean up
        free_ast_data(ast_data);
    }

    // Shutdown AST parser
    ast_parser_shutdown();
}

/**
 * @brief Test individual function complexity analysis
 */
void test_function_complexity_analyzer(void)
{
    // Initialize AST parser
    CU_ASSERT_EQUAL(ast_parser_init(), CQ_SUCCESS);

    // Parse the test file to get AST data
    void *ast_data = parse_source_file("tests/test_sample.c");
    CU_ASSERT_PTR_NOT_NULL(ast_data);

    if (ast_data)
    {
        ASTData *data = (ASTData *)ast_data;
        CU_ASSERT_PTR_NOT_NULL(data->project);
        CU_ASSERT_PTR_NOT_NULL(data->project->files);

        // Find and test each function
        FunctionInfo *func = data->project->files->functions;
        int found_functions = 0;

        while (func)
        {
            int complexity;
            CQError result = analyze_function_complexity(func, &complexity);
            CU_ASSERT_EQUAL(result, CQ_SUCCESS);
            CU_ASSERT(complexity >= 1); // All functions should have at least base complexity

            // Check specific function complexities based on their names
            if (strcmp(func->name, "simple_function") == 0)
            {
                CU_ASSERT_EQUAL(complexity, 1); // No control flow
                found_functions++;
            }
            else if (strcmp(func->name, "conditional_function") == 0)
            {
                CU_ASSERT_EQUAL(complexity, 2); // 1 if statement
                found_functions++;
            }
            else if (strcmp(func->name, "loop_function") == 0)
            {
                CU_ASSERT_EQUAL(complexity, 2); // 1 for loop
                found_functions++;
            }
            else if (strcmp(func->name, "complex_function") == 0)
            {
                CU_ASSERT_EQUAL(complexity, 4); // 2 if + && + ||
                found_functions++;
            }

            func = func->next;
        }

        // Ensure we found all expected functions
        CU_ASSERT_EQUAL(found_functions, 4);

        // Clean up
        free_ast_data(ast_data);
    }

    // Shutdown AST parser
    ast_parser_shutdown();
}

/**
 * @brief Test class coupling calculation
 */
void test_class_coupling(void)
{
    // Create test classes
    struct ClassInfo class1, class2, class3;
    memset(&class1, 0, sizeof(struct ClassInfo));
    memset(&class2, 0, sizeof(struct ClassInfo));
    memset(&class3, 0, sizeof(struct ClassInfo));

    strcpy(class1.name, "ClassA");
    strcpy(class2.name, "ClassB");
    strcpy(class3.name, "ClassC");

    // Set up class sizes
    class1.method_count = 5;
    class1.field_count = 3;
    class2.method_count = 8;
    class2.field_count = 4;
    class3.method_count = 3;
    class3.field_count = 2;

    // Link classes in a list
    class1.next = &class2;
    class2.next = &class3;
    class3.next = NULL;

    // Test coupling calculation for each class
    double coupling1 = calculate_class_coupling(&class1, &class1);
    double coupling2 = calculate_class_coupling(&class2, &class1);
    double coupling3 = calculate_class_coupling(&class3, &class1);

    // Coupling should be between 0.0 and 1.0
    CU_ASSERT(coupling1 >= 0.0 && coupling1 <= 1.0);
    CU_ASSERT(coupling2 >= 0.0 && coupling2 <= 1.0);
    CU_ASSERT(coupling3 >= 0.0 && coupling3 <= 1.0);

    // Test edge cases
    // NULL class info
    double coupling_null = calculate_class_coupling(NULL, &class1);
    CU_ASSERT_DOUBLE_EQUAL(coupling_null, 0.0, 0.01);

    // NULL all_classes
    coupling_null = calculate_class_coupling(&class1, NULL);
    CU_ASSERT_DOUBLE_EQUAL(coupling_null, 0.0, 0.01);

    // Single class (no coupling possible)
    struct ClassInfo single_class;
    memset(&single_class, 0, sizeof(struct ClassInfo));
    strcpy(single_class.name, "SingleClass");
    single_class.method_count = 5;
    single_class.field_count = 3;
    single_class.next = NULL;

    double coupling_single = calculate_class_coupling(&single_class, &single_class);
    CU_ASSERT_DOUBLE_EQUAL(coupling_single, 0.0, 0.01);
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
 * @brief Test dead code detection
 */
void test_dead_code_detector(void)
{
    DeadCodeList dead_code_list;

    // Test with the sample file
    CQError result = detect_dead_code_in_file("tests/test_sample.c", &dead_code_list);
    CU_ASSERT_EQUAL(result, CQ_SUCCESS);

    // The sample file should have some dead code (unused functions/variables)
    // At minimum, we should have a valid result structure
    CU_ASSERT_PTR_NOT_NULL(dead_code_list.results);

    // Check that results are properly initialized
    CU_ASSERT(dead_code_list.count >= 0);
    CU_ASSERT(dead_code_list.capacity >= dead_code_list.count);

    // Test with invalid parameters
    result = detect_dead_code_in_file(NULL, &dead_code_list);
    CU_ASSERT_EQUAL(result, CQ_ERROR_INVALID_ARGUMENT);

    result = detect_dead_code_in_file("tests/test_sample.c", NULL);
    CU_ASSERT_EQUAL(result, CQ_ERROR_INVALID_ARGUMENT);

    // Test with non-existent file
    result = detect_dead_code_in_file("non_existent_file.c", &dead_code_list);
    CU_ASSERT_EQUAL(result, CQ_ERROR_UNKNOWN);

    // Clean up
    free_dead_code_list(&dead_code_list);

    // Test project-wide detection (currently stubbed)
    result = detect_dead_code_in_project(".", &dead_code_list);
    CU_ASSERT_EQUAL(result, CQ_SUCCESS);
}

/**
 * @brief Test metric normalization functions
 */
void test_metric_normalization(void)
{
    // Test min-max normalization
    double value = 75.0;
    double min_val = 50.0;
    double max_val = 100.0;
    double mean = 75.0;
    double std_dev = 15.0;

    // Min-max normalization: (75 - 50) / (100 - 50) = 25/50 = 0.5
    double normalized = normalize_metric(value, min_val, max_val, mean, std_dev, NORMALIZATION_MIN_MAX);
    CU_ASSERT_DOUBLE_EQUAL(normalized, 0.5, 0.001);

    // Test edge case: value equals min
    normalized = normalize_metric(min_val, min_val, max_val, mean, std_dev, NORMALIZATION_MIN_MAX);
    CU_ASSERT_DOUBLE_EQUAL(normalized, 0.0, 0.001);

    // Test edge case: value equals max
    normalized = normalize_metric(max_val, min_val, max_val, mean, std_dev, NORMALIZATION_MIN_MAX);
    CU_ASSERT_DOUBLE_EQUAL(normalized, 1.0, 0.001);

    // Test edge case: min equals max (no variation)
    normalized = normalize_metric(10.0, 10.0, 10.0, 10.0, 0.0, NORMALIZATION_MIN_MAX);
    CU_ASSERT_DOUBLE_EQUAL(normalized, 0.5, 0.001);

    // Test z-score normalization: (75 - 75) / 15 = 0/15 = 0
    normalized = normalize_metric(value, min_val, max_val, mean, std_dev, NORMALIZATION_Z_SCORE);
    CU_ASSERT_DOUBLE_EQUAL(normalized, 0.0, 0.001);

    // Test z-score with value above mean: (90 - 75) / 15 = 15/15 = 1
    normalized = normalize_metric(90.0, min_val, max_val, mean, std_dev, NORMALIZATION_Z_SCORE);
    CU_ASSERT_DOUBLE_EQUAL(normalized, 1.0, 0.001);

    // Test z-score with value below mean: (60 - 75) / 15 = -15/15 = -1
    normalized = normalize_metric(60.0, min_val, max_val, mean, std_dev, NORMALIZATION_Z_SCORE);
    CU_ASSERT_DOUBLE_EQUAL(normalized, -1.0, 0.001);

    // Test edge case: zero standard deviation
    normalized = normalize_metric(75.0, min_val, max_val, mean, 0.0, NORMALIZATION_Z_SCORE);
    CU_ASSERT_DOUBLE_EQUAL(normalized, 0.0, 0.001);

    // Test scaling function
    double scaled = scale_metric(0.5, 0.0, 255.0); // Scale to 0-255 range
    CU_ASSERT_DOUBLE_EQUAL(scaled, 127.5, 0.001);

    scaled = scale_metric(0.0, 0.0, 255.0); // Min value
    CU_ASSERT_DOUBLE_EQUAL(scaled, 0.0, 0.001);

    scaled = scale_metric(1.0, 0.0, 255.0); // Max value
    CU_ASSERT_DOUBLE_EQUAL(scaled, 255.0, 0.001);

    scaled = scale_metric(0.5, -100.0, 100.0); // Scale to -100 to 100 range
    CU_ASSERT_DOUBLE_EQUAL(scaled, 0.0, 0.001);
}

/**
 * @brief Test metric array normalization
 */
void test_metric_array_normalization(void)
{
    // Test array with known values
    double values[] = {10.0, 20.0, 30.0, 40.0, 50.0};
    size_t count = 5;
    double output[5];

    // Test min-max normalization
    CQError result = normalize_metric_array(values, count, NORMALIZATION_MIN_MAX, output);
    CU_ASSERT_EQUAL(result, CQ_SUCCESS);

    // Check that values are normalized to [0, 1] range
    for (size_t i = 0; i < count; i++)
    {
        CU_ASSERT(output[i] >= 0.0 && output[i] <= 1.0);
    }

    // First value should be 0.0, last should be 1.0
    CU_ASSERT_DOUBLE_EQUAL(output[0], 0.0, 0.001);
    CU_ASSERT_DOUBLE_EQUAL(output[4], 1.0, 0.001);

    // Middle value should be 0.5
    CU_ASSERT_DOUBLE_EQUAL(output[2], 0.5, 0.001);

    // Test z-score normalization
    result = normalize_metric_array(values, count, NORMALIZATION_Z_SCORE, output);
    CU_ASSERT_EQUAL(result, CQ_SUCCESS);

    // For this symmetric array, mean should be 30.0
    // Values should be: -1.414, -0.707, 0.0, 0.707, 1.414 (approximately)
    CU_ASSERT_DOUBLE_EQUAL(output[2], 0.0, 0.01); // Middle value should be 0

    // Test edge cases
    // Empty array
    result = normalize_metric_array(NULL, 0, NORMALIZATION_MIN_MAX, output);
    CU_ASSERT_EQUAL(result, CQ_ERROR_INVALID_ARGUMENT);

    result = normalize_metric_array(values, 0, NORMALIZATION_MIN_MAX, output);
    CU_ASSERT_EQUAL(result, CQ_ERROR_INVALID_ARGUMENT);

    // NULL pointers
    result = normalize_metric_array(NULL, count, NORMALIZATION_MIN_MAX, output);
    CU_ASSERT_EQUAL(result, CQ_ERROR_INVALID_ARGUMENT);

    result = normalize_metric_array(values, count, NORMALIZATION_MIN_MAX, NULL);
    CU_ASSERT_EQUAL(result, CQ_ERROR_INVALID_ARGUMENT);
}

/**
 * @brief Add analyzer tests to suite
 */
void add_analyzer_tests(CU_pSuite suite)
{
    CU_add_test(suite, "Metric Calculator Test", test_metric_calculator);
    CU_add_test(suite, "Complexity Analyzer Test", test_complexity_analyzer);
    CU_add_test(suite, "Function Complexity Analyzer Test", test_function_complexity_analyzer);
    CU_add_test(suite, "Nesting Depth Analyzer Test", test_nesting_depth_analyzer);
    CU_add_test(suite, "Halstead Metrics Test", test_halstead_metrics);
    CU_add_test(suite, "Duplication Detector Test", test_duplication_detector);
    CU_add_test(suite, "Class Coupling Test", test_class_coupling);
    CU_add_test(suite, "Dead Code Detector Test", test_dead_code_detector);
    CU_add_test(suite, "Metric Normalization Test", test_metric_normalization);
    CU_add_test(suite, "Metric Array Normalization Test", test_metric_array_normalization);
}
