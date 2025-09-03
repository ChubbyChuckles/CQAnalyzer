#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser/file_scanner.h"
#include "parser/ast_parser.h"
#include "parser/language_support.h"
#include "parser/preprocessor.h"
#include "parser/generic_parser.h"

/**
 * @brief Test file scanning
 */
void test_file_scanner(void)
{
    char *files[100];

    // Test scanning current directory
    int count = scan_directory(".", files, 100);
    CU_ASSERT(count >= 0);

    // Free allocated memory
    for (int i = 0; i < count; i++)
    {
        free(files[i]);
    }

    CU_PASS("File scanner test completed");
}

/**
 * @brief Test file scanning with progress
 */
void test_file_scanner_with_progress(void)
{
    char *files[100];

    // Test scanning with progress callback
    int count = scan_directory_with_progress(".", files, 100, NULL);
    CU_ASSERT(count >= 0);

    // Free allocated memory
    for (int i = 0; i < count; i++)
    {
        free(files[i]);
    }

    CU_PASS("File scanner with progress test completed");
}

/**
 * @brief Test file scanning with invalid parameters
 */
void test_file_scanner_invalid_params(void)
{
    char *files[100];

    // Test with NULL path
    int count = scan_directory(NULL, files, 100);
    CU_ASSERT_EQUAL(count, -1);

    // Test with NULL files array
    count = scan_directory(".", NULL, 100);
    CU_ASSERT_EQUAL(count, -1);

    CU_PASS("File scanner invalid params test completed");
}

/**
 * @brief Test AST parsing
 */
void test_ast_parser(void)
{
    CU_ASSERT_EQUAL(ast_parser_init(), CQ_SUCCESS);
    // TODO: Implement AST parser tests
    ast_parser_shutdown();
    CU_PASS("AST parser test placeholder");
}

/**
 * @brief Test language support
 */
void test_language_support(void)
{
    CU_ASSERT_EQUAL(detect_language("test.c"), LANG_C);
    CU_ASSERT_EQUAL(detect_language("test.cpp"), LANG_CPP);
    CU_ASSERT_EQUAL(detect_language("test.java"), LANG_JAVA);
    CU_ASSERT_EQUAL(detect_language("unknown.xyz"), LANG_UNKNOWN);
}

/**
 * @brief Test preprocessor initialization
 */
void test_preprocessor_init(void)
{
    PreprocessingContext *ctx = preprocessor_init();
    CU_ASSERT_PTR_NOT_NULL(ctx);
    CU_ASSERT_EQUAL(ctx->include_count, 0);
    CU_ASSERT_EQUAL(ctx->macro_count, 0);
    CU_ASSERT_PTR_NULL(ctx->include_paths);
    CU_ASSERT_PTR_NULL(ctx->macros);
    preprocessor_free(ctx);
}

/**
 * @brief Test include path scanning
 */
void test_preprocessor_scan_includes(void)
{
    PreprocessingContext *ctx = preprocessor_init();
    CU_ASSERT_PTR_NOT_NULL(ctx);

    // Test with current directory (should find at least system includes)
    CU_ASSERT_EQUAL(preprocessor_scan_includes(ctx, "."), CQ_SUCCESS);
    CU_ASSERT(ctx->include_count >= 2); // At least /usr/include and /usr/local/include

    preprocessor_free(ctx);
}

/**
 * @brief Test macro extraction from source
 */
