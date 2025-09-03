#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "parser/file_scanner.h"
#include "utils/logger.h"

/**
 * @brief Helper function for recursive directory scanning
 *
 * @param path Current directory path
 * @param files Array to store found file paths
 * @param max_files Maximum number of files to find
 * @param count Current count of files found
 * @return 0 on success, -1 on error
 */
static int scan_directory_recursive(const char *path, char **files, int max_files, int *count)
{
    DIR *dir = opendir(path);
    if (!dir)
    {
        LOG_ERROR("Failed to open directory: %s", path);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[MAX_PATH_LENGTH];
        int ret = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (ret < 0 || ret >= (int)sizeof(full_path))
        {
            LOG_WARNING("Path too long: %s/%s", path, entry->d_name);
            continue;
        }

        struct stat st;
        if (lstat(full_path, &st) == -1)
        {
            LOG_WARNING("Failed to stat: %s", full_path);
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            // Recurse into subdirectory
            if (scan_directory_recursive(full_path, files, max_files, count) == -1)
            {
                closedir(dir);
                return -1;
            }
        }
        else if (S_ISREG(st.st_mode))
        {
            // Check if it's a source file
            SupportedLanguage langs[] = {LANG_C, LANG_CPP, LANG_JAVA, LANG_PYTHON, LANG_JAVASCRIPT, LANG_TYPESCRIPT};
            bool is_source = false;
            for (size_t i = 0; i < sizeof(langs) / sizeof(langs[0]); i++)
            {
                if (is_source_file(entry->d_name, langs[i]))
                {
                    is_source = true;
                    break;
                }
            }

            if (is_source)
            {
                if (*count < max_files)
                {
                    files[*count] = strdup(full_path);
                    if (!files[*count])
                    {
                        LOG_ERROR("Memory allocation failed for file path");
                        closedir(dir);
                        return -1;
                    }
                    (*count)++;
                }
                else
                {
                    LOG_WARNING("Maximum file limit reached (%d)", max_files);
                }
            }
        }
        // Symbolic links are ignored to prevent cycles and permission issues
    }

    closedir(dir);
    return 0;
}

/**
 * @brief Scan directory recursively for source files
 *
 * @param path Directory path to scan
 * @param files Array to store found file paths
 * @param max_files Maximum number of files to find
 * @return Number of files found, or -1 on error
 */
int scan_directory(const char *path, char **files, int max_files)
{
    if (!path || !files)
    {
        LOG_ERROR("Invalid arguments to scan_directory");
        return -1;
    }

    LOG_INFO("Scanning directory: %s", path);

    int count = 0;
    if (scan_directory_recursive(path, files, max_files, &count) == -1)
    {
        return -1;
    }

    LOG_INFO("Found %d source files", count);
    return count;
}

/**
 * @brief Check if file is a supported source file
 *
 * @param filename File name to check
 * @param language Programming language
 * @return true if supported, false otherwise
 */
bool is_source_file(const char *filename, SupportedLanguage language)
{
    if (!filename)
    {
        return false;
    }

    const char *ext = strrchr(filename, '.');
    if (!ext)
    {
        return false;
    }
    ext++; // Skip the dot

    switch (language)
    {
        case LANG_C:
            return strcmp(ext, "c") == 0 || strcmp(ext, "h") == 0;
        case LANG_CPP:
            return strcmp(ext, "cpp") == 0 || strcmp(ext, "hpp") == 0 ||
                   strcmp(ext, "cc") == 0 || strcmp(ext, "cxx") == 0 ||
                   strcmp(ext, "hxx") == 0 || strcmp(ext, "h") == 0;
        case LANG_JAVA:
            return strcmp(ext, "java") == 0;
        case LANG_PYTHON:
            return strcmp(ext, "py") == 0;
        case LANG_JAVASCRIPT:
            return strcmp(ext, "js") == 0;
        case LANG_TYPESCRIPT:
            return strcmp(ext, "ts") == 0;
        default:
            return false;
    }
}
