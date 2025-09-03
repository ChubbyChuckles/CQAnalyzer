#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include "parser/preprocessor.h"
#include "utils/logger.h"

/**
 * @brief Check if directory contains header files
 */
static bool has_header_files(const char *path)
{
    DIR *dir = opendir(path);
    if (!dir)
    {
        return false;
    }

    struct dirent *entry;
    bool has_headers = false;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        const char *ext = strrchr(entry->d_name, '.');
        if (ext && (strcmp(ext, ".h") == 0 || strcmp(ext, ".hpp") == 0))
        {
            has_headers = true;
            break;
        }
    }

    closedir(dir);
    return has_headers;
}

/**
 * @brief Recursively scan for include directories
 */
static void scan_include_dirs_recursive(PreprocessingContext *context, const char *path, int depth)
{
    if (depth > 5) // Limit recursion depth
    {
        return;
    }

    DIR *dir = opendir(path);
    if (!dir)
    {
        return;
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
            continue;
        }

        struct stat st;
        if (lstat(full_path, &st) == -1)
        {
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            // Check common include directory names
            if (strcmp(entry->d_name, "include") == 0 ||
                strcmp(entry->d_name, "inc") == 0 ||
                strncmp(entry->d_name, "include", 7) == 0 ||
                has_header_files(full_path))
            {
                // Add to include paths
                IncludePath *new_path = calloc(1, sizeof(IncludePath));
                if (new_path)
                {
                    strncpy(new_path->path, full_path, sizeof(new_path->path) - 1);
                    new_path->next = context->include_paths;
                    context->include_paths = new_path;
                    context->include_count++;
                    LOG_DEBUG("Added include path: %s", full_path);
                }
            }

            // Recurse into subdirectory
            scan_include_dirs_recursive(context, full_path, depth + 1);
        }
    }

    closedir(dir);
}

/**
 * @brief Extract macro definitions from a line of code
 */
static void extract_macro_from_line(PreprocessingContext *context, const char *line)
{
    // Skip leading whitespace
    while (*line && isspace(*line))
    {
        line++;
    }

    // Check if it's a #define directive
    if (strncmp(line, "#define", 7) != 0)
    {
        return;
    }

    line += 7; // Skip "#define"

    // Skip whitespace
    while (*line && isspace(*line))
    {
        line++;
    }

    // Extract macro name
    const char *name_start = line;
    while (*line && !isspace(*line) && *line != '(')
    {
        line++;
    }

    size_t name_len = line - name_start;
    if (name_len == 0 || name_len >= MAX_NAME_LENGTH)
    {
        return;
    }

    char macro_name[MAX_NAME_LENGTH];
    strncpy(macro_name, name_start, name_len);
    macro_name[name_len] = '\0';

    // Skip whitespace and parameters if any
    while (*line && isspace(*line))
    {
        line++;
    }

    // Skip function-like macro parameters
    if (*line == '(')
    {
        int paren_count = 1;
        line++;
        while (*line && paren_count > 0)
        {
            if (*line == '(') paren_count++;
            else if (*line == ')') paren_count--;
            line++;
        }
        while (*line && isspace(*line))
        {
            line++;
        }
    }

    // Extract macro value
    const char *value_start = line;
    while (*line && *line != '\n' && *line != '\r')
    {
        line++;
    }

    size_t value_len = line - value_start;
    if (value_len >= MAX_VALUE_LENGTH)
    {
        value_len = MAX_VALUE_LENGTH - 1;
    }

    // Add macro definition
    MacroDefinition *new_macro = calloc(1, sizeof(MacroDefinition));
    if (new_macro)
    {
        strncpy(new_macro->name, macro_name, sizeof(new_macro->name) - 1);
        if (value_len > 0)
        {
            strncpy(new_macro->value, value_start, value_len);
            new_macro->value[value_len] = '\0';
        }
        new_macro->next = context->macros;
        context->macros = new_macro;
        context->macro_count++;
        LOG_DEBUG("Extracted macro: %s = %s", macro_name, new_macro->value);
    }
}

