#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
 * @param progress_callback Optional progress callback
 * @param total_dirs Total directories to scan (for progress estimation)
 * @param current_dir Current directory counter
 * @return 0 on success, -1 on error
 */
static int scan_directory_recursive(const char *path, char **files, int max_files, int *count,
                                    ProgressCallback progress_callback, int total_dirs, int *current_dir)
{
    DIR *dir = opendir(path);
    if (!dir)
    {
        // Check specific error types for better error handling
        if (errno == EACCES)
        {
            LOG_WARNING("Permission denied accessing directory: %s", path);
        }
        else if (errno == ENOENT)
        {
            LOG_WARNING("Directory does not exist: %s", path);
        }
        else if (errno == ENOTDIR)
        {
            LOG_WARNING("Path is not a directory: %s", path);
        }
        else
        {
            LOG_ERROR("Failed to open directory: %s (errno: %d)", path, errno);
        }
        return -1;
    }

    // Report progress for directory scanning
    if (progress_callback && total_dirs > 0)
    {
        (*current_dir)++;
        char status_msg[256];
        snprintf(status_msg, sizeof(status_msg), "Scanning directory: %s", path);
        progress_callback(*current_dir, total_dirs, status_msg);
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
            LOG_ERROR("Path too long, skipping file: %s/%s (max length: %d)", path, entry->d_name, MAX_PATH_LENGTH);
            continue;
        }

        struct stat st;
        if (lstat(full_path, &st) == -1)
        {
            // Handle specific stat errors
            if (errno == EACCES)
            {
                LOG_WARNING("Permission denied accessing file: %s", full_path);
            }
            else if (errno == ENOENT)
            {
                LOG_WARNING("File no longer exists: %s", full_path);
            }
            else if (errno == ENOTDIR)
            {
                LOG_WARNING("Path component is not a directory: %s", full_path);
            }
            else
            {
                LOG_WARNING("Failed to stat file: %s (errno: %d)", full_path, errno);
            }
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            // Recurse into subdirectory
            if (scan_directory_recursive(full_path, files, max_files, count,
                                       progress_callback, total_dirs, current_dir) == -1)
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
    return scan_directory_with_progress(path, files, max_files, NULL);
}

/**
 * @brief Scan directory recursively for source files with progress reporting
 *
 * @param path Directory path to scan
 * @param files Array to store found file paths
 * @param max_files Maximum number of files to find
 * @param progress_callback Optional progress callback function
 * @return Number of files found, or -1 on error
 */
int scan_directory_with_progress(const char *path, char **files, int max_files, ProgressCallback progress_callback)
{
    if (!path || !files)
    {
        LOG_ERROR("Invalid arguments to scan_directory_with_progress");
        return -1;
    }

    LOG_INFO("Scanning directory: %s", path);

    // For progress estimation, we could count directories first, but for simplicity
    // we'll use a basic approach. In a real implementation, you might want to
    // do a first pass to count directories.
    int total_dirs_estimate = 10; // Rough estimate
    int current_dir = 0;

    int count = 0;
    if (scan_directory_recursive(path, files, max_files, &count,
                               progress_callback, total_dirs_estimate, &current_dir) == -1)
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

/**
 * @brief Check if a file is accessible for reading
 *
 * @param filepath Path to the file to check
 * @return true if file is accessible, false otherwise
 */
bool is_file_accessible(const char *filepath)
{
    if (!filepath)
    {
        return false;
    }

    // Check if file exists and is readable
    if (access(filepath, R_OK) == 0)
    {
        return true;
    }

    // Log specific access errors
    if (errno == EACCES)
    {
        LOG_WARNING("Permission denied reading file: %s", filepath);
    }
    else if (errno == ENOENT)
    {
        LOG_WARNING("File does not exist: %s", filepath);
    }
    else if (errno == EISDIR)
    {
        LOG_WARNING("Path is a directory, not a file: %s", filepath);
    }
    else
    {
        LOG_WARNING("File not accessible: %s (errno: %d)", filepath, errno);
    }

    return false;
}
