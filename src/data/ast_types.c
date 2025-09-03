#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data/ast_types.h"
#include "utils/logger.h"

// String Pool Implementation
CQError string_pool_init(StringPool *pool, uint32_t initial_capacity)
{
    if (!pool || initial_capacity == 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    pool->strings = (char **)malloc(initial_capacity * sizeof(char *));
    pool->hashes = (uint32_t *)malloc(initial_capacity * sizeof(uint32_t));
    pool->hash_table = (uint32_t *)malloc(initial_capacity * sizeof(uint32_t));

    if (!pool->strings || !pool->hashes || !pool->hash_table)
    {
        free(pool->strings);
        free(pool->hashes);
        free(pool->hash_table);
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    pool->count = 0;
    pool->capacity = initial_capacity;
    pool->hash_table_size = initial_capacity;

    // Initialize hash table to invalid values
    memset(pool->hash_table, 0xFF, initial_capacity * sizeof(uint32_t));

    return CQ_SUCCESS;
}

void string_pool_destroy(StringPool *pool)
{
    if (!pool)
    {
        return;
    }

    for (uint32_t i = 0; i < pool->count; i++)
    {
        free(pool->strings[i]);
    }

    free(pool->strings);
    free(pool->hashes);
    free(pool->hash_table);

    pool->strings = NULL;
    pool->hashes = NULL;
    pool->hash_table = NULL;
    pool->count = 0;
    pool->capacity = 0;
    pool->hash_table_size = 0;
}

uint32_t string_pool_intern(StringPool *pool, const char *str)
{
    if (!pool || !str)
    {
        return 0;
    }

    // Simple hash function
    uint32_t hash = 0;
    for (const char *c = str; *c; c++)
    {
        hash = hash * 31 + *c;
    }

    // Check if string already exists
    uint32_t bucket = hash % pool->hash_table_size;
    uint32_t index = pool->hash_table[bucket];

    while (index != 0xFFFFFFFF)
    {
        if (pool->hashes[index] == hash && strcmp(pool->strings[index], str) == 0)
        {
            return index;
        }
        // Linear probing
        bucket = (bucket + 1) % pool->hash_table_size;
        index = pool->hash_table[bucket];
    }

    // String not found, add it
    if (pool->count >= pool->capacity)
    {
        // Expand capacity
        uint32_t new_capacity = pool->capacity * 2;
        char **new_strings = (char **)realloc(pool->strings, new_capacity * sizeof(char *));
        uint32_t *new_hashes = (uint32_t *)realloc(pool->hashes, new_capacity * sizeof(uint32_t));

        if (!new_strings || !new_hashes)
        {
            free(new_strings);
            free(new_hashes);
            return 0;
        }

        pool->strings = new_strings;
        pool->hashes = new_hashes;
        pool->capacity = new_capacity;
    }

    // Add string
    pool->strings[pool->count] = strdup(str);
    pool->hashes[pool->count] = hash;

    // Add to hash table
    bucket = hash % pool->hash_table_size;
    while (pool->hash_table[bucket] != 0xFFFFFFFF)
    {
        bucket = (bucket + 1) % pool->hash_table_size;
    }
    pool->hash_table[bucket] = pool->count;

    return pool->count++;
}

const char *string_pool_get(const StringPool *pool, uint32_t id)
{
    if (!pool || id >= pool->count)
    {
        return NULL;
    }

    return pool->strings[id];
}

// Symbol Table Implementation
CQError symbol_table_init(SymbolTable *table, uint32_t initial_capacity)
{
    if (!table || initial_capacity == 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    table->symbol_ids = (uint32_t *)malloc(initial_capacity * sizeof(uint32_t));
    table->file_indices = (uint32_t *)malloc(initial_capacity * sizeof(uint32_t));
    table->hash_table = (uint32_t *)malloc(initial_capacity * sizeof(uint32_t));

    if (!table->symbol_ids || !table->file_indices || !table->hash_table)
    {
        free(table->symbol_ids);
        free(table->file_indices);
        free(table->hash_table);
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    table->count = 0;
    table->capacity = initial_capacity;
    table->hash_table_size = initial_capacity;

    // Initialize hash table to invalid values
    memset(table->hash_table, 0xFF, initial_capacity * sizeof(uint32_t));

    return CQ_SUCCESS;
}

void symbol_table_destroy(SymbolTable *table)
{
    if (!table)
    {
        return;
    }

    free(table->symbol_ids);
    free(table->file_indices);
    free(table->hash_table);

    table->symbol_ids = NULL;
    table->file_indices = NULL;
    table->hash_table = NULL;
    table->count = 0;
    table->capacity = 0;
    table->hash_table_size = 0;
}

CQError symbol_table_add(SymbolTable *table, uint32_t symbol_id, uint32_t file_index)
{
    if (!table)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (table->count >= table->capacity)
    {
        // Expand capacity
        uint32_t new_capacity = table->capacity * 2;
        uint32_t *new_symbol_ids = (uint32_t *)realloc(table->symbol_ids, new_capacity * sizeof(uint32_t));
        uint32_t *new_file_indices = (uint32_t *)realloc(table->file_indices, new_capacity * sizeof(uint32_t));

        if (!new_symbol_ids || !new_file_indices)
        {
            free(new_symbol_ids);
            free(new_file_indices);
            return CQ_ERROR_MEMORY_ALLOCATION;
        }

        table->symbol_ids = new_symbol_ids;
        table->file_indices = new_file_indices;
        table->capacity = new_capacity;
    }

    table->symbol_ids[table->count] = symbol_id;
    table->file_indices[table->count] = file_index;
    table->count++;

    return CQ_SUCCESS;
}

uint32_t symbol_table_find(const SymbolTable *table, uint32_t symbol_id)
{
    if (!table)
    {
        return 0xFFFFFFFF;
    }

    for (uint32_t i = 0; i < table->count; i++)
    {
        if (table->symbol_ids[i] == symbol_id)
        {
            return table->file_indices[i];
        }
    }

    return 0xFFFFFFFF;
}

// Function Array Implementation
CQError function_array_init(FunctionArray *array, uint32_t initial_capacity)
{
    if (!array || initial_capacity == 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    array->functions = (FunctionInfo *)malloc(initial_capacity * sizeof(FunctionInfo));
    if (!array->functions)
    {
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    array->count = 0;
    array->capacity = initial_capacity;

    return CQ_SUCCESS;
}

void function_array_destroy(FunctionArray *array)
{
    if (!array)
    {
        return;
    }

    free(array->functions);
    array->functions = NULL;
    array->count = 0;
    array->capacity = 0;
}

CQError function_array_add(FunctionArray *array, const FunctionInfo *func)
{
    if (!array || !func)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (array->count >= array->capacity)
    {
        // Expand capacity
        uint32_t new_capacity = array->capacity * 2;
        FunctionInfo *new_functions = (FunctionInfo *)realloc(array->functions, new_capacity * sizeof(FunctionInfo));

        if (!new_functions)
        {
            return CQ_ERROR_MEMORY_ALLOCATION;
        }

        array->functions = new_functions;
        array->capacity = new_capacity;
    }

    array->functions[array->count] = *func;
    array->count++;

    return CQ_SUCCESS;
}

FunctionInfo *function_array_get(FunctionArray *array, uint32_t index)
{
    if (!array || index >= array->count)
    {
        return NULL;
    }

    return &array->functions[index];
}

// Class Array Implementation
CQError class_array_init(ClassArray *array, uint32_t initial_capacity)
{
    if (!array || initial_capacity == 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    array->classes = (ClassInfo *)malloc(initial_capacity * sizeof(ClassInfo));
    if (!array->classes)
    {
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    array->count = 0;
    array->capacity = initial_capacity;

    return CQ_SUCCESS;
}

void class_array_destroy(ClassArray *array)
{
    if (!array)
    {
        return;
    }

    // Free method indices arrays
    for (uint32_t i = 0; i < array->count; i++)
    {
        free(array->classes[i].method_indices);
    }

    free(array->classes);
    array->classes = NULL;
    array->count = 0;
    array->capacity = 0;
}

CQError class_array_add(ClassArray *array, const ClassInfo *cls)
{
    if (!array || !cls)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (array->count >= array->capacity)
    {
        // Expand capacity
        uint32_t new_capacity = array->capacity * 2;
        ClassInfo *new_classes = (ClassInfo *)realloc(array->classes, new_capacity * sizeof(ClassInfo));

        if (!new_classes)
        {
            return CQ_ERROR_MEMORY_ALLOCATION;
        }

        array->classes = new_classes;
        array->capacity = new_capacity;
    }

    array->classes[array->count] = *cls;

    // Deep copy method indices if any
    if (cls->method_count > 0 && cls->method_indices)
    {
        array->classes[array->count].method_indices = (uint32_t *)malloc(cls->method_count * sizeof(uint32_t));
        if (array->classes[array->count].method_indices)
        {
            memcpy(array->classes[array->count].method_indices, cls->method_indices,
                   cls->method_count * sizeof(uint32_t));
        }
    }

    array->count++;

    return CQ_SUCCESS;
}

ClassInfo *class_array_get(ClassArray *array, uint32_t index)
{
    if (!array || index >= array->count)
    {
        return NULL;
    }

    return &array->classes[index];
}

// Variable Array Implementation
CQError variable_array_init(VariableArray *array, uint32_t initial_capacity)
{
    if (!array || initial_capacity == 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    array->variables = (VariableInfo *)malloc(initial_capacity * sizeof(VariableInfo));
    if (!array->variables)
    {
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    array->count = 0;
    array->capacity = initial_capacity;

    return CQ_SUCCESS;
}

void variable_array_destroy(VariableArray *array)
{
    if (!array)
    {
        return;
    }

    free(array->variables);
    array->variables = NULL;
    array->count = 0;
    array->capacity = 0;
}

CQError variable_array_add(VariableArray *array, const VariableInfo *var)
{
    if (!array || !var)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (array->count >= array->capacity)
    {
        // Expand capacity
        uint32_t new_capacity = array->capacity * 2;
        VariableInfo *new_variables = (VariableInfo *)realloc(array->variables, new_capacity * sizeof(VariableInfo));

        if (!new_variables)
        {
            return CQ_ERROR_MEMORY_ALLOCATION;
        }

        array->variables = new_variables;
        array->capacity = new_capacity;
    }

    array->variables[array->count] = *var;
    array->count++;

    return CQ_SUCCESS;
}

VariableInfo *variable_array_get(VariableArray *array, uint32_t index)
{
    if (!array || index >= array->count)
    {
        return NULL;
    }

    return &array->variables[index];
}

// File Array Implementation
CQError file_array_init(FileArray *array, uint32_t initial_capacity)
{
    if (!array || initial_capacity == 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    array->files = (FileInfo *)malloc(initial_capacity * sizeof(FileInfo));
    if (!array->files)
    {
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    array->count = 0;
    array->capacity = initial_capacity;

    return CQ_SUCCESS;
}

void file_array_destroy(FileArray *array)
{
    if (!array)
    {
        return;
    }

    free(array->files);
    array->files = NULL;
    array->count = 0;
    array->capacity = 0;
}

CQError file_array_add(FileArray *array, const FileInfo *file)
{
    if (!array || !file)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (array->count >= array->capacity)
    {
        // Expand capacity
        uint32_t new_capacity = array->capacity * 2;
        FileInfo *new_files = (FileInfo *)realloc(array->files, new_capacity * sizeof(FileInfo));

        if (!new_files)
        {
            return CQ_ERROR_MEMORY_ALLOCATION;
        }

        array->files = new_files;
        array->capacity = new_capacity;
    }

    array->files[array->count] = *file;
    array->count++;

    return CQ_SUCCESS;
}

FileInfo *file_array_get(FileArray *array, uint32_t index)
{
    if (!array || index >= array->count)
    {
        return NULL;
    }

    return &array->files[index];
}

// Project Implementation
CQError project_init(Project *project, const char *root_path, uint32_t initial_capacity)
{
    if (!project || !root_path || initial_capacity == 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Initialize string pool
    CQError result = string_pool_init(&project->string_pool, initial_capacity);
    if (result != CQ_SUCCESS)
    {
        return result;
    }

    // Initialize arrays
    result = function_array_init(&project->functions, initial_capacity);
    if (result != CQ_SUCCESS)
    {
        string_pool_destroy(&project->string_pool);
        return result;
    }

    result = class_array_init(&project->classes, initial_capacity);
    if (result != CQ_SUCCESS)
    {
        function_array_destroy(&project->functions);
        string_pool_destroy(&project->string_pool);
        return result;
    }

    result = variable_array_init(&project->variables, initial_capacity);
    if (result != CQ_SUCCESS)
    {
        class_array_destroy(&project->classes);
        function_array_destroy(&project->functions);
        string_pool_destroy(&project->string_pool);
        return result;
    }

    result = file_array_init(&project->files, initial_capacity);
    if (result != CQ_SUCCESS)
    {
        variable_array_destroy(&project->variables);
        class_array_destroy(&project->classes);
        function_array_destroy(&project->functions);
        string_pool_destroy(&project->string_pool);
        return result;
    }

    // Initialize symbol table
    result = symbol_table_init(&project->symbol_table, initial_capacity);
    if (result != CQ_SUCCESS)
    {
        file_array_destroy(&project->files);
        variable_array_destroy(&project->variables);
        class_array_destroy(&project->classes);
        function_array_destroy(&project->functions);
        string_pool_destroy(&project->string_pool);
        return result;
    }

    // Set root path
    project->root_path_id = string_pool_intern(&project->string_pool, root_path);

    // Initialize counters
    project->total_functions = 0;
    project->total_classes = 0;
    project->total_variables = 0;
    project->dependency_graph = NULL;

    return CQ_SUCCESS;
}

void project_destroy(Project *project)
{
    if (!project)
    {
        return;
    }

    // Destroy all components
    if (project->dependency_graph)
    {
        // TODO: Implement dependency graph destruction
        free(project->dependency_graph);
    }

    symbol_table_destroy(&project->symbol_table);
    file_array_destroy(&project->files);
    variable_array_destroy(&project->variables);
    class_array_destroy(&project->classes);
    function_array_destroy(&project->functions);
    string_pool_destroy(&project->string_pool);
}

CQError project_add_file(Project *project, const char *filepath, SupportedLanguage language, FileInfo **file_info)
{
    if (!project || !filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    FileInfo file_data = {0};
    file_data.filepath_id = string_pool_intern(&project->string_pool, filepath);
    file_data.language = language;
    file_data.total_lines = 0;
    file_data.code_lines = 0;
    file_data.comment_lines = 0;
    file_data.blank_lines = 0;
    file_data.function_start = project->functions.count;
    file_data.function_count = 0;
    file_data.class_start = project->classes.count;
    file_data.class_count = 0;
    file_data.variable_start = project->variables.count;
    file_data.variable_count = 0;

    CQError result = file_array_add(&project->files, &file_data);
    if (result == CQ_SUCCESS && file_info)
    {
        *file_info = file_array_get(&project->files, project->files.count - 1);
    }

    return result;
}

CQError project_add_function(Project *project, const FunctionInfo *func, uint32_t *func_id)
{
    if (!project || !func)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    CQError result = function_array_add(&project->functions, func);
    if (result == CQ_SUCCESS)
    {
        project->total_functions++;
        if (func_id)
        {
            *func_id = project->functions.count - 1;
        }
    }

    return result;
}

CQError project_add_class(Project *project, const ClassInfo *cls, uint32_t *class_id)
{
    if (!project || !cls)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    CQError result = class_array_add(&project->classes, cls);
    if (result == CQ_SUCCESS)
    {
        project->total_classes++;
        if (class_id)
        {
            *class_id = project->classes.count - 1;
        }
    }

    return result;
}

CQError project_add_variable(Project *project, const VariableInfo *var, uint32_t *var_id)
{
    if (!project || !var)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    CQError result = variable_array_add(&project->variables, var);
    if (result == CQ_SUCCESS)
    {
        project->total_variables++;
        if (var_id)
        {
            *var_id = project->variables.count - 1;
        }
    }

    return result;
}

// Validation functions
bool ast_data_validate(const ASTData *data)
{
    if (!data)
    {
        return false;
    }

    return project_validate(data->project);
}

bool project_validate(const Project *project)
{
    if (!project)
    {
        return false;
    }

    // Basic validation - check if arrays are initialized
    return project->functions.functions != NULL &&
           project->classes.classes != NULL &&
           project->variables.variables != NULL &&
           project->files.files != NULL &&
           project->string_pool.strings != NULL;
}

bool function_info_validate(const FunctionInfo *func, const Project *project)
{
    if (!func || !project)
    {
        return false;
    }

    // Check if string IDs are valid
    return func->name_id < project->string_pool.count &&
           func->return_type_id < project->string_pool.count;
}

bool class_info_validate(const ClassInfo *cls, const Project *project)
{
    if (!cls || !project)
    {
        return false;
    }

    // Check if string ID is valid
    return cls->name_id < project->string_pool.count;
}

bool variable_info_validate(const VariableInfo *var, const Project *project)
{
    if (!var || !project)
    {
        return false;
    }

    // Check if string IDs are valid
    return var->name_id < project->string_pool.count &&
           var->type_id < project->string_pool.count;
}

bool file_info_validate(const FileInfo *file, const Project *project)
{
    if (!file || !project)
    {
        return false;
    }

    // Check if string ID is valid
    return file->filepath_id < project->string_pool.count;
}