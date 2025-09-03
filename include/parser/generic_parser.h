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

/**
 * @brief Parse an entire project with progress reporting
 *
 * @param project_path Path to the project root directory
 * @param max_files Maximum number of files to parse
 * @param progress_callback Optional progress callback function
 * @return Pointer to parsed project AST data, or NULL on error
 */
void *parse_project(const char *project_path, int max_files, void (*progress_callback)(int, int, const char *));

#endif // GENERIC_PARSER_H