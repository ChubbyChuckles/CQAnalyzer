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
 * @brief Progress callback function type
 *
 * @param current Current item being processed
 * @param total Total number of items
 * @param status Optional status message
 */
typedef void (*ProgressCallback)(int current, int total, const char *status);

/**
 * @brief Scan directory recursively for source files
 *
 * @param path Directory path to scan
 * @param files Array to store found file paths
 * @param max_files Maximum number of files to find
 * @return Number of files found, or -1 on error
 */
int scan_directory(const char *path, char **files, int max_files);

/**
 * @brief Scan directory recursively for source files with progress reporting
 *
 * @param path Directory path to scan
 * @param files Array to store found file paths
 * @param max_files Maximum number of files to find
 * @param progress_callback Optional progress callback function
 * @return Number of files found, or -1 on error
 */
int scan_directory_with_progress(const char *path, char **files, int max_files, ProgressCallback progress_callback);

/**
 * @brief Check if file is a supported source file
 *
 * @param filename File name to check
 * @param language Programming language
 * @return true if supported, false otherwise
 */
bool is_source_file(const char *filename, SupportedLanguage language);

/**
 * @brief Check if a file is accessible for reading
 *
 * @param filepath Path to the file to check
 * @return true if file is accessible, false otherwise
 */
bool is_file_accessible(const char *filepath);

#endif // FILE_SCANNER_H
