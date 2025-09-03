#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils/string_utils.h"
#include "utils/logger.h"

CQError cq_strcpy_safe(char *dest, size_t dest_size, const char *src)
{
    if (!dest || !src || dest_size == 0)
    {
        LOG_ERROR("Invalid arguments for safe strcpy");
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    size_t src_len = strlen(src);
    if (src_len >= dest_size)
    {
        LOG_ERROR("Source string too long (%zu) for destination (%zu)", src_len, dest_size);
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    strcpy(dest, src);
    return CQ_SUCCESS;
}

CQError cq_strcat_safe(char *dest, size_t dest_size, const char *src)
{
    if (!dest || !src || dest_size == 0)
    {
        LOG_ERROR("Invalid arguments for safe strcat");
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    size_t dest_len = strlen(dest);
    size_t src_len = strlen(src);

    if (dest_len + src_len >= dest_size)
    {
        LOG_ERROR("Concatenation would exceed destination size (%zu + %zu >= %zu)",
                  dest_len, src_len, dest_size);
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    strcat(dest, src);
    return CQ_SUCCESS;
}

bool cq_starts_with(const char *str, const char *prefix)
{
    if (!str || !prefix)
    {
        return false;
    }

    size_t prefix_len = strlen(prefix);
    if (strlen(str) < prefix_len)
    {
        return false;
    }

    return strncmp(str, prefix, prefix_len) == 0;
}

bool cq_ends_with(const char *str, const char *suffix)
{
    if (!str || !suffix)
    {
        return false;
    }

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (str_len < suffix_len)
    {
        return false;
    }

    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

void cq_to_lower(char *str)
{
    if (!str)
    {
        return;
    }

    for (char *p = str; *p; p++)
    {
        *p = tolower(*p);
    }
}

void cq_to_upper(char *str)
{
    if (!str)
    {
        return;
    }

    for (char *p = str; *p; p++)
    {
        *p = toupper(*p);
    }
}

void cq_trim(char *str)
{
    if (!str)
    {
        return;
    }

    // Trim leading whitespace
    char *start = str;
    while (*start && isspace(*start))
    {
        start++;
    }

    // Trim trailing whitespace
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end))
    {
        *end-- = '\0';
    }

    // Move string if we trimmed leading whitespace
    if (start != str)
    {
        memmove(str, start, strlen(start) + 1);
    }
}

const char *cq_get_file_extension(const char *path)
{
    if (!path)
    {
        return NULL;
    }

    const char *dot = strrchr(path, '.');
    if (!dot || dot == path)
    {
        return NULL;
    }

    return dot;
}
