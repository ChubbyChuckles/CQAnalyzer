#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "data/ast_types.h"
#include "utils/logger.h"
#include "utils/memory.h"

// Hash function for strings
static uint32_t hash_string(const char *str)
{
    uint32_t hash = 0;
    while (*str)
    {
        hash = (hash * 31) + (uint32_t)*str++;
    }
    return hash;
}

// Hash function for uint32_t
static uint32_t hash_uint32(uint32_t value)
{
    value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = (value >> 16) ^ value;
    return value;
}

// Dynamic array growth factor
#define GROWTH_FACTOR 1.5f
#define INITIAL_CAPACITY 16

// String Pool Implementation
CQError string_pool_init(StringPool *pool, uint32_t initial_capacity)
{
    if (!pool || initial_capacity == 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    pool->capacity = initial_capacity;
    pool->count = 0;
    pool->hash_table_size = initial_capacity * 2; // Load factor 0.5

    pool->strings = (char **)malloc(sizeof(char *) * pool->capacity);
    pool->hashes = (uint32_t *)malloc(sizeof(uint32_t) * pool->capacity);
    pool->hash_table = (uint32_t *)malloc(sizeof(uint32_t) * pool->hash_table_size);

    if (!pool->strings || !pool->hashes || !pool->hash_table)
    {
        free(pool->strings);
        free(pool->hashes);
        free(pool->hash_table);
        LOG_ERROR("Failed to allocate memory for string pool");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    // Initialize hash table with invalid indices
    memset(pool->hash_table, 0xFF, sizeof(uint32_t) * pool->hash_table_size);

    LOG_DEBUG("String pool initialized with capacity %u", initial_capacity);
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

    memset(pool, 0, sizeof(StringPool));
    LOG_DEBUG("String pool destroyed");
}

uint32_t string_pool_intern(StringPool *pool, const char *str)
{
    if (!pool || !str)
    {
        return UINT32_MAX;
    }

    uint32_t hash = hash_string(str);
    uint32_t bucket = hash % pool->hash_table_size;

    // Linear probing for collision resolution
    for (uint32_t i = 0; i < pool->hash_table_size; i++)
    {
        uint32_t index = pool->hash_table[(bucket + i) % pool->hash_table_size];
        if (index == UINT32_MAX)
        {
            // Empty slot found, add new string
            break;
        }
        if (index < pool->count && pool->hashes[index] == hash &&
            strcmp(pool->strings[index], str) == 0)
        {
            // String already exists
            return index;
        }
    }

    // Need to add new string
    if (pool->count >= pool->capacity)
    {
        // Grow the pool
        uint32_t new_capacity = (uint32_t)(pool->capacity * GROWTH_FACTOR);
        if (new_capacity <= pool->capacity)
        {
            new_capacity = pool->capacity + INITIAL_CAPACITY;
        }

        char **new_strings = (char **)realloc(pool->strings, sizeof(char *) * new_capacity);
        uint32_t *new_hashes = (uint32_t *)realloc(pool->hashes, sizeof(uint32_t) * new_capacity);

        if (!new_strings || !new_hashes)
        {
            LOG_ERROR("Failed to grow string pool");
            return UINT32_MAX;
        }

        pool->strings = new_strings;
        pool->hashes = new_hashes;
        pool->capacity = new_capacity;
    }

    // Add the string
    pool->strings[pool->count] = strdup(str);
    if (!pool->strings[pool->count])
    {
        LOG_ERROR("Failed to duplicate string");
        return UINT32_MAX;
    }

    pool->hashes[pool->count] = hash;

    // Update hash table
    uint32_t bucket = hash % pool->hash_table_size;
    for (uint32_t i = 0; i < pool->hash_table_size; i++)
    {
        uint32_t slot = (bucket + i) % pool->hash_table_size;
        if (pool->hash_table[slot] == UINT32_MAX)
        {
            pool->hash_table[slot] = pool->count;
            break;
        }
    }

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

    table->capacity = initial_capacity;
    table->count = 0;
    table->hash_table_size = initial_capacity * 2;

    table->symbol_ids = (uint32_t *)malloc(sizeof(uint32_t) * table->capacity);
    table->file_indices = (uint32_t *)malloc(sizeof(uint32_t) * table->capacity);
    table->hash_table = (uint32_t *)malloc(sizeof(uint32_t) * table->hash_table_size);

    if (!table->symbol_ids || !table->file_indices || !table->hash_table)
    {
        free(table->symbol_ids);
        free(table->file_indices);
        free(table->hash_table);
        LOG_ERROR("Failed to allocate memory for symbol table");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    memset(table->hash_table, 0xFF, sizeof(uint32_t) * table->hash_table_size);

    LOG_DEBUG("Symbol table initialized with capacity %u", initial_capacity);
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

    memset(table, 0, sizeof(SymbolTable));
    LOG_DEBUG("Symbol table destroyed");
}

CQError symbol_table_add(SymbolTable *table, uint32_t symbol_id, uint32_t file_index)
{
    if (!table)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (table->count >= table->capacity)
    {
        uint32_t new_capacity = (uint32_t)(table->capacity * GROWTH_FACTOR);
        if (new_capacity <= table->capacity)
        {
            new_capacity = table->capacity + INITIAL_CAPACITY;
        }

        uint32_t *new_symbol_ids = (uint32_t *)realloc(table->symbol_ids, sizeof(uint32_t) * new_capacity);
        uint32_t *new_file_indices = (uint32_t *)realloc(table->file_indices, sizeof(uint32_t) * new_capacity);

        if (!new_symbol_ids || !new_file_indices)
        {
            LOG_ERROR("Failed to grow symbol table");
            return CQ_ERROR_MEMORY_ALLOCATION;
        }

        table->symbol_ids = new_symbol_ids;
        table->file_indices = new_file_indices;
        table->capacity = new_capacity;
    }

    table->symbol_ids[table->count] = symbol_id;
    table->file_indices[table->count] = file_index;

    // Update hash table
    uint32_t hash = hash_uint32(symbol_id);
    uint32_t bucket = hash % table->hash_table_size;
    for (uint32_t i = 0; i < table->hash_table_size; i++)
    {
        uint32_t slot = (bucket + i) % table->hash_table_size;
        if (table->hash_table[slot] == UINT32_MAX)
        {
            table->hash_table[slot] = table->count;
            break;
        }
    }

    table->count++;
    return CQ_SUCCESS;
}

uint32_t symbol_table_find(const SymbolTable *table, uint32_t symbol_id)
{
    if (!table)
    {
        return UINT32_MAX;
    }

    uint32_t hash = hash_uint32(symbol_id);
    uint32_t bucket = hash % table->hash_table_size;

    for (uint32_t i = 0; i < table->hash_table_size; i++)
    {
        uint32_t slot = (bucket + i) % table->hash_table_size;
        uint32_t index = table->hash_table[slot];
        if (index == UINT32_MAX)
        {
            break;
        }
        if (index < table->count && table->symbol_ids[index] == symbol_id)
        {
            return table->file_indices[index];
        }
    }

    return UINT32_MAX;
}

// Function Array Implementation
CQError function_array_init(FunctionArray *array, uint32_t initial_capacity)
{
    if (!array || initial_capacity == 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    array->capacity = initial_capacity;
    array->count = 0;
    array->functions = (FunctionInfo *)malloc(sizeof(FunctionInfo) * array->capacity);

    if (!array->functions)
    {
        LOG_ERROR("Failed to allocate memory for function array");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    LOG_DEBUG("Function array initialized with capacity %u", initial_capacity);
    return CQ_SUCCESS;
}

void function_array_destroy(FunctionArray *array)
{
    if (!array)
    {
        return;
    }

    free(array->functions);
    memset(array, 0, sizeof(FunctionArray));
    LOG_DEBUG("Function array destroyed");
}

CQError function_array_add(FunctionArray *array, const FunctionInfo *func)
{
    if (!array || !func)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (array->count >= array->capacity)
    {
        uint32_t new_capacity = (uint32_t)(array->capacity * GROWTH_FACTOR);
        if (new_capacity <= array->capacity)
        {
            new_capacity = array->capacity + INITIAL_CAPACITY;
        }

        FunctionInfo *new_functions = (FunctionInfo *)realloc(array->functions, sizeof(FunctionInfo) * new_capacity);
        if (!new_functions)
        {
            LOG_ERROR("Failed to grow function array");
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

    array->capacity = initial_capacity;
    array->count = 0;
    array->classes = (ClassInfo *)malloc(sizeof(ClassInfo) * array->capacity);

    if (!array->classes)
    {
        LOG_ERROR("Failed to allocate memory for class array");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    LOG_DEBUG("Class array initialized with capacity %u", initial_capacity);
    return CQ_SUCCESS;
}

void class_array_destroy(ClassArray *array)
{
    if (!array)
    {
        return;
    }

    for (uint32_t i = 0; i < array->count; i++)
    {
        free(array->classes[i].method_indices);
    }

    free(array->classes);
    memset(array, 0, sizeof(ClassArray));
    LOG_DEBUG("Class array destroyed");
}

CQError class_array_add(ClassArray *array, const ClassInfo *cls)
{
    if (!array || !cls)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (array->count >= array->capacity)
    {
        uint32_t new_capacity = (uint32_t)(array->capacity * GROWTH_FACTOR);
        if (new_capacity <= array->capacity)
        {
            new_capacity = array->capacity + INITIAL_CAPACITY;
        }

        ClassInfo *new_classes = (ClassInfo *)realloc(array->classes, sizeof(ClassInfo) * new_capacity);
        if (!new_classes)
        {
            LOG_ERROR("Failed to grow class array");
            return CQ_ERROR_MEMORY_ALLOCATION;
        }

        array->classes = new_classes;
        array->capacity = new_capacity;
    }

    array->classes[array->count] = *cls;

    // Deep copy method indices if present
    if (cls->method_indices && cls->method_count > 0)
    {
        array->classes[array->count].method_indices = (uint32_t *)malloc(sizeof(uint32_t) * cls->method_count);
        if (!array->classes[array->count].method_indices)
        {
            LOG_ERROR("Failed to allocate method indices");
            return CQ_ERROR_MEMORY_ALLOCATION;
        }
        memcpy(array->classes[array->count].method_indices, cls->method_indices, sizeof(uint32_t) * cls->method_count);
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

    array->capacity = initial_capacity;
    array->count = 0;
    array->variables = (VariableInfo *)malloc(sizeof(VariableInfo) * array->capacity);

    if (!array->variables)
    {
        LOG_ERROR("Failed to allocate memory for variable array");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    LOG_DEBUG("Variable array initialized with capacity %u", initial_capacity);
    return CQ_SUCCESS;
}

void variable_array_destroy(VariableArray *array)
{
    if (!array)
    {
        return;
    }

    free(array->variables);
    memset(array, 0, sizeof(VariableArray));
    LOG_DEBUG("Variable array destroyed");
}

CQError variable_array_add(VariableArray *array, const VariableInfo *var)
{
    if (!array || !var)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (array->count >= array->capacity)
    {
        uint32_t new_capacity = (uint32_t)(array->capacity * GROWTH_FACTOR);
        if (new_capacity <= array->capacity)
        {
            new_capacity = array->capacity + INITIAL_CAPACITY;
        }

        VariableInfo *new_variables = (VariableInfo *)realloc(array->variables, sizeof(VariableInfo) * new_capacity);
        if (!new_variables)
        {
            LOG_ERROR("Failed to grow variable array");
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

    array->capacity = initial_capacity;
    array->count = 0;
    array->files = (FileInfo *)malloc(sizeof(FileInfo) * array->capacity);

    if (!array->files)
    {
        LOG_ERROR("Failed to allocate memory for file array");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    LOG_DEBUG("File array initialized with capacity %u", initial_capacity);
    return CQ_SUCCESS;
}

void file_array_destroy(FileArray *array)
{
    if (!array)
    {
        return;
    }

    free(array->files);
    memset(array, 0, sizeof(FileArray));
    LOG_DEBUG("File array destroyed");
}

CQError file_array_add(FileArray *array, const FileInfo *file)
{
    if (!array || !file)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (array->count >= array->capacity)
    {
        uint32_t new_capacity = (uint32_t)(array->capacity * GROWTH_FACTOR);
        if (new_capacity <= array->capacity)
        {
            new_capacity = array->capacity + INITIAL_CAPACITY;
        }

        FileInfo *new_files = (FileInfo *)realloc(array->files, sizeof(FileInfo) * new_capacity);
        if (!new_files)
        {
            LOG_ERROR("Failed to grow file array");
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
    if (!project || !root_path)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    memset(project, 0, sizeof(Project));

    // Initialize string pool
    CQError err = string_pool_init(&project->string_pool, initial_capacity);
    if (err != CQ_SUCCESS)
    {
        return err;
    }

    // Intern root path
    project->root_path_id = string_pool_intern(&project->string_pool, root_path);
    if (project->root_path_id == UINT32_MAX)
    {
        string_pool_destroy(&project->string_pool);
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    // Initialize arrays
    err = file_array_init(&project->files, initial_capacity);
    if (err != CQ_SUCCESS)
    {
        string_pool_destroy(&project->string_pool);
        return err;
    }

    err = function_array_init(&project->functions, initial_capacity * 4); // More functions than files
    if (err != CQ_SUCCESS)
    {
        file_array_destroy(&project->files);
        string_pool_destroy(&project->string_pool);
        return err;
    }

    err = class_array_init(&project->classes, initial_capacity);
    if (err != CQ_SUCCESS)
    {
        function_array_destroy(&project->functions);
        file_array_destroy(&project->files);
        string_pool_destroy(&project->string_pool);
        return err;
    }

    err = variable_array_init(&project->variables, initial_capacity * 8); // Many variables
    if (err != CQ_SUCCESS)
    {
        class_array_destroy(&project->classes);
        function_array_destroy(&project->functions);
        file_array_destroy(&project->files);
        string_pool_destroy(&project->string_pool);
        return err;
    }

    // Initialize symbol table
    err = symbol_table_init(&project->symbol_table, initial_capacity * 4);
    if (err != CQ_SUCCESS)
    {
        variable_array_destroy(&project->variables);
        class_array_destroy(&project->classes);
        function_array_destroy(&project->functions);
        file_array_destroy(&project->files);
        string_pool_destroy(&project->string_pool);
        return err;
    }

    // Initialize dependency graph
    project->dependency_graph = (DependencyGraph *)malloc(sizeof(DependencyGraph));
    if (!project->dependency_graph)
    {
        LOG_ERROR("Failed to allocate memory for dependency graph");
        symbol_table_destroy(&project->symbol_table);
        variable_array_destroy(&project->variables);
        class_array_destroy(&project->classes);
        function_array_destroy(&project->functions);
        file_array_destroy(&project->files);
        string_pool_destroy(&project->string_pool);
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    err = dependency_graph_init(project->dependency_graph, initial_capacity * 4);
    if (err != CQ_SUCCESS)
    {
        free(project->dependency_graph);
        project->dependency_graph = NULL;
        symbol_table_destroy(&project->symbol_table);
        variable_array_destroy(&project->variables);
        class_array_destroy(&project->classes);
        function_array_destroy(&project->functions);
        file_array_destroy(&project->files);
        string_pool_destroy(&project->string_pool);
        return err;
    }

    LOG_INFO("Project initialized for root path: %s", root_path);
    return CQ_SUCCESS;
}

void project_destroy(Project *project)
{
    if (!project)
    {
        return;
    }

    if (project->dependency_graph)
    {
        dependency_graph_destroy(project->dependency_graph);
        free(project->dependency_graph);
        project->dependency_graph = NULL;
    }

    symbol_table_destroy(&project->symbol_table);
    variable_array_destroy(&project->variables);
    class_array_destroy(&project->classes);
    function_array_destroy(&project->functions);
    file_array_destroy(&project->files);
    string_pool_destroy(&project->string_pool);

    memset(project, 0, sizeof(Project));
    LOG_INFO("Project destroyed");
}

CQError project_add_file(Project *project, const char *filepath, SupportedLanguage language, FileInfo **file_info)
{
    if (!project || !filepath)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Intern filepath
    uint32_t filepath_id = string_pool_intern(&project->string_pool, filepath);
    if (filepath_id == UINT32_MAX)
    {
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    // Create file info
    FileInfo file = {
        .filepath_id = filepath_id,
        .language = language,
        .total_lines = 0,
        .code_lines = 0,
        .comment_lines = 0,
        .blank_lines = 0,
        .function_start = project->functions.count,
        .function_count = 0,
        .class_start = project->classes.count,
        .class_count = 0,
        .variable_start = project->variables.count,
        .variable_count = 0
    };

    CQError err = file_array_add(&project->files, &file);
    if (err != CQ_SUCCESS)
    {
        return err;
    }

    if (file_info)
    {
        *file_info = file_array_get(&project->files, project->files.count - 1);
    }

    LOG_DEBUG("Added file to project: %s", filepath);
    return CQ_SUCCESS;
}

CQError project_add_function(Project *project, const FunctionInfo *func, uint32_t *func_id)
{
    if (!project || !func)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    CQError err = function_array_add(&project->functions, func);
    if (err != CQ_SUCCESS)
    {
        return err;
    }

    uint32_t id = project->functions.count - 1;
    if (func_id)
    {
        *func_id = id;
    }

    // Add to symbol table
    err = symbol_table_add(&project->symbol_table, func->name_id, func->location.file_id);
    if (err != CQ_SUCCESS)
    {
        LOG_WARNING("Failed to add function to symbol table");
    }

    project->total_functions++;
    return CQ_SUCCESS;
}

CQError project_add_class(Project *project, const ClassInfo *cls, uint32_t *class_id)
{
    if (!project || !cls)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    CQError err = class_array_add(&project->classes, cls);
    if (err != CQ_SUCCESS)
    {
        return err;
    }

    uint32_t id = project->classes.count - 1;
    if (class_id)
    {
        *class_id = id;
    }

    // Add to symbol table
    err = symbol_table_add(&project->symbol_table, cls->name_id, cls->file_id);
    if (err != CQ_SUCCESS)
    {
        LOG_WARNING("Failed to add class to symbol table");
    }

    project->total_classes++;
    return CQ_SUCCESS;
}

CQError project_add_variable(Project *project, const VariableInfo *var, uint32_t *var_id)
{
    if (!project || !var)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    CQError err = variable_array_add(&project->variables, var);
    if (err != CQ_SUCCESS)
    {
        return err;
    }

    uint32_t id = project->variables.count - 1;
    if (var_id)
    {
        *var_id = id;
    }

    project->total_variables++;
    return CQ_SUCCESS;
}
// Data validation and integrity checking functions
bool ast_data_validate(const ASTData *data)
{
    if (!data)
    {
        LOG_ERROR("AST data is NULL");
        return false;
    }

    if (!data->project)
    {
        LOG_ERROR("AST data project is NULL");
        return false;
    }

    return project_validate(data->project);
}

bool project_validate(const Project *project)
{
    if (!project)
    {
        LOG_ERROR("Project is NULL");
        return false;
    }

    // Validate string pool
    if (!project->string_pool.strings || !project->string_pool.hashes ||
        !project->string_pool.hash_table)
    {
        LOG_ERROR("Project string pool is corrupted");
        return false;
    }

    if (project->string_pool.count > project->string_pool.capacity)
    {
        LOG_ERROR("Project string pool count exceeds capacity");
        return false;
    }

    // Validate arrays
    if (!project->files.files || !project->functions.functions ||
        !project->classes.classes || !project->variables.variables)
    {
        LOG_ERROR("Project arrays are corrupted");
        return false;
    }

    if (project->files.count > project->files.capacity ||
        project->functions.count > project->functions.capacity ||
        project->classes.count > project->classes.capacity ||
        project->variables.count > project->variables.capacity)
    {
        LOG_ERROR("Project array counts exceed capacities");
        return false;
    }

    // Validate symbol table
    if (!project->symbol_table.symbol_ids || !project->symbol_table.file_indices ||
        !project->symbol_table.hash_table)
    {
        LOG_ERROR("Project symbol table is corrupted");
        return false;
    }

    if (project->symbol_table.count > project->symbol_table.capacity)
    {
        LOG_ERROR("Project symbol table count exceeds capacity");
        return false;
    }

    // Validate dependency graph
    if (!project->dependency_graph)
    {
        LOG_ERROR("Project dependency graph is NULL");
        return false;
    }

    if (!dependency_graph_validate(project->dependency_graph))
    {
        LOG_ERROR("Project dependency graph validation failed");
        return false;
    }

    // Validate individual elements
    for (uint32_t i = 0; i < project->files.count; i++)
    {
        if (!file_info_validate(&project->files.files[i], project))
        {
            LOG_ERROR("File %u validation failed", i);
            return false;
        }
    }

    for (uint32_t i = 0; i < project->functions.count; i++)
    {
        if (!function_info_validate(&project->functions.functions[i], project))
        {
            LOG_ERROR("Function %u validation failed", i);
            return false;
        }
    }

    for (uint32_t i = 0; i < project->classes.count; i++)
    {
        if (!class_info_validate(&project->classes.classes[i], project))
        {
            LOG_ERROR("Class %u validation failed", i);
            return false;
        }
    }

    for (uint32_t i = 0; i < project->variables.count; i++)
    {
        if (!variable_info_validate(&project->variables.variables[i], project))
        {
            LOG_ERROR("Variable %u validation failed", i);
            return false;
        }
    }

    // Validate totals
    if (project->total_functions != project->functions.count ||
        project->total_classes != project->classes.count ||
        project->total_variables != project->variables.count)
    {
        LOG_ERROR("Project totals don't match actual counts");
        return false;
    }

    return true;
}

bool function_info_validate(const FunctionInfo *func, const Project *project)
{
    if (!func || !project)
    {
        return false;
    }

    // Validate string IDs
    if (func->name_id >= project->string_pool.count ||
        func->return_type_id >= project->string_pool.count)
    {
        LOG_ERROR("Function has invalid string ID");
        return false;
    }

    // Validate location
    if (func->location.file_id >= project->files.count)
    {
        LOG_ERROR("Function has invalid file ID");
        return false;
    }

    // Validate class ID if it's a method
    if (func->class_id != UINT32_MAX && func->class_id >= project->classes.count)
    {
        LOG_ERROR("Function has invalid class ID");
        return false;
    }

    return true;
}

bool class_info_validate(const ClassInfo *cls, const Project *project)
{
    if (!cls || !project)
    {
        return false;
    }

    // Validate string ID
    if (cls->name_id >= project->string_pool.count)
    {
        LOG_ERROR("Class has invalid string ID");
        return false;
    }

    // Validate file ID
    if (cls->file_id >= project->files.count)
    {
        LOG_ERROR("Class has invalid file ID");
        return false;
    }

    // Validate method indices
    if (cls->method_count > 0 && !cls->method_indices)
    {
        LOG_ERROR("Class has method count but no method indices");
        return false;
    }

    for (uint32_t i = 0; i < cls->method_count; i++)
    {
        if (cls->method_indices[i] >= project->functions.count)
        {
            LOG_ERROR("Class has invalid method index %u", cls->method_indices[i]);
            return false;
        }
    }

    return true;
}

bool variable_info_validate(const VariableInfo *var, const Project *project)
{
    if (!var || !project)
    {
        return false;
    }

    // Validate string IDs
    if (var->name_id >= project->string_pool.count ||
        var->type_id >= project->string_pool.count)
    {
        LOG_ERROR("Variable has invalid string ID");
        return false;
    }

    // Validate location
    if (var->location.file_id >= project->files.count)
    {
        LOG_ERROR("Variable has invalid file ID");
        return false;
    }

    // Validate scope ID
    if (var->scope_id != UINT32_MAX)
    {
        // Could be function or class scope
        bool valid_scope = (var->scope_id < project->functions.count) ||
                          (var->scope_id < project->classes.count);
        if (!valid_scope)
        {
            LOG_ERROR("Variable has invalid scope ID");
            return false;
        }
    }

    return true;
}

bool file_info_validate(const FileInfo *file, const Project *project)
{
    if (!file || !project)
    {
        return false;
    }

    // Validate filepath ID
    if (file->filepath_id >= project->string_pool.count)
    {
        LOG_ERROR("File has invalid filepath ID");
        return false;
    }

    // Validate array ranges
    if (file->function_start + file->function_count > project->functions.count ||
        file->class_start + file->class_count > project->classes.count ||
        file->variable_start + file->variable_count > project->variables.count)
    {
        LOG_ERROR("File has invalid array ranges");
        return false;
    }

    // Validate line counts
    if (file->code_lines + file->comment_lines + file->blank_lines > file->total_lines)
    {
        LOG_ERROR("File line counts are inconsistent");
        return false;
    }

    return true;
}
