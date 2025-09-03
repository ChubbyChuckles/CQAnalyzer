#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "parser/generic_parser.h"
#include "parser/ast_parser.h"
#include "parser/file_scanner.h"
#include "utils/logger.h"

// Forward declarations for language-specific parsers
static void *parse_python_file(const char *filepath, SupportedLanguage language);
static void *parse_java_file(const char *filepath, SupportedLanguage language);
static void *parse_javascript_file(const char *filepath, SupportedLanguage language);
static void *parse_typescript_file(const char *filepath, SupportedLanguage language);

// Wrapper function for C/C++ parsing
static void *parse_c_cpp_file(const char *filepath, SupportedLanguage language)
{
    (void)language; // Unused parameter

    // Check file accessibility before parsing
    if (!is_file_accessible(filepath))
    {
        LOG_ERROR("Cannot access C/C++ file for parsing: %s", filepath);
        return NULL;
    }

    // Check file size to prevent parsing extremely large files
    struct stat st;
    if (stat(filepath, &st) == 0)
    {
        const long long MAX_FILE_SIZE = 50 * 1024 * 1024; // 50MB limit
        if (st.st_size > MAX_FILE_SIZE)
        {
            LOG_WARNING("Skipping large file: %s (size: %lld bytes)", filepath, (long long)st.st_size);
            return NULL;
        }
    }

    void *result = parse_source_file(filepath);
    if (!result)
    {
        LOG_WARNING("C/C++ parser failed for file: %s", filepath);
    }
    return result;
}

ParserFunction get_parser_for_language(SupportedLanguage language)
{
    switch (language)
    {
    case LANG_C:
    case LANG_CPP:
        return parse_c_cpp_file; // Wrapper for existing libclang parser
    case LANG_PYTHON:
        return parse_python_file;
    case LANG_JAVA:
        return parse_java_file;
    case LANG_JAVASCRIPT:
        return parse_javascript_file;
    case LANG_TYPESCRIPT:
        return parse_typescript_file;
    default:
        LOG_WARNING("Unsupported language: %d", language);
        return NULL;
    }
}

CQError initialize_language_parsers(void)
{
    LOG_INFO("Initializing language parsers");

    // Initialize C/C++ parser (libclang)
    CQError result = ast_parser_init();
    if (result != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to initialize C/C++ parser");
        return result;
    }

    // Other parsers don't need initialization for now
    LOG_INFO("Language parsers initialized successfully");
    return CQ_SUCCESS;
}

void shutdown_language_parsers(void)
{
    LOG_INFO("Shutting down language parsers");

    // Shutdown C/C++ parser
    ast_parser_shutdown();

    // Other parsers don't need shutdown for now
    LOG_INFO("Language parsers shut down");
}

// Basic Python parser implementation
static void *parse_python_file(const char *filepath, SupportedLanguage language)
{
    LOG_INFO("Parsing Python file: %s", filepath);

    // Check file accessibility
    if (!is_file_accessible(filepath))
    {
        LOG_ERROR("Cannot access Python file for parsing: %s", filepath);
        return NULL;
    }

    // Check file size
    struct stat st;
    if (stat(filepath, &st) == 0)
    {
        const long long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit for Python files
        if (st.st_size > MAX_FILE_SIZE)
        {
            LOG_WARNING("Skipping large Python file: %s (size: %lld bytes)", filepath, (long long)st.st_size);
            return NULL;
        }
    }

    // For now, return a basic AST structure
    // This is a placeholder - full implementation would parse Python AST

    ASTData *ast_data = calloc(1, sizeof(ASTData));
    if (!ast_data)
    {
        LOG_ERROR("Memory allocation failed for Python AST data");
        return NULL;
    }

    ast_data->project = calloc(1, sizeof(Project));
    if (!ast_data->project)
    {
        LOG_ERROR("Memory allocation failed for Python project data");
        free(ast_data);
        return NULL;
    }

    // Set basic project info
    strncpy(ast_data->project->root_path, filepath, sizeof(ast_data->project->root_path) - 1);

    // Create file info
    ast_data->project->files = calloc(1, sizeof(FileInfo));
    if (!ast_data->project->files)
    {
        LOG_ERROR("Memory allocation failed for Python file info");
        free(ast_data->project);
        free(ast_data);
        return NULL;
    }

    strncpy(ast_data->project->files->filepath, filepath, sizeof(ast_data->project->files->filepath) - 1);
    ast_data->project->files->language = language;
    ast_data->project->file_count = 1;

    LOG_INFO("Python file parsing completed (basic implementation)");
    return ast_data;
}

