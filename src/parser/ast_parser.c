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
    VariableInfo *current_variables;
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
 * @brief Context for nesting depth visitor
 */
typedef struct {
    int current_depth;
    int max_depth;
} NestingContext;

/**
 * @brief Visitor function for calculating nesting depth
 */
static enum CXChildVisitResult calculate_nesting_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    NestingContext *context = (NestingContext *)client_data;
    enum CXCursorKind kind = clang_getCursorKind(cursor);

    // Check if this is a control structure that increases nesting
    int is_control_structure = 0;
    switch (kind) {
        case CXCursor_IfStmt:
        case CXCursor_WhileStmt:
        case CXCursor_ForStmt:
        case CXCursor_DoStmt:
        case CXCursor_SwitchStmt:
            is_control_structure = 1;
            break;
        default:
            break;
    }

    if (is_control_structure) {
        // Increase depth for this control structure
        context->current_depth++;

        // Update max depth if current is greater
        if (context->current_depth > context->max_depth) {
            context->max_depth = context->current_depth;
        }

        // Visit children with increased depth
        clang_visitChildren(cursor, calculate_nesting_visitor, client_data);

        // Decrease depth when leaving this control structure
        context->current_depth--;

        return CXChildVisit_Continue;
    }

    // For non-control structures, just continue visiting children
    return CXChildVisit_Recurse;
}

/**
 * @brief Calculate nesting depth for a function cursor
 */
static void calculate_function_nesting_depth(CXCursor function_cursor, int *max_depth) {
    NestingContext context = {
        .current_depth = 0,
        .max_depth = 0
    };

    clang_visitChildren(function_cursor, calculate_nesting_visitor, &context);

    *max_depth = context.max_depth;
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
            FunctionInfo func_data = {0};
            func_data.name_id = string_pool_intern(&ast_data->project->string_pool, name);
            func_data.location.line = line;
            func_data.location.column = column;
            func_data.location.offset = offset;

            // Get return type
            CXType return_type = clang_getCursorResultType(cursor);
            CXString type_spelling = clang_getTypeSpelling(return_type);
            const char *type_str = clang_getCString(type_spelling);
            func_data.return_type_id = string_pool_intern(&ast_data->project->string_pool, type_str);
            clang_disposeString(type_spelling);

            // Count parameters
            int num_args = clang_Cursor_getNumArguments(cursor);
            func_data.parameter_count = num_args;

            // Calculate cyclomatic complexity
            int decision_count = 0;
            count_decision_points(cursor, &decision_count);
            func_data.complexity = 1 + decision_count; // Base complexity + decision points

            // Calculate nesting depth
            int max_nesting = 0;
            calculate_function_nesting_depth(cursor, &max_nesting);
            func_data.nesting_depth = max_nesting;

            LOG_DEBUG("Function %s complexity: %u, nesting depth: %u", name, func_data.complexity, func_data.nesting_depth);

            // Add to project's function array
            uint32_t func_id;
            if (project_add_function(ast_data->project, &func_data, &func_id) == CQ_SUCCESS) {
                LOG_DEBUG("Added function %s with ID %u", name, func_id);
            } else {
                LOG_ERROR("Failed to add function %s", name);
            }
            break;
        }

        case CXCursor_StructDecl:
        case CXCursor_ClassDecl:
        {
            LOG_DEBUG("Found %s: %s at line %u",
                      kind == CXCursor_StructDecl ? "struct" : "class", name, line);

            // Create class info
            ClassInfo class_data = {0};
            class_data.name_id = string_pool_intern(&ast_data->project->string_pool, name);
            class_data.location.line = line;
            class_data.location.column = column;
            class_data.location.offset = offset;

            // Add to project's class array
            uint32_t class_id;
            if (project_add_class(ast_data->project, &class_data, &class_id) == CQ_SUCCESS) {
                LOG_DEBUG("Added class %s with ID %u", name, class_id);
            } else {
                LOG_ERROR("Failed to add class %s", name);
            }
            break;
        }

        case CXCursor_VarDecl:
        {
            LOG_DEBUG("Found variable: %s at line %u", name, line);

            // Create variable info
            VariableInfo var_data = {0};
            var_data.name_id = string_pool_intern(&ast_data->project->string_pool, name);
            var_data.location.line = line;
            var_data.location.column = column;
            var_data.location.offset = offset;
            var_data.usage_count = 0; // Will be updated during usage tracking

            // Get variable type
            CXType var_type = clang_getCursorType(cursor);
            CXString type_spelling = clang_getTypeSpelling(var_type);
            const char *type_str = clang_getCString(type_spelling);
            if (type_str)
            {
                var_data.type_id = string_pool_intern(&ast_data->project->string_pool, type_str);
            }
            clang_disposeString(type_spelling);

            // Add to project's variable array
            uint32_t var_id;
            if (project_add_variable(ast_data->project, &var_data, &var_id) == CQ_SUCCESS) {
                LOG_DEBUG("Added variable %s with ID %u", name, var_id);
            } else {
                LOG_ERROR("Failed to add variable %s", name);
            }
            break;
        }

        case CXCursor_DeclRefExpr:
        {
            // This is a reference to a declared symbol (variable or function usage)
            LOG_DEBUG("Found reference to: %s at line %u", name, line);

            // Track usage for variables
            for (uint32_t i = 0; i < ast_data->project->variables.count; i++) {
                VariableInfo *var = variable_array_get(&ast_data->project->variables, i);
                if (var) {
                    const char *var_name = string_pool_get(&ast_data->project->string_pool, var->name_id);
                    if (strcmp(var_name, name) == 0) {
                        var->usage_count++;
                        LOG_DEBUG("Incremented usage count for variable %s to %u", name, var->usage_count);
                        break;
                    }
                }
            }

            // Track usage for functions
            for (uint32_t i = 0; i < ast_data->project->functions.count; i++) {
                FunctionInfo *func = function_array_get(&ast_data->project->functions, i);
                if (func) {
                    const char *func_name = string_pool_get(&ast_data->project->string_pool, func->name_id);
                    if (strcmp(func_name, name) == 0) {
                        func->usage_count++;
                        LOG_DEBUG("Incremented usage count for function %s to %u", name, func->usage_count);
                        break;
                    }
                }
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

    // Get file name from translation unit
    CXFile file;
    clang_getFileLocation(clang_getCursorLocation(root_cursor), &file, NULL, NULL, NULL);
    const char *filename_str = NULL;
    if (file)
    {
        CXString filename = clang_getFileName(file);
        filename_str = clang_getCString(filename);
        clang_disposeString(filename);
    }

    // Create file info using proper initialization
    FileInfo file_info = {0};
    if (filename_str) {
        file_info.filepath_id = string_pool_intern(&ast_data->project->string_pool, filename_str);
    }
    file_info.language = LANG_C; // Assume C for now, could be detected

    // Add file to project
    FileInfo *added_file;
    if (project_add_file(ast_data->project, filename_str, LANG_C, &added_file) != CQ_SUCCESS) {
        LOG_ERROR("Failed to add file to project");
        return;
    }

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

    // Initialize project with proper data structures
    if (project_init(ast_data->project, project_root, 100) != CQ_SUCCESS) {
        LOG_ERROR("Failed to initialize project data structures");
        free(ast_data->project);
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
    ast_data->project->root_path_id = string_pool_intern(&ast_data->project->string_pool, project_root);

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
        // The project_destroy function will handle freeing all arrays and string pool
        project_destroy(ast_data->project);
    }

    free(ast_data);
    LOG_INFO("AST data freed successfully");
}