void test_preprocessor_extract_macros(void)
{
    // Create a temporary test file with macros
    const char *test_content =
        "#define MAX_SIZE 100\n"
        "#define DEBUG 1\n"
        "#define VERSION \"1.0\"\n"
        "#define FUNC(x) ((x) * 2)\n"
        "\n"
        "int main() {\n"
        "    return MAX_SIZE;\n"
        "}\n";

    FILE *test_file = fopen("test_macros.c", "w");
    CU_ASSERT_PTR_NOT_NULL(test_file);
    if (test_file)
    {
        fputs(test_content, test_file);
        fclose(test_file);

        PreprocessingContext *ctx = preprocessor_init();
        CU_ASSERT_PTR_NOT_NULL(ctx);

        CU_ASSERT_EQUAL(preprocessor_extract_macros(ctx, "test_macros.c"), CQ_SUCCESS);
        CU_ASSERT(ctx->macro_count >= 3); // Should find at least MAX_SIZE, DEBUG, VERSION

        // Check if MAX_SIZE was extracted
        MacroDefinition *macro = ctx->macros;
        bool found_max_size = false;
        while (macro)
        {
            if (strcmp(macro->name, "MAX_SIZE") == 0)
            {
                found_max_size = true;
                CU_ASSERT_STRING_EQUAL(macro->value, "100");
                break;
            }
            macro = macro->next;
        }
        CU_ASSERT_TRUE(found_max_size);

        preprocessor_free(ctx);

        // Clean up test file
        remove("test_macros.c");
    }
}

/**
 * @brief Test argument building
 */
void test_preprocessor_build_args(void)
{
    PreprocessingContext *ctx = preprocessor_init();
    CU_ASSERT_PTR_NOT_NULL(ctx);

    // Add a test include path
    IncludePath *path = calloc(1, sizeof(IncludePath));
    CU_ASSERT_PTR_NOT_NULL(path);
    strcpy(path->path, "/test/include");
    ctx->include_paths = path;
    ctx->include_count = 1;

    // Add a test macro
    MacroDefinition *macro = calloc(1, sizeof(MacroDefinition));
    CU_ASSERT_PTR_NOT_NULL(macro);
    strcpy(macro->name, "TEST_MACRO");
    strcpy(macro->value, "42");
    ctx->macros = macro;
    ctx->macro_count = 1;

    const char *args[10];
    int count = preprocessor_build_args(ctx, args, 10);

    CU_ASSERT(count >= 3); // -I/test/include, -DTEST_MACRO=42, -std=c11

    // Free allocated args
    for (int i = 0; i < count; i++)
    {
        free((void *)args[i]);
    }

    preprocessor_free(ctx);
}

/**
 * @brief Test project parsing with progress
 */
void test_parse_project(void)
{
    // Initialize parsers for testing
    CU_ASSERT_EQUAL(initialize_language_parsers(), CQ_SUCCESS);

    // Test parsing current directory
    void *project_ast = parse_project(".", 50, NULL);
    CU_ASSERT_PTR_NOT_NULL(project_ast);

    // Free project AST if allocated
    if (project_ast)
    {
        free(project_ast);
    }

    shutdown_language_parsers();
}

/**
 * @brief Test project parsing with invalid parameters
 */
void test_parse_project_invalid_params(void)
{
    // Test with NULL path
    void *result = parse_project(NULL, 50, NULL);
    CU_ASSERT_PTR_NULL(result);

    // Test with zero max files
    result = parse_project(".", 0, NULL);
    CU_ASSERT_PTR_NULL(result);
}

/**
 * @brief Test file accessibility checking
 */
void test_file_accessibility(void)
{
    // Test with existing file
    CU_ASSERT_TRUE(is_file_accessible("test_parser.c"));

    // Test with non-existent file
    CU_ASSERT_FALSE(is_file_accessible("non_existent_file.xyz"));

    // Test with NULL parameter
    CU_ASSERT_FALSE(is_file_accessible(NULL));
}

/**
 * @brief Test error handling for inaccessible directories
 */
void test_scan_inaccessible_directory(void)
{
    char *files[10];

    // Test with non-existent directory
    int count = scan_directory("/non/existent/directory", files, 10);
    CU_ASSERT_EQUAL(count, -1);

    // Test with NULL path
    count = scan_directory(NULL, files, 10);
    CU_ASSERT_EQUAL(count, -1);

    // Test with NULL files array
    count = scan_directory(".", NULL, 10);
    CU_ASSERT_EQUAL(count, -1);
}