// Basic Java parser implementation
static void *parse_java_file(const char *filepath, SupportedLanguage language)
{
    LOG_INFO("Parsing Java file: %s", filepath);

    // Check file accessibility
    if (!is_file_accessible(filepath))
    {
        LOG_ERROR("Cannot access Java file for parsing: %s", filepath);
        return NULL;
    }

    // Check file size
    struct stat st;
    if (stat(filepath, &st) == 0)
    {
        const long long MAX_FILE_SIZE = 20 * 1024 * 1024; // 20MB limit for Java files
        if (st.st_size > MAX_FILE_SIZE)
        {
            LOG_WARNING("Skipping large Java file: %s (size: %lld bytes)", filepath, (long long)st.st_size);
            return NULL;
        }
    }

    // Placeholder implementation similar to Python
    ASTData *ast_data = calloc(1, sizeof(ASTData));
    if (!ast_data)
    {
        LOG_ERROR("Memory allocation failed for Java AST data");
        return NULL;
    }

    ast_data->project = calloc(1, sizeof(Project));
    if (!ast_data->project)
    {
        LOG_ERROR("Memory allocation failed for Java project data");
        free(ast_data);
        return NULL;
    }

    strncpy(ast_data->project->root_path, filepath, sizeof(ast_data->project->root_path) - 1);

    ast_data->project->files = calloc(1, sizeof(FileInfo));
    if (!ast_data->project->files)
    {
        LOG_ERROR("Memory allocation failed for Java file info");
        free(ast_data->project);
        free(ast_data);
        return NULL;
    }

    strncpy(ast_data->project->files->filepath, filepath, sizeof(ast_data->project->files->filepath) - 1);
    ast_data->project->files->language = language;
    ast_data->project->file_count = 1;

    LOG_INFO("Java file parsing completed (basic implementation)");
    return ast_data;
}

// Basic JavaScript parser implementation
static void *parse_javascript_file(const char *filepath, SupportedLanguage language)
{
    LOG_INFO("Parsing JavaScript file: %s", filepath);

    // Check file accessibility
    if (!is_file_accessible(filepath))
    {
        LOG_ERROR("Cannot access JavaScript file for parsing: %s", filepath);
        return NULL;
    }

    // Check file size
    struct stat st;
    if (stat(filepath, &st) == 0)
    {
        const long long MAX_FILE_SIZE = 15 * 1024 * 1024; // 15MB limit for JavaScript files
        if (st.st_size > MAX_FILE_SIZE)
        {
            LOG_WARNING("Skipping large JavaScript file: %s (size: %lld bytes)", filepath, (long long)st.st_size);
            return NULL;
        }
    }

    // Placeholder implementation
    ASTData *ast_data = calloc(1, sizeof(ASTData));
    if (!ast_data)
    {
        LOG_ERROR("Memory allocation failed for JavaScript AST data");
        return NULL;
    }

    ast_data->project = calloc(1, sizeof(Project));
    if (!ast_data->project)
    {
        LOG_ERROR("Memory allocation failed for JavaScript project data");
        free(ast_data);
        return NULL;
    }

    strncpy(ast_data->project->root_path, filepath, sizeof(ast_data->project->root_path) - 1);

    ast_data->project->files = calloc(1, sizeof(FileInfo));
    if (!ast_data->project->files)
    {
        LOG_ERROR("Memory allocation failed for JavaScript file info");
        free(ast_data->project);
        free(ast_data);
        return NULL;
    }

    strncpy(ast_data->project->files->filepath, filepath, sizeof(ast_data->project->files->filepath) - 1);
    ast_data->project->files->language = language;
    ast_data->project->file_count = 1;

    LOG_INFO("JavaScript file parsing completed (basic implementation)");
    return ast_data;
}

