#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdbool.h>
#include <string.h>
#include "cqanalyzer.h"

/**
 * @file string_utils.h
 * @brief String manipulation utilities
 *
 * Provides safe string operations and utilities for text processing.
 */

/**
 * @brief Safe string copy with bounds checking
 *
 * @param dest Destination buffer
 * @param dest_size Size of destination buffer
 * @param src Source string
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError cq_strcpy_safe(char* dest, size_t dest_size, const char* src);

/**
 * @brief Safe string concatenation with bounds checking
 *
 * @param dest Destination buffer
 * @param dest_size Size of destination buffer
 * @param src Source string
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError cq_strcat_safe(char* dest, size_t dest_size, const char* src);

/**
 * @brief Check if string starts with prefix
 *
 * @param str String to check
 * @param prefix Prefix to look for
 * @return true if string starts with prefix, false otherwise
 */
bool cq_starts_with(const char* str, const char* prefix);

/**
 * @brief Check if string ends with suffix
 *
 * @param str String to check
 * @param suffix Suffix to look for
 * @return true if string ends with suffix, false otherwise
 */
bool cq_ends_with(const char* str, const char* suffix);

/**
 * @brief Convert string to lowercase
 *
 * @param str String to convert (modified in place)
 */
void cq_to_lower(char* str);

/**
 * @brief Convert string to uppercase
 *
 * @param str String to convert (modified in place)
 */
void cq_to_upper(char* str);

/**
 * @brief Trim whitespace from both ends of string
 *
 * @param str String to trim (modified in place)
 */
void cq_trim(char* str);

/**
 * @brief Get file extension from path
 *
 * @param path File path
 * @return Pointer to extension (including dot), or NULL if no extension
 */
const char* cq_get_file_extension(const char* path);

#endif // STRING_UTILS_H
