#include <stdio.h>

#include "analyzer/complexity_analyzer.h"
#include "parser/ast_parser.h"
#include "data/ast_types.h"
#include "utils/logger.h"

CQError analyze_function_complexity(void *ast_data, int *complexity)
{
    if (!ast_data || !complexity)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // TODO: Implement function complexity analysis
    // TODO: Count control flow statements
    // TODO: Calculate cyclomatic complexity

    LOG_WARNING("Function complexity analysis not yet implemented");
    *complexity = 1; // Default complexity

    return CQ_SUCCESS;
}

CQError analyze_file_complexity(const char *filepath, int *complexity)
{
    if (!filepath || !complexity)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Initialize AST parser if not already done
    static int parser_initialized = 0;
    if (!parser_initialized)
    {
        if (ast_parser_init() != CQ_SUCCESS)
        {
            LOG_ERROR("Failed to initialize AST parser");
            return CQ_ERROR_UNKNOWN;
        }
        parser_initialized = 1;
    }

    // Parse the source file
    void *ast_data = parse_source_file(filepath);
    if (!ast_data)
    {
        LOG_ERROR("Failed to parse file: %s", filepath);
        return CQ_ERROR_UNKNOWN;
    }

    ASTData *data = (ASTData *)ast_data;
    if (!data->project || !data->project->files)
    {
        LOG_ERROR("Invalid AST data structure");
        free_ast_data(ast_data);
        return CQ_ERROR_UNKNOWN;
    }

    // Calculate average complexity of all functions
    int total_complexity = 0;
    int function_count = 0;
    FunctionInfo *func = data->project->files->functions;
    while (func)
    {
        total_complexity += func->complexity;
        function_count++;
        func = func->next;
    }

    if (function_count > 0)
    {
        *complexity = total_complexity / function_count;
    }
    else
    {
        *complexity = 1; // Default for files with no functions
    }

    LOG_INFO("File %s complexity: %d (average of %d functions)", filepath, *complexity, function_count);

    // Clean up
    free_ast_data(ast_data);

    return CQ_SUCCESS;
}