// Basic TypeScript parser implementation
static void *parse_typescript_file(const char *filepath, SupportedLanguage language)
{
    LOG_INFO("Parsing TypeScript file: %s", filepath);

    // Check file accessibility
    if (!is_file_accessible(filepath))
    {
        LOG_ERROR("Cannot access TypeScript file for parsing: %s", filepath);
        return NULL;
    }

    // Check file size
    struct stat st;
    if (stat(filepath, &st) == 0)
    {
        const long long MAX_FILE_SIZE = 15 * 1024 * 1024; // 15MB limit for TypeScript files
        if (st.st_size > MAX_FILE_SIZE)
        {
            LOG_WARNING("Skipping large TypeScript file: %s (size: %lld bytes)", filepath, (long long)st.st_size);
            return NULL;
        }
    }

    // Placeholder implementation - TypeScript is similar to JavaScript
    ASTData *ast_data = calloc(1, sizeof(ASTData));
    if (!ast_data)
    {
        LOG_ERROR("Memory allocation failed for TypeScript AST data");
        return NULL;
    }

    ast_data->project = calloc(1, sizeof(Project));
    if (!ast_data->project)
    {
        LOG_ERROR("Memory allocation failed for TypeScript project data");
        free(ast_data);
        return NULL;
    }

    strncpy(ast_data->project->root_path, filepath, sizeof(ast_data->project->root_path) - 1);

    ast_data->project->files = calloc(1, sizeof(FileInfo));
    if (!ast_data->project->files)
    {
        LOG_ERROR("Memory allocation failed for TypeScript file info");
        free(ast_data->project);
        free(ast_data);
        return NULL;
    }

    strncpy(ast_data->project->files->filepath, filepath, sizeof(ast_data->project->files->filepath) - 1);
    ast_data->project->files->language = language;
    ast_data->project->file_count = 1;

    LOG_INFO("TypeScript file parsing completed (basic implementation)");
    return ast_data;
}

/**
 * @brief Parse an entire project with progress reporting
 *
 * @param project_path Path to the project root directory
 * @param max_files Maximum number of files to parse
 * @param progress_callback Optional progress callback function
 * @return Pointer to parsed project AST data, or NULL on error
 */