/**
 * @brief Test parsing with inaccessible files
 */
void test_parse_inaccessible_files(void)
{
    // Initialize parsers for testing
    CU_ASSERT_EQUAL(initialize_language_parsers(), CQ_SUCCESS);

    // Test parsing non-existent project
    void *result = parse_project("/non/existent/project", 10, NULL);
    CU_ASSERT_PTR_NULL(result);

    // Test parsing empty directory (create a temp empty dir for testing)
    // This would require creating a temporary directory, which is complex in unit tests
    // For now, we'll just test the basic error handling

    shutdown_language_parsers();
}

/**
 * @brief Test large file handling
 */
void test_large_file_handling(void)
{
    // Create a temporary large file for testing
    const char *large_file = "test_large_file.c";
    FILE *file = fopen(large_file, "w");
    if (file)
    {
        // Write some content to make it a valid C file
        fprintf(file, "#include <stdio.h>\n\n");
        fprintf(file, "int main() {\n");
        fprintf(file, "    printf(\"Hello World\\n\");\n");
        fprintf(file, "    return 0;\n");
        fprintf(file, "}\n");

        // Try to make it "large" by writing many lines
        for (int i = 0; i < 10000; i++)
        {
            fprintf(file, "    // Comment line %d\n", i);
        }
        fclose(file);

        // Test parsing (should succeed for reasonable file sizes)
        void *result = parse_source_file(large_file);
        // Result may be NULL due to libclang limitations in test environment
        // but the important thing is that it doesn't crash
        if (result)
        {
            free_ast_data(result);
        }

        // Clean up
        remove(large_file);
    }
}

/**
 * @brief Test malformed file handling
 */
void test_malformed_file_handling(void)
{
    // Create a malformed C file
    const char *malformed_file = "test_malformed.c";
    FILE *file = fopen(malformed_file, "w");
    if (file)
    {
        // Write malformed C code
        fprintf(file, "#include <stdio.h>\n\n");
        fprintf(file, "int main() {\n");
        fprintf(file, "    printf(\"Hello World\\n\");\n");
        fprintf(file, "    // Missing closing brace and parenthesis\n");
        fclose(file);

        // Test parsing malformed file
        void *result = parse_source_file(malformed_file);
        // libclang should handle this gracefully
        if (result)
        {
            free_ast_data(result);
        }

        // Clean up
        remove(malformed_file);
    }
}

/**
 * @brief Add parser tests to suite
 */
void add_parser_tests(CU_pSuite suite)
{
    CU_add_test(suite, "File Scanner Test", test_file_scanner);
    CU_add_test(suite, "File Scanner With Progress Test", test_file_scanner_with_progress);
    CU_add_test(suite, "File Scanner Invalid Params Test", test_file_scanner_invalid_params);
    CU_add_test(suite, "Scan Inaccessible Directory Test", test_scan_inaccessible_directory);
    CU_add_test(suite, "File Accessibility Test", test_file_accessibility);
    CU_add_test(suite, "AST Parser Test", test_ast_parser);
    CU_add_test(suite, "Language Support Test", test_language_support);
    CU_add_test(suite, "Preprocessor Init Test", test_preprocessor_init);
    CU_add_test(suite, "Preprocessor Scan Includes Test", test_preprocessor_scan_includes);
    CU_add_test(suite, "Preprocessor Extract Macros Test", test_preprocessor_extract_macros);
    CU_add_test(suite, "Preprocessor Build Args Test", test_preprocessor_build_args);
    CU_add_test(suite, "Parse Project Test", test_parse_project);
    CU_add_test(suite, "Parse Project Invalid Params Test", test_parse_project_invalid_params);
    CU_add_test(suite, "Parse Inaccessible Files Test", test_parse_inaccessible_files);
    CU_add_test(suite, "Large File Handling Test", test_large_file_handling);
    CU_add_test(suite, "Malformed File Handling Test", test_malformed_file_handling);
}
