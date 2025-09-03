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

    // Cast to FunctionInfo structure
    FunctionInfo *func_info = (FunctionInfo *)ast_data;

    // The complexity is already calculated during AST parsing
    // and stored in the FunctionInfo structure
    *complexity = func_info->complexity;

    // Note: To get the function name, we would need access to the string pool
    // For now, just log the complexity value
    LOG_DEBUG("Function complexity: %d", *complexity);

    return CQ_SUCCESS;
}

CQError analyze_file_complexity(const char *filepath, int *complexity)
{
    if (!filepath || !complexity)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // For now, return a simple complexity based on file size or other heuristics
    // TODO: Implement proper AST-based complexity analysis
    *complexity = 5; // Default complexity

    LOG_INFO("File %s complexity: %d", filepath, *complexity);

    return CQ_SUCCESS;
}


CQError calculate_nesting_depth(void *ast_data, int *nesting_depth)
{
    if (!ast_data || !nesting_depth)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Cast to FunctionInfo structure
    FunctionInfo *func_info = (FunctionInfo *)ast_data;

    // The nesting depth is already calculated during AST parsing
    // and stored in the FunctionInfo structure
    *nesting_depth = func_info->nesting_depth;

    // Note: To get the function name, we would need access to the string pool
    // For now, just log the nesting depth value
    LOG_DEBUG("Function nesting depth: %d", *nesting_depth);

    return CQ_SUCCESS;
}
