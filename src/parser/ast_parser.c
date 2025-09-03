#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <clang-c/Index.h>

#include "parser/ast_parser.h"
#include "parser/generic_parser.h"
#include "parser/language_support.h"
#include "parser/preprocessor.h"
#include "data/ast_types.h"
#include "utils/logger.h"

// Global libclang index
static CXIndex clang_index = NULL;

/**
 * @brief Context for AST visitor
 */
typedef struct {
    ASTData *ast_data;
    FunctionInfo *current_function;
    int complexity_count;
} VisitorContext;

/**
 * @brief Visitor function for counting decision points
 */
static enum CXChildVisitResult count_decision_points_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    int *count = (int *)client_data;
    enum CXCursorKind kind = clang_getCursorKind(cursor);

    switch (kind) {
        case CXCursor_IfStmt:
        case CXCursor_WhileStmt:
        case CXCursor_ForStmt:
        case CXCursor_DoStmt:
        case CXCursor_SwitchStmt:
            (*count)++;
            break;
        case CXCursor_CaseStmt:
        case CXCursor_DefaultStmt:
            (*count)++;
            break;
        case CXCursor_BinaryOperator: {
            CXString spelling = clang_getCursorSpelling(cursor);
            const char *op = clang_getCString(spelling);
            if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
                (*count)++;
            }
            clang_disposeString(spelling);
            break;
        }
        case CXCursor_ConditionalOperator: // ?:
            (*count)++;
            break;
        default:
            break;
    }

    return CXChildVisit_Recurse;
}

/**
 * @brief Count decision points in a cursor subtree
 */
static void count_decision_points(CXCursor cursor, int *count) {
    clang_visitChildren(cursor, count_decision_points_visitor, count);
}

/**
 * @brief AST visitor function for libclang
 */
static enum CXChildVisitResult ast_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    VisitorContext *context = (VisitorContext *)client_data;
    ASTData *ast_data = context->ast_data;
    enum CXCursorKind kind = clang_getCursorKind(cursor);

    // Get cursor location
    CXSourceLocation location = clang_getCursorLocation(cursor);
    CXFile file;
    unsigned line, column, offset;
    clang_getFileLocation(location, &file, &line, &column, &offset);

    // Get cursor spelling (name)
    CXString cursor_spelling = clang_getCursorSpelling(cursor);
    const char *name = clang_getCString(cursor_spelling);

    switch (kind)
    {
        case CXCursor_FunctionDecl:
        {
            LOG_DEBUG("Found function: %s at line %u", name, line);

            // Create function info
            FunctionInfo *func = calloc(1, sizeof(FunctionInfo));
            if (func)
            {
                strncpy(func->name, name, sizeof(func->name) - 1);
                func->location.line = line;
                func->location.column = column;
                func->location.offset = offset;

                // Get return type
                CXType return_type = clang_getCursorResultType(cursor);
                CXString type_spelling = clang_getTypeSpelling(return_type);
                const char *type_str = clang_getCString(type_spelling);
                strncpy(func->return_type, type_str, sizeof(func->return_type) - 1);
                clang_disposeString(type_spelling);

                // Count parameters
                int num_args = clang_Cursor_getNumArguments(cursor);
                func->parameter_count = num_args;

                // Calculate cyclomatic complexity
                int decision_count = 0;
                count_decision_points(cursor, &decision_count);
                func->complexity = 1 + decision_count; // Base complexity + decision points

                LOG_DEBUG("Function %s complexity: %u", name, func->complexity);

                // Add to project's function list
                func->next = ast_data->project->files->functions;
                ast_data->project->files->functions = func;
                ast_data->project->files->function_count++;
                ast_data->project->total_functions++;
            }
            break;
        }

        case CXCursor_StructDecl:
        case CXCursor_ClassDecl:
        {
            LOG_DEBUG("Found %s: %s at line %u",
                      kind == CXCursor_StructDecl ? "struct" : "class", name, line);

            // Create class info
            ClassInfo *class_info = calloc(1, sizeof(ClassInfo));
            if (class_info)
            {
                strncpy(class_info->name, name, sizeof(class_info->name) - 1);
                class_info->location.line = line;
                class_info->location.column = column;
                class_info->location.offset = offset;

                // Add to project's class list
                class_info->next = ast_data->project->files->classes;
                ast_data->project->files->classes = class_info;
                ast_data->project->files->class_count++;
                ast_data->project->total_classes++;
            }
            break;
        }

        default:
            // Other cursor types can be handled here
            break;
    }

    clang_disposeString(cursor_spelling);
    return CXChildVisit_Continue;
}