void *parse_project(const char *project_path, int max_files, void (*progress_callback)(int, int, const char *))
{
    if (!project_path)
    {
        LOG_ERROR("Invalid project path");
        return NULL;
    }

    LOG_INFO("Starting project parsing: %s", project_path);

    // Allocate memory for file paths
    char **file_paths = calloc(max_files, sizeof(char *));
    if (!file_paths)
    {
        LOG_ERROR("Memory allocation failed for file paths");
        return NULL;
    }

    // Scan directory for source files with progress
    int file_count = scan_directory_with_progress(project_path, file_paths, max_files,
                                                 (ProgressCallback)progress_callback);
    if (file_count == -1)
    {
        LOG_ERROR("Failed to scan directory");
        free(file_paths);
        return NULL;
    }

    if (file_count == 0)
    {
        LOG_WARNING("No source files found in project");
        free(file_paths);
        return NULL;
    }

    LOG_INFO("Found %d files to parse", file_count);

    // Track parsing statistics
    int parse_errors = 0;
    int access_errors = 0;
    int skipped_files = 0;

    // Create main AST data structure
    ASTData *project_ast = calloc(1, sizeof(ASTData));
    if (!project_ast)
    {
        LOG_ERROR("Memory allocation failed for project AST");
        // Free allocated file paths
        for (int i = 0; i < file_count; i++)
        {
            free(file_paths[i]);
        }
        free(file_paths);
        return NULL;
    }

    // Create project structure
    project_ast->project = calloc(1, sizeof(Project));
    if (!project_ast->project)
    {
        LOG_ERROR("Memory allocation failed for project structure");
        free(project_ast);
        // Free allocated file paths
        for (int i = 0; i < file_count; i++)
        {
            free(file_paths[i]);
        }
        free(file_paths);
        return NULL;
    }

    // Set project root path
    strncpy(project_ast->project->root_path, project_path, sizeof(project_ast->project->root_path) - 1);

    // Parse each file
    FileInfo *last_file = NULL;
    int parsed_count = 0;

    for (int i = 0; i < file_count; i++)
    {
        // Report progress for parsing
        if (progress_callback)
        {
            char status_msg[256];
            snprintf(status_msg, sizeof(status_msg), "Parsing file: %s", file_paths[i]);
            progress_callback(i + 1, file_count, status_msg);
        }

        // Determine language from file extension
        SupportedLanguage language = LANG_UNKNOWN;
        const char *ext = strrchr(file_paths[i], '.');
        if (ext)
        {
            ext++; // Skip the dot
            if (strcmp(ext, "c") == 0 || strcmp(ext, "h") == 0)
                language = LANG_C;
            else if (strcmp(ext, "cpp") == 0 || strcmp(ext, "hpp") == 0 || strcmp(ext, "cc") == 0 ||
                     strcmp(ext, "cxx") == 0 || strcmp(ext, "hxx") == 0)
                language = LANG_CPP;
            else if (strcmp(ext, "java") == 0)
                language = LANG_JAVA;
            else if (strcmp(ext, "py") == 0)
                language = LANG_PYTHON;
            else if (strcmp(ext, "js") == 0)
                language = LANG_JAVASCRIPT;
            else if (strcmp(ext, "ts") == 0)
                language = LANG_TYPESCRIPT;
        }

        if (language == LANG_UNKNOWN)
        {
            LOG_WARNING("Unknown file type, skipping: %s", file_paths[i]);
            free(file_paths[i]);
            continue;
        }

        // Get appropriate parser
        ParserFunction parser = get_parser_for_language(language);
        if (!parser)
        {
            LOG_WARNING("No parser available for language, skipping: %s", file_paths[i]);
            free(file_paths[i]);
            continue;
        }

        // Check if file is accessible before parsing
        if (!is_file_accessible(file_paths[i]))
        {
            LOG_WARNING("Skipping inaccessible file: %s", file_paths[i]);
            access_errors++;
            free(file_paths[i]);
            continue;
        }

        // Parse the file
        void *file_ast = parser(file_paths[i], language);
        if (!file_ast)
        {
            LOG_WARNING("Failed to parse file (possibly malformed or too large): %s", file_paths[i]);
            parse_errors++;
            free(file_paths[i]);
            continue;
        }

        // For now, we have placeholder parsers that return ASTData
        // In a real implementation, we'd need to merge these into the project structure
        // For this basic implementation, we'll just count successful parses
        free(file_ast); // Placeholder cleanup

        parsed_count++;
        free(file_paths[i]);
    }

    free(file_paths);

    // Calculate total errors
    int total_errors = access_errors + parse_errors + skipped_files;

    // Set final project statistics
    project_ast->project->file_count = parsed_count;
    project_ast->project->total_functions = 0; // Would be calculated from actual parsing
    project_ast->project->total_classes = 0;   // Would be calculated from actual parsing

    // Log comprehensive parsing results
    LOG_INFO("Project parsing completed:");
    LOG_INFO("  Total files found: %d", file_count);
    LOG_INFO("  Successfully parsed: %d", parsed_count);
    LOG_INFO("  Access errors: %d", access_errors);
    LOG_INFO("  Parse errors: %d", parse_errors);
    LOG_INFO("  Skipped files: %d", skipped_files);

    // Handle case where no files were successfully parsed
    if (parsed_count == 0)
    {
        if (file_count == 0)
        {
            LOG_WARNING("No source files found in project directory");
        }
        else
        {
            LOG_ERROR("Failed to parse any files. Check file permissions and formats.");
            // Clean up and return NULL to indicate complete failure
            if (project_ast->project)
            {
                free(project_ast->project);
            }
            free(project_ast);
            return NULL;
        }
    }

    // Report completion with error summary
    if (progress_callback)
    {
        if (total_errors > 0)
        {
            char status_msg[256];
            snprintf(status_msg, sizeof(status_msg),
                    "Parsing completed: %d parsed, %d errors", parsed_count, total_errors);
            progress_callback(file_count, file_count, status_msg);
        }
        else
        {
            progress_callback(file_count, file_count, "Parsing completed successfully");
        }
    }

    return project_ast;
}