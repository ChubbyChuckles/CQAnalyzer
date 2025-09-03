#ifndef DEPENDENCY_GRAPH_H
#define DEPENDENCY_GRAPH_H

#include "cqanalyzer.h"
#include <stdint.h>
#include <stdbool.h>

// Forward declaration for Project to avoid circular dependency
typedef struct Project Project;

/**
 * @file dependency_graph.h
 * @brief Data structures for representing code dependencies
 *
 * Provides linked lists and tree structures for modeling code dependencies,
 * function call relationships, and hierarchical code organization.
 */

// Forward declarations
typedef struct DependencyNode DependencyNode;
typedef struct DependencyList DependencyList;
typedef struct DependencyTree DependencyTree;
typedef struct CallGraph CallGraph;

/**
 * @brief Types of code dependencies
 */
typedef enum
{
    DEPENDENCY_INCLUDE,      // #include or import statements
    DEPENDENCY_FUNCTION_CALL, // Function/method calls
    DEPENDENCY_INHERITANCE,  // Class inheritance
    DEPENDENCY_COMPOSITION,  // Object composition
    DEPENDENCY_USAGE,        // Variable/constant usage
    DEPENDENCY_TYPE,         // Type dependencies
    DEPENDENCY_MODULE        // Module/package dependencies
} DependencyType;

/**
 * @brief Node in the dependency graph
 */
struct DependencyNode
{
    uint32_t id;                    // Unique identifier
    uint32_t name_id;               // Interned string ID for name
    uint32_t file_id;               // File containing this element
    DependencyType type;            // Type of code element
    void *data;                     // Additional data (function info, etc.)
    struct DependencyNode *next;    // Next in linked list
};

/**
 * @brief Linked list of dependencies
 */
struct DependencyList
{
    DependencyNode *head;
    DependencyNode *tail;
    uint32_t count;
};

/**
 * @brief Tree node for hierarchical relationships
 */
typedef struct TreeNode
{
    uint32_t id;                    // Unique identifier
    uint32_t name_id;               // Interned string ID for name
    uint32_t file_id;               // File containing this element
    DependencyType type;            // Type of code element
    void *data;                     // Additional data
    struct TreeNode *parent;        // Parent node
    struct TreeNode *first_child;   // First child
    struct TreeNode *next_sibling;  // Next sibling
    struct TreeNode *prev_sibling;  // Previous sibling
    uint32_t child_count;           // Number of children
} TreeNode;

/**
 * @brief Tree structure for hierarchical code organization
 */
struct DependencyTree
{
    TreeNode *root;
    uint32_t node_count;
};

/**
 * @brief Edge in the call graph
 */
typedef struct CallEdge
{
    uint32_t caller_id;             // Calling function ID
    uint32_t callee_id;             // Called function ID
    uint32_t call_count;            // Number of calls
    struct CallEdge *next;          // Next edge from caller
} CallEdge;

/**
 * @brief Call graph for function relationships
 */
struct CallGraph
{
    CallEdge **edges;               // Array of edge lists (indexed by caller ID)
    uint32_t node_count;            // Number of functions
    uint32_t edge_count;            // Total number of edges
};

/**
 * @brief Dependency graph combining all structures
 */
typedef struct
{
    DependencyList include_deps;     // Include/import dependencies
    DependencyList function_deps;    // Function call dependencies
    DependencyList type_deps;        // Type dependencies
    DependencyTree hierarchy;        // Hierarchical code structure
    CallGraph call_graph;           // Function call graph
} DependencyGraph;

// Function declarations for dependency management

// Dependency List operations
CQError dependency_list_init(DependencyList *list);
void dependency_list_destroy(DependencyList *list);
CQError dependency_list_add(DependencyList *list, uint32_t id, uint32_t name_id,
                           uint32_t file_id, DependencyType type, void *data);
DependencyNode *dependency_list_find(const DependencyList *list, uint32_t id);
CQError dependency_list_remove(DependencyList *list, uint32_t id);

// Tree operations
CQError dependency_tree_init(DependencyTree *tree);
void dependency_tree_destroy(DependencyTree *tree);
CQError dependency_tree_add_node(DependencyTree *tree, uint32_t id, uint32_t name_id,
                                uint32_t file_id, DependencyType type, void *data,
                                uint32_t parent_id);
TreeNode *dependency_tree_find_node(const DependencyTree *tree, uint32_t id);
CQError dependency_tree_remove_node(DependencyTree *tree, uint32_t id);
CQError dependency_tree_get_children(const DependencyTree *tree, uint32_t parent_id,
                                   TreeNode ***children, uint32_t *count);

// Call Graph operations
CQError call_graph_init(CallGraph *graph, uint32_t initial_node_count);
void call_graph_destroy(CallGraph *graph);
CQError call_graph_add_edge(CallGraph *graph, uint32_t caller_id, uint32_t callee_id);
CQError call_graph_remove_edge(CallGraph *graph, uint32_t caller_id, uint32_t callee_id);
uint32_t call_graph_get_call_count(const CallGraph *graph, uint32_t caller_id, uint32_t callee_id);
CQError call_graph_get_callees(const CallGraph *graph, uint32_t caller_id,
                              uint32_t **callees, uint32_t *count);
CQError call_graph_get_callers(const CallGraph *graph, uint32_t callee_id,
                             uint32_t **callers, uint32_t *count);

// Dependency Graph operations
CQError dependency_graph_init(DependencyGraph *graph, uint32_t initial_node_count);
void dependency_graph_destroy(DependencyGraph *graph);
CQError dependency_graph_add_include_dep(DependencyGraph *graph, uint32_t depender_id,
                                       uint32_t dependee_id, uint32_t name_id, uint32_t file_id);
CQError dependency_graph_add_function_dep(DependencyGraph *graph, uint32_t caller_id,
                                        uint32_t callee_id, uint32_t name_id, uint32_t file_id);
CQError dependency_graph_add_type_dep(DependencyGraph *graph, uint32_t user_id,
                                    uint32_t type_id, uint32_t name_id, uint32_t file_id);
CQError dependency_graph_build_hierarchy(DependencyGraph *graph, const Project *project);

// Analysis functions
CQError dependency_graph_detect_cycles(const DependencyGraph *graph, uint32_t **cycles,
                                     uint32_t *cycle_count);
CQError dependency_graph_get_transitive_deps(const DependencyGraph *graph, uint32_t node_id,
                                           uint32_t **deps, uint32_t *count);
CQError dependency_graph_calculate_depth(const DependencyGraph *graph, uint32_t node_id,
                                       uint32_t *depth);
CQError dependency_graph_find_roots(const DependencyGraph *graph, uint32_t **roots,
                                  uint32_t *count);
CQError dependency_graph_find_leaves(const DependencyGraph *graph, uint32_t **leaves,
                                   uint32_t *count);

// Validation functions
bool dependency_graph_validate(const DependencyGraph *graph);
bool dependency_list_validate(const DependencyList *list);
bool dependency_tree_validate(const DependencyTree *tree);
bool call_graph_validate(const CallGraph *graph);

#endif // DEPENDENCY_GRAPH_H