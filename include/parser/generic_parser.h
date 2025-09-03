#ifndef GENERIC_PARSER_H
#define GENERIC_PARSER_H

#include "cqanalyzer.h"
#include "data/ast_types.h"

/**
 * @file generic_parser.h
 * @brief Generic parser interface for multiple programming languages
 *
 * Provides a unified interface for parsing different programming languages
 * and extracting AST information.
 */

/**
 * @brief Parser function type for different languages
 *
 * @param filepath Path to source file
 * @param language Detected language
 * @return Pointer to parsed AST data, or NULL on error
 */
typedef void *(*ParserFunction)(const char *filepath, SupportedLanguage language);

/**
 * @brief Get parser function for a specific language
 *
 * @param language Programming language
 * @return Parser function pointer, or NULL if not supported
 */
ParserFunction get_parser_for_language(SupportedLanguage language);

/**
 * @brief Initialize all language parsers
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError initialize_language_parsers(void);

/**
 * @brief Shutdown all language parsers
 */
void shutdown_language_parsers(void);

#endif // GENERIC_PARSER_H