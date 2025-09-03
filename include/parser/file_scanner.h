#ifndef FILE_SCANNER_H
#define FILE_SCANNER_H

#include <stdbool.h>
#include "cqanalyzer.h"

/**
 * @file file_scanner.h
 * @brief File system scanning functionality
 *
 * Provides functions to scan directories and identify source files
 * for code analysis.
 */

/**
 * @brief Scan directory recursively for source files
 *
 * @param path Directory path to scan
 * @param files Array to store found file paths
 * @param max_files Maximum number of files to find
 * @return Number of files found, or -1 on error
 */
int scan_directory(const char* path, char** files, int max_files);

/**
 * @brief Check if file is a supported source file
 *
 * @param filename File name to check
 * @param language Programming language
 * @return true if supported, false otherwise
 */
bool is_source_file(const char* filename, SupportedLanguage language);

#endif // FILE_SCANNER_H
