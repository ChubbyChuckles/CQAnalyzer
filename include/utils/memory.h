#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include <string.h>
#include "cqanalyzer.h"

/**
 * @file memory.h
 * @brief Memory management utilities
 *
 * Provides safe memory allocation and deallocation functions
 * with error handling and logging.
 */

/**
 * @brief Safe memory allocation with error checking
 *
 * @param size Size in bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void *cq_malloc(size_t size);

/**
 * @brief Safe memory reallocation with error checking
 *
 * @param ptr Pointer to existing memory
 * @param size New size in bytes
 * @return Pointer to reallocated memory, or NULL on failure
 */
void *cq_realloc(void *ptr, size_t size);

/**
 * @brief Safe memory deallocation
 *
 * @param ptr Pointer to memory to free
 */
void cq_free(void *ptr);

/**
 * @brief Safe string duplication
 *
 * @param str String to duplicate
 * @return Pointer to duplicated string, or NULL on failure
 */
char *cq_strdup(const char *str);

/**
 * @brief Safe memory copy with bounds checking
 *
 * @param dest Destination buffer
 * @param dest_size Size of destination buffer
 * @param src Source buffer
 * @param src_size Size of source buffer
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError cq_memcpy_safe(void *dest, size_t dest_size, const void *src, size_t src_size);

#endif // MEMORY_H
