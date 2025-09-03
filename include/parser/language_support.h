#ifndef LANGUAGE_SUPPORT_H
#define LANGUAGE_SUPPORT_H

#include "cqanalyzer.h"

/**
 * @file language_support.h
 * @brief Language-specific functionality
 *
 * Provides language detection and language-specific parsing support.
 */

/**
 * @brief Detect programming language from file extension
 *
 * @param filename File name to analyze
 * @return Detected language, or LANG_UNKNOWN if not recognized
 */
SupportedLanguage detect_language(const char* filename);

/**
 * @brief Get file extensions for a programming language
 *
 * @param language Programming language
 * @return Array of file extensions (NULL-terminated)
 */
const char** get_language_extensions(SupportedLanguage language);

/**
 * @brief Check if language is supported
 *
 * @param language Programming language
 * @return true if supported, false otherwise
 */
bool is_language_supported(SupportedLanguage language);

#endif // LANGUAGE_SUPPORT_H