/**
 * @brief Traverse AST and extract information
 */
static void traverse_ast(CXCursor root_cursor, ASTData *ast_data)
{
    LOG_INFO("Starting AST traversal");

    // Create file info for this translation unit
    ast_data->project->files = calloc(1, sizeof(FileInfo));
    if (!ast_data->project->files)
    {
        LOG_ERROR("Memory allocation failed for file info");
        return;
    }

    // Get file name from translation unit
    CXFile file;
    clang_getFileLocation(clang_getCursorLocation(root_cursor), &file, NULL, NULL, NULL);
    if (file)
    {
        CXString filename = clang_getFileName(file);
        const char *filename_str = clang_getCString(filename);
        if (filename_str)
        {
            strncpy(ast_data->project->files->filepath, filename_str,
                   sizeof(ast_data->project->files->filepath) - 1);
        }
        clang_disposeString(filename);
    }

    ast_data->project->file_count = 1;

    // Create visitor context
    VisitorContext context = {
        .ast_data = ast_data,
        .current_function = NULL,
        .complexity_count = 0
    };

    // Visit all children of the root cursor
    clang_visitChildren(root_cursor, ast_visitor, &context);

    LOG_INFO("AST traversal completed. Found %u functions, %u classes",
             ast_data->project->total_functions, ast_data->project->total_classes);
}

CQError ast_parser_init(void)
{
    LOG_INFO("Initializing AST parser");

    // Initialize libclang index
    clang_index = clang_createIndex(0, 0);
    if (!clang_index)
    {
        LOG_ERROR("Failed to create libclang index");
        return CQ_ERROR_UNKNOWN;
    }

    LOG_INFO("Libclang index created successfully");
    return CQ_SUCCESS;
}

void ast_parser_shutdown(void)
{
    LOG_INFO("Shutting down AST parser");

    if (clang_index)
    {
        clang_disposeIndex(clang_index);
        clang_index = NULL;
        LOG_INFO("Libclang index disposed");
    }
}

