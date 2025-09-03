#ifndef AST_TYPES_H
#define AST_TYPES_H

#include "cqanalyzer.h"
#include <stdint.h>
#include <stdbool.h>

// Forward declaration for DependencyGraph to avoid circular dependency
typedef struct DependencyGraph DependencyGraph;

/**
 * @file ast_types.h
 * @brief Efficient data structures for AST information
 *
 * Defines optimized data structures for storing parsed AST information
 * with improved memory efficiency and access patterns for large codebases.
 */

// Forward declarations
typedef struct StringPool StringPool;
typedef struct SymbolTable SymbolTable;
typedef struct FunctionArray FunctionArray;
typedef struct ClassArray ClassArray;
typedef struct VariableArray VariableArray;
typedef struct FileArray FileArray;

/**
 * @brief String interning pool for memory-efficient string storage
 */
struct StringPool
{
    char **strings;           // Array of interned strings
    uint32_t *hashes;         // Hash values for fast lookup
    uint32_t count;           // Number of strings
    uint32_t capacity;        // Allocated capacity
    uint32_t *hash_table;     // Hash table for string lookup
    uint32_t hash_table_size; // Size of hash table
};

/**
 * @brief Symbol table for fast symbol lookup
 */
struct SymbolTable
{
    uint32_t *symbol_ids;     // Array of symbol IDs
    uint32_t *file_indices;   // Corresponding file indices
    uint32_t count;           // Number of symbols
    uint32_t capacity;        // Allocated capacity
    uint32_t *hash_table;     // Hash table for symbol lookup
    uint32_t hash_table_size; // Size of hash table
};

/**
 * @brief Location information in source code (compact)
 */
typedef struct
{
    uint32_t file_id;         // Index into file array (instead of full path)
    uint32_t line;
    uint32_t column;
    uint32_t offset;
} SourceLocation;

/**
 * @brief Function information (optimized structure)
 */
typedef struct
{
    uint32_t name_id;         // Interned string ID
    SourceLocation location;
    uint32_t complexity;      // Cyclomatic complexity
    uint32_t nesting_depth;   // Maximum nesting depth
    uint32_t lines_of_code;   // Physical lines
    uint32_t parameter_count;
    uint32_t return_type_id;  // Interned string ID
    uint32_t usage_count;     // Number of times called
    uint32_t class_id;        // Parent class ID (if method)
} FunctionInfo;

/**
 * @brief Dynamic array of functions
 */
struct FunctionArray
{
    FunctionInfo *functions;
    uint32_t count;
    uint32_t capacity;
};

/**
 * @brief Class/struct information (optimized)
 */
typedef struct
{
    uint32_t name_id;         // Interned string ID
    SourceLocation location;
    uint32_t method_count;
    uint32_t field_count;
    uint32_t *method_indices; // Array of function indices
    uint32_t file_id;         // Parent file ID
} ClassInfo;

/**
 * @brief Dynamic array of classes
 */
struct ClassArray
{
    ClassInfo *classes;
    uint32_t count;
    uint32_t capacity;
};

/**
 * @brief Variable/Field information (optimized)
 */
typedef struct
{
    uint32_t name_id;         // Interned string ID
    uint32_t type_id;         // Interned string ID
    SourceLocation location;
    uint32_t usage_count;     // Number of times used
    uint32_t scope_id;        // Function/class scope ID
    bool is_global;           // Global variable flag
} VariableInfo;

/**
 * @brief Dynamic array of variables
 */
struct VariableArray
{
    VariableInfo *variables;
    uint32_t count;
    uint32_t capacity;
};

/**
 * @brief File information (optimized)
 */
typedef struct
{
    uint32_t filepath_id;     // Interned string ID
    SupportedLanguage language;
    uint32_t total_lines;
    uint32_t code_lines;
    uint32_t comment_lines;
    uint32_t blank_lines;
    uint32_t function_start;  // Start index in global function array
    uint32_t function_count;
    uint32_t class_start;     // Start index in global class array
    uint32_t class_count;
    uint32_t variable_start;  // Start index in global variable array
    uint32_t variable_count;
} FileInfo;

/**
 * @brief Dynamic array of files
 */
struct FileArray
{
    FileInfo *files;
    uint32_t count;
    uint32_t capacity;
};

/**
 * @brief Project-level information (optimized)
 */
typedef struct
{
    uint32_t root_path_id;    // Interned string ID
    FileArray files;
    FunctionArray functions;
    ClassArray classes;
    VariableArray variables;
    StringPool string_pool;
    SymbolTable symbol_table;
    DependencyGraph *dependency_graph; // Code dependency relationships
    uint32_t total_functions;
    uint32_t total_classes;
    uint32_t total_variables;
} Project;

/**
 * @brief AST data container (optimized)
 */
typedef struct
{
    Project *project;
    void *clang_index;        // CXIndex from libclang
    void *clang_translation_unit; // CXTranslationUnit
    bool owns_project;        // Whether this container owns the project memory
} ASTData;

// Function declarations for data structure management
CQError string_pool_init(StringPool *pool, uint32_t initial_capacity);
void string_pool_destroy(StringPool *pool);
uint32_t string_pool_intern(StringPool *pool, const char *str);
const char *string_pool_get(const StringPool *pool, uint32_t id);

CQError symbol_table_init(SymbolTable *table, uint32_t initial_capacity);
void symbol_table_destroy(SymbolTable *table);
CQError symbol_table_add(SymbolTable *table, uint32_t symbol_id, uint32_t file_index);
uint32_t symbol_table_find(const SymbolTable *table, uint32_t symbol_id);

CQError function_array_init(FunctionArray *array, uint32_t initial_capacity);
void function_array_destroy(FunctionArray *array);
CQError function_array_add(FunctionArray *array, const FunctionInfo *func);
FunctionInfo *function_array_get(FunctionArray *array, uint32_t index);

CQError class_array_init(ClassArray *array, uint32_t initial_capacity);
void class_array_destroy(ClassArray *array);
CQError class_array_add(ClassArray *array, const ClassInfo *cls);
ClassInfo *class_array_get(ClassArray *array, uint32_t index);

CQError variable_array_init(VariableArray *array, uint32_t initial_capacity);
void variable_array_destroy(VariableArray *array);
CQError variable_array_add(VariableArray *array, const VariableInfo *var);
VariableInfo *variable_array_get(VariableArray *array, uint32_t index);

CQError file_array_init(FileArray *array, uint32_t initial_capacity);
void file_array_destroy(FileArray *array);
CQError file_array_add(FileArray *array, const FileInfo *file);
FileInfo *file_array_get(FileArray *array, uint32_t index);

CQError project_init(Project *project, const char *root_path, uint32_t initial_capacity);
void project_destroy(Project *project);
CQError project_add_file(Project *project, const char *filepath, SupportedLanguage language, FileInfo **file_info);
CQError project_add_function(Project *project, const FunctionInfo *func, uint32_t *func_id);
CQError project_add_class(Project *project, const ClassInfo *cls, uint32_t *class_id);
CQError project_add_variable(Project *project, const VariableInfo *var, uint32_t *var_id);

// Data validation and integrity functions
bool ast_data_validate(const ASTData *data);
bool project_validate(const Project *project);
bool function_info_validate(const FunctionInfo *func, const Project *project);
bool class_info_validate(const ClassInfo *cls, const Project *project);
bool variable_info_validate(const VariableInfo *var, const Project *project);
bool file_info_validate(const FileInfo *file, const Project *project);

#endif // AST_TYPES_H