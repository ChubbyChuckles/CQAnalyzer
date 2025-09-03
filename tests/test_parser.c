#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "parser/file_scanner.h"
#include "parser/ast_parser.h"
#include "parser/language_support.h"

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
 * @brief Add parser tests to suite
 */
void add_parser_tests(CU_pSuite suite)
{
    CU_add_test(suite, "File Scanner Test", test_file_scanner);
    CU_add_test(suite, "AST Parser Test", test_ast_parser);
    CU_add_test(suite, "Language Support Test", test_language_support);
}