void *parse_source_file(const char *filepath)
{
    if (!filepath)
    {
        LOG_ERROR("Invalid filepath for AST parsing");
        return NULL;
    }

    if (!clang_index)
    {
        LOG_ERROR("Libclang index not initialized");
        return NULL;
    }

    LOG_INFO("Parsing source file: %s", filepath);

    // Initialize preprocessing context
    PreprocessingContext *preproc_ctx = preprocessor_init();
    if (!preproc_ctx)
    {
        LOG_ERROR("Failed to initialize preprocessing context");
        return NULL;
    }

    // Determine project root (simplified - use file's directory)
    char project_root[MAX_PATH_LENGTH];
    char *last_slash = strrchr(filepath, '/');
    if (last_slash)
    {
        size_t root_len = last_slash - filepath;
        if (root_len < sizeof(project_root))
        {
            strncpy(project_root, filepath, root_len);
            project_root[root_len] = '\0';
        }
        else
        {
            strcpy(project_root, ".");
        }
    }
    else
    {
        strcpy(project_root, ".");
    }

    // Scan for include directories
    if (preprocessor_scan_includes(preproc_ctx, project_root) != CQ_SUCCESS)
    {
        LOG_WARNING("Failed to scan include directories");
    }

    // Extract macros from the source file
    if (preprocessor_extract_macros(preproc_ctx, filepath) != CQ_SUCCESS)
    {
        LOG_WARNING("Failed to extract macros from source file");
    }

    // Build command line arguments
    const char *args[100]; // Reasonable maximum
    int arg_count = preprocessor_build_args(preproc_ctx, args, 100);

    LOG_DEBUG("Using %d preprocessing arguments for libclang", arg_count);

    // Parse the file with libclang
    CXTranslationUnit tu = clang_parseTranslationUnit(
        clang_index,
        filepath,
        args, arg_count,    // command line args
        NULL, 0,    // unsaved files
        CXTranslationUnit_None
    );

    if (!tu)
    {
        LOG_ERROR("Failed to parse translation unit for file: %s", filepath);
        preprocessor_free(preproc_ctx);
        // Free allocated args
        for (int i = 0; i < arg_count; i++)
        {
            free((void *)args[i]);
        }
        return NULL;
    }

    // Create AST data structure
    ASTData *ast_data = calloc(1, sizeof(ASTData));
    if (!ast_data)
    {
        LOG_ERROR("Memory allocation failed for AST data");
        clang_disposeTranslationUnit(tu);
        preprocessor_free(preproc_ctx);
        // Free allocated args
        for (int i = 0; i < arg_count; i++)
        {
            free((void *)args[i]);
        }
        return NULL;
    }

    ast_data->clang_index = clang_index;
    ast_data->clang_translation_unit = tu;

    // Create project structure
    ast_data->project = calloc(1, sizeof(Project));
    if (!ast_data->project)
    {
        LOG_ERROR("Memory allocation failed for project data");
        free(ast_data);
        clang_disposeTranslationUnit(tu);
        preprocessor_free(preproc_ctx);
        // Free allocated args
        for (int i = 0; i < arg_count; i++)
        {
            free((void *)args[i]);
        }
        return NULL;
    }

    // Set project path
    if (strlen(project_root) < sizeof(ast_data->project->root_path))
    {
        strcpy(ast_data->project->root_path, project_root);
    }

    // Traverse AST and extract information
    CXCursor root_cursor = clang_getTranslationUnitCursor(tu);
    traverse_ast(root_cursor, ast_data);

    // Clean up preprocessing context
    preprocessor_free(preproc_ctx);

    // Free allocated args
    for (int i = 0; i < arg_count; i++)
    {
        free((void *)args[i]);
    }

    LOG_INFO("Successfully parsed file: %s", filepath);
    return ast_data;
}

void *parse_source_file_with_detection(const char *filepath)
{
    if (!filepath)
    {
        LOG_ERROR("Invalid filepath for parsing");
        return NULL;
    }

    LOG_INFO("Detecting language for file: %s", filepath);

    // Detect language from file extension
    SupportedLanguage language = detect_language(filepath);
    if (language == LANG_UNKNOWN)
    {
        LOG_WARNING("Unknown file type for: %s", filepath);
        return NULL;
    }

    LOG_INFO("Detected language: %d for file: %s", language, filepath);

    // Get appropriate parser for the language
    ParserFunction parser = get_parser_for_language(language);
    if (!parser)
    {
        LOG_ERROR("No parser available for language: %d", language);
        return NULL;
    }

    // Parse the file using the appropriate parser
    return parser(filepath, language);
}

void free_ast_data(void *data)
{
    if (!data)
    {
        return;
    }

    ASTData *ast_data = (ASTData *)data;

    // Dispose translation unit (only for C/C++ files)
    if (ast_data->clang_translation_unit)
    {
        clang_disposeTranslationUnit((CXTranslationUnit)ast_data->clang_translation_unit);
    }

    // Free project data
    if (ast_data->project)
    {
        // Free all files
        FileInfo *file = ast_data->project->files;
        while (file)
        {
            FileInfo *next_file = file->next;

            // Free functions
            FunctionInfo *func = file->functions;
            while (func)
            {
                FunctionInfo *next_func = func->next;
                free(func);
                func = next_func;
            }

            // Free classes
            ClassInfo *class_info = file->classes;
            while (class_info)
            {
                ClassInfo *next_class = class_info->next;
                free(class_info);
                class_info = next_class;
            }

            free(file);
            file = next_file;
        }

        free(ast_data->project);
    }

    free(ast_data);
    LOG_INFO("AST data freed successfully");
}
