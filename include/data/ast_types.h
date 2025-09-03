#ifndef AST_TYPES_H
#define AST_TYPES_H

#include "cqanalyzer.h"
#include <stdint.h>

/**
 * @file ast_types.h
 * @brief Data structures for AST information
 *
 * Defines the core data structures used to store parsed AST information
 * from source code files.
 */

/**
 * @brief Location information in source code
 */
typedef struct
{
    char filepath[MAX_PATH_LENGTH];
    uint32_t line;
    uint32_t column;
    uint32_t offset;
} SourceLocation;

/**
 * @brief Function information extracted from AST
 */
struct FunctionInfo
{
    char name[MAX_NAME_LENGTH];
    SourceLocation location;
    uint32_t complexity;        // Cyclomatic complexity
    uint32_t lines_of_code;     // Physical lines
    uint32_t parameter_count;
    char return_type[MAX_NAME_LENGTH];
    struct FunctionInfo *next;  // For linked list
};

/**
 * @brief Class/struct information extracted from AST
 */
struct ClassInfo
{
    char name[MAX_NAME_LENGTH];
    SourceLocation location;
    uint32_t method_count;
    uint32_t field_count;
    struct FunctionInfo *methods;
    struct ClassInfo *next;     // For linked list
};

/**
 * @brief Variable/Field information
 */
typedef struct
{
    char name[MAX_NAME_LENGTH];
    char type[MAX_NAME_LENGTH];
    SourceLocation location;
} VariableInfo;

/**
 * @brief File-level information
 */
struct FileInfo
{
    char filepath[MAX_PATH_LENGTH];
    SupportedLanguage language;
    uint32_t total_lines;
    uint32_t code_lines;
    uint32_t comment_lines;
    uint32_t blank_lines;
    struct FunctionInfo *functions;
    struct ClassInfo *classes;
    uint32_t function_count;
    uint32_t class_count;
    struct FileInfo *next;      // For linked list
};

/**
 * @brief Project-level information
 */
struct Project
{
    char root_path[MAX_PATH_LENGTH];
    struct FileInfo *files;
    uint32_t file_count;
    uint32_t total_functions;
    uint32_t total_classes;
};

/**
 * @brief AST data container
 */
typedef struct
{
    struct Project *project;
    void *clang_index;      // CXIndex from libclang
    void *clang_translation_unit; // CXTranslationUnit
} ASTData;

#endif // AST_TYPES_H