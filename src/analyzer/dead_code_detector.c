#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer/dead_code_detector.h"
#include "parser/ast_parser.h"
#include "data/ast_types.h"
#include "utils/logger.h"
#include "cqanalyzer.h"

/**
 * @brief Initialize dead code list
 */
static void init_dead_code_list(DeadCodeList *list)
{
    if (!list)
        return;

    list->results = NULL;
    list->count = 0;
    list->capacity = 0;
}

/**
 * @brief Add result to dead code list
 */
static CQError add_dead_code_result(DeadCodeList *list, const char *name, const char *type, SourceLocation location)
{
    if (!list || !name || !type)
        return CQ_ERROR_INVALID_ARGUMENT;

    // Expand capacity if needed
    if (list->count >= list->capacity)
    {
        uint32_t new_capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        DeadCodeResult *new_results = realloc(list->results, new_capacity * sizeof(DeadCodeResult));
        if (!new_results)
        {
            LOG_ERROR("Failed to allocate memory for dead code results");
            return CQ_ERROR_MEMORY_ALLOCATION;
        }
        list->results = new_results;
        list->capacity = new_capacity;
    }

    // Add the result
    DeadCodeResult *result = &list->results[list->count++];
    strncpy(result->symbol_name, name, sizeof(result->symbol_name) - 1);
    strncpy(result->symbol_type, type, sizeof(result->symbol_type) - 1);
    result->location = location;

    LOG_DEBUG("Added dead code result: %s (%s) at line %u", name, type, location.line);

    return CQ_SUCCESS;
}

CQError detect_dead_code_in_file(const char *filepath, DeadCodeList *dead_code_list)
{
    if (!filepath || !dead_code_list)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    LOG_INFO("Detecting dead code in file: %s", filepath);

    // Initialize the result list
    init_dead_code_list(dead_code_list);

    // For now, return empty list as proper AST parsing is not implemented
    // TODO: Implement proper dead code detection using AST data

    LOG_INFO("Found %u dead code items in file: %s", dead_code_list->count, filepath);

    return CQ_SUCCESS;
}

CQError detect_dead_code_in_project(const char *project_root, DeadCodeList *dead_code_list)
{
    if (!project_root || !dead_code_list)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    LOG_INFO("Detecting dead code in project: %s", project_root);

    // Initialize the result list
    init_dead_code_list(dead_code_list);

    // For now, implement a simple file-by-file approach
    // In a real implementation, this would scan the entire project
    // and build a global symbol table for cross-file analysis

    LOG_WARNING("Project-wide dead code detection not fully implemented yet");
    LOG_WARNING("Currently only supports single-file analysis");

    return CQ_SUCCESS;
}

void free_dead_code_list(DeadCodeList *dead_code_list)
{
    if (!dead_code_list)
        return;

    if (dead_code_list->results)
    {
        free(dead_code_list->results);
        dead_code_list->results = NULL;
    }

    dead_code_list->count = 0;
    dead_code_list->capacity = 0;

    LOG_DEBUG("Freed dead code list");
}