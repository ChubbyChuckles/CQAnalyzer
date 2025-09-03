#ifndef AST_PARSER_H
#define AST_PARSER_H

#include "cqanalyzer.h"

/**
 * @file ast_parser.h
 * @brief AST parsing functionality using libclang
 *
 * Provides functions to parse source code and extract AST information
 * for code analysis.
 */

/**
 * @brief Initialize AST parser
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError ast_parser_init(void);

/**
 * @brief Shutdown AST parser
 */
void ast_parser_shutdown(void);

/**
 * @brief Parse source file and extract AST
 *
 * @param filepath Path to source file
 * @return Pointer to parsed AST data, or NULL on error
 */
void* parse_source_file(const char* filepath);

/**
 * @brief Free AST data
 *
 * @param ast_data AST data to free
 */
void free_ast_data(void* ast_data);

#endif // AST_PARSER_H