PreprocessingContext *preprocessor_init(void)
{
    LOG_INFO("Initializing preprocessor");

    PreprocessingContext *context = calloc(1, sizeof(PreprocessingContext));
    if (!context)
    {
        LOG_ERROR("Failed to allocate preprocessing context");
        return NULL;
    }

    LOG_INFO("Preprocessor initialized successfully");
    return context;
}

CQError preprocessor_scan_includes(PreprocessingContext *context, const char *project_root)
{
    if (!context || !project_root)
    {
        LOG_ERROR("Invalid arguments to preprocessor_scan_includes");
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    LOG_INFO("Scanning for include directories in: %s", project_root);

    // Add standard system include paths
    const char *system_paths[] = {
        "/usr/include",
        "/usr/local/include",
        NULL
    };

    for (int i = 0; system_paths[i]; i++)
    {
        IncludePath *new_path = calloc(1, sizeof(IncludePath));
        if (new_path)
        {
            strncpy(new_path->path, system_paths[i], sizeof(new_path->path) - 1);
            new_path->next = context->include_paths;
            context->include_paths = new_path;
            context->include_count++;
        }
    }

    // Scan project directory
    scan_include_dirs_recursive(context, project_root, 0);

    LOG_INFO("Found %u include paths", context->include_count);
    return CQ_SUCCESS;
}

CQError preprocessor_extract_macros(PreprocessingContext *context, const char *filepath)
{
    if (!context || !filepath)
    {
        LOG_ERROR("Invalid arguments to preprocessor_extract_macros");
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    LOG_INFO("Extracting macros from: %s", filepath);

    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        LOG_ERROR("Failed to open file for macro extraction: %s", filepath);
        return CQ_ERROR_FILE_NOT_FOUND;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file))
    {
        extract_macro_from_line(context, line);
    }

    fclose(file);
    LOG_INFO("Extracted %u macros from %s", context->macro_count, filepath);
    return CQ_SUCCESS;
}

int preprocessor_build_args(PreprocessingContext *context, const char **args, int max_args)
{
    if (!context || !args)
    {
        return 0;
    }

    int arg_count = 0;

    // Add include paths
    IncludePath *path = context->include_paths;
    while (path && arg_count < max_args - 1)
    {
        char include_arg[MAX_PATH_LENGTH + 3]; // "-I" + path + null
        int ret = snprintf(include_arg, sizeof(include_arg), "-I%s", path->path);
        if (ret > 0 && ret < (int)sizeof(include_arg))
        {
            args[arg_count] = strdup(include_arg);
            if (args[arg_count])
            {
                arg_count++;
            }
        }
        path = path->next;
    }

    // Add macro definitions
    MacroDefinition *macro = context->macros;
    while (macro && arg_count < max_args - 1)
    {
        char macro_arg[MAX_NAME_LENGTH + MAX_VALUE_LENGTH + 10]; // "-D" + name + "=" + value + null
        int ret;
        if (strlen(macro->value) > 0)
        {
            ret = snprintf(macro_arg, sizeof(macro_arg), "-D%s=%s", macro->name, macro->value);
        }
        else
        {
            ret = snprintf(macro_arg, sizeof(macro_arg), "-D%s", macro->name);
        }

        if (ret > 0 && ret < (int)sizeof(macro_arg))
        {
            args[arg_count] = strdup(macro_arg);
            if (args[arg_count])
            {
                arg_count++;
            }
        }
        macro = macro->next;
    }

    // Add standard arguments
    if (arg_count < max_args)
    {
        args[arg_count++] = "-std=c11";
    }

    return arg_count;
}

void preprocessor_free(PreprocessingContext *context)
{
    if (!context)
    {
        return;
    }

    LOG_INFO("Freeing preprocessing context");

    // Free include paths
    IncludePath *path = context->include_paths;
    while (path)
    {
        IncludePath *next = path->next;
        free(path);
        path = next;
    }

    // Free macros
    MacroDefinition *macro = context->macros;
    while (macro)
    {
        MacroDefinition *next = macro->next;
        free(macro);
        macro = next;
    }

    free(context);
    LOG_INFO("Preprocessing context freed");
}