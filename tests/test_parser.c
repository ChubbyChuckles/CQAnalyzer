#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser/file_scanner.h"
#include "parser/ast_parser.h"
#include "parser/language_support.h"
#include "parser/preprocessor.h"

/**
 * @brief Test file scanning
 */
void test_file_scanner(void)
{
    // TODO: Implement file scanner tests
    CU_PASS("File scanner test placeholder");
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
 * @brief Add parser tests to suite
 */
void add_parser_tests(CU_pSuite suite)
{
    CU_add_test(suite, "File Scanner Test", test_file_scanner);
    CU_add_test(suite, "AST Parser Test", test_ast_parser);
    CU_add_test(suite, "Language Support Test", test_language_support);
    CU_add_test(suite, "Preprocessor Init Test", test_preprocessor_init);
    CU_add_test(suite, "Preprocessor Scan Includes Test", test_preprocessor_scan_includes);
    CU_add_test(suite, "Preprocessor Extract Macros Test", test_preprocessor_extract_macros);
    CU_add_test(suite, "Preprocessor Build Args Test", test_preprocessor_build_args);
}
