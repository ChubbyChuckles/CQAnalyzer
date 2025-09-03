#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/memory.h"
#include "utils/logger.h"

void *cq_malloc(size_t size)
{
    if (size == 0)
    {
        LOG_WARNING("Attempted to allocate 0 bytes");
        return NULL;
    }

    void *ptr = malloc(size);
    if (!ptr)
    {
        LOG_ERROR("Memory allocation failed for %zu bytes", size);
        return NULL;
    }

    LOG_DEBUG("Allocated %zu bytes at %p", size, ptr);
    return ptr;
}

void *cq_realloc(void *ptr, size_t size)
{
    if (size == 0)
    {
        cq_free(ptr);
        return NULL;
    }

    void *new_ptr = realloc(ptr, size);
    if (!new_ptr)
    {
        LOG_ERROR("Memory reallocation failed for %zu bytes", size);
        return NULL;
    }

    LOG_DEBUG("Reallocated memory to %zu bytes at %p (was %p)", size, new_ptr, ptr);
    return new_ptr;
}

void cq_free(void *ptr)
{
    if (!ptr)
    {
        return;
    }

    LOG_DEBUG("Freeing memory at %p", ptr);
    free(ptr);
}

char *cq_strdup(const char *str)
{
    if (!str)
    {
        LOG_WARNING("Attempted to duplicate NULL string");
        return NULL;
    }

    size_t len = strlen(str);
    char *dup = (char *)cq_malloc(len + 1);
    if (!dup)
    {
        LOG_ERROR("Failed to allocate memory for string duplication");
        return NULL;
    }

    strcpy(dup, str);
    LOG_DEBUG("Duplicated string: %s", str);
    return dup;
}

CQError cq_memcpy_safe(void *dest, size_t dest_size, const void *src, size_t src_size)
{
    if (!dest || !src)
    {
        LOG_ERROR("Invalid arguments for safe memcpy");
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (src_size > dest_size)
    {
        LOG_ERROR("Source size (%zu) exceeds destination size (%zu)", src_size, dest_size);
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    memcpy(dest, src, src_size);
    LOG_DEBUG("Safely copied %zu bytes", src_size);

    return CQ_SUCCESS;
}
