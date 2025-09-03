#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "data/dependency_graph.h"
#include "data/ast_types.h"
#include "utils/logger.h"
#include "utils/memory.h"

// Internal helper functions
static DependencyNode *create_dependency_node(uint32_t id, uint32_t name_id,
                                            uint32_t file_id, DependencyType type, void *data);
static void destroy_dependency_node(DependencyNode *node);
static TreeNode *create_tree_node(uint32_t id, uint32_t name_id, uint32_t file_id,
                                DependencyType type, void *data);
static void destroy_tree_node(TreeNode *node);
static CallEdge *create_call_edge(uint32_t caller_id, uint32_t callee_id);
static void destroy_call_edge(CallEdge *edge);

// Dependency List Implementation
CQError dependency_list_init(DependencyList *list)
{
    if (!list)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;

    return CQ_SUCCESS;
}

void dependency_list_destroy(DependencyList *list)
{
    if (!list)
    {
        return;
    }

    DependencyNode *current = list->head;
    while (current)
    {
        DependencyNode *next = current->next;
        destroy_dependency_node(current);
        current = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

CQError dependency_list_add(DependencyList *list, uint32_t id, uint32_t name_id,
                           uint32_t file_id, DependencyType type, void *data)
{
    if (!list)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Check if node already exists
    if (dependency_list_find(list, id))
    {
        return CQ_ERROR_INVALID_ARGUMENT; // ID already exists
    }

    DependencyNode *node = create_dependency_node(id, name_id, file_id, type, data);
    if (!node)
    {
        LOG_ERROR("Failed to create dependency node");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    if (!list->head)
    {
        list->head = node;
        list->tail = node;
    }
    else
    {
        list->tail->next = node;
        list->tail = node;
    }

    list->count++;
    return CQ_SUCCESS;
}

DependencyNode *dependency_list_find(const DependencyList *list, uint32_t id)
{
    if (!list)
    {
        return NULL;
    }

    DependencyNode *current = list->head;
    while (current)
    {
        if (current->id == id)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

CQError dependency_list_remove(DependencyList *list, uint32_t id)
{
    if (!list)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    DependencyNode *current = list->head;
    DependencyNode *prev = NULL;

    while (current)
    {
        if (current->id == id)
        {
            if (prev)
            {
                prev->next = current->next;
            }
            else
            {
                list->head = current->next;
            }

            if (current == list->tail)
            {
                list->tail = prev;
            }

            destroy_dependency_node(current);
            list->count--;
            return CQ_SUCCESS;
        }

        prev = current;
        current = current->next;
    }

    return CQ_ERROR_INVALID_ARGUMENT; // ID not found
}

// Tree Implementation
CQError dependency_tree_init(DependencyTree *tree)
{
    if (!tree)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    tree->root = NULL;
    tree->node_count = 0;

    return CQ_SUCCESS;
}

void dependency_tree_destroy(DependencyTree *tree)
{
    if (!tree || !tree->root)
    {
        return;
    }

    // Recursive destruction of tree nodes
    // This is a simple implementation - in production, consider iterative approach
    destroy_tree_node(tree->root);
    tree->root = NULL;
    tree->node_count = 0;
}

CQError dependency_tree_add_node(DependencyTree *tree, uint32_t id, uint32_t name_id,
                                uint32_t file_id, DependencyType type, void *data,
                                uint32_t parent_id)
{
    if (!tree)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    TreeNode *node = create_tree_node(id, name_id, file_id, type, data);
    if (!node)
    {
        LOG_ERROR("Failed to create tree node");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    if (parent_id == 0) // Root node
    {
        if (tree->root)
        {
            destroy_tree_node(node);
            return CQ_ERROR_INVALID_ARGUMENT; // Root already exists
        }
        tree->root = node;
    }
    else
    {
        TreeNode *parent = dependency_tree_find_node(tree, parent_id);
        if (!parent)
        {
            destroy_tree_node(node);
            return CQ_ERROR_INVALID_ARGUMENT; // Parent not found
        }

        node->parent = parent;

        if (!parent->first_child)
        {
            parent->first_child = node;
        }
        else
        {
            TreeNode *sibling = parent->first_child;
            while (sibling->next_sibling)
            {
                sibling = sibling->next_sibling;
            }
            sibling->next_sibling = node;
            node->prev_sibling = sibling;
        }

        parent->child_count++;
    }

    tree->node_count++;
    return CQ_SUCCESS;
}

TreeNode *dependency_tree_find_node(const DependencyTree *tree, uint32_t id)
{
    if (!tree || !tree->root)
    {
        return NULL;
    }

    // Simple recursive search - for large trees, consider iterative or hash-based approach
    return find_node_recursive(tree->root, id);
}

static TreeNode *find_node_recursive(TreeNode *node, uint32_t id)
{
    if (!node)
    {
        return NULL;
    }

    if (node->id == id)
    {
        return node;
    }

    TreeNode *found = find_node_recursive(node->first_child, id);
    if (found)
    {
        return found;
    }

    return find_node_recursive(node->next_sibling, id);
}

CQError dependency_tree_remove_node(DependencyTree *tree, uint32_t id)
{
    if (!tree)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    TreeNode *node = dependency_tree_find_node(tree, id);
    if (!node)
    {
        return CQ_ERROR_INVALID_ARGUMENT; // Node not found
    }

    // Cannot remove root if it has children
    if (node == tree->root && node->first_child)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Remove from parent's children list
    if (node->parent)
    {
        if (node->parent->first_child == node)
        {
            node->parent->first_child = node->next_sibling;
        }

        if (node->next_sibling)
        {
            node->next_sibling->prev_sibling = node->prev_sibling;
        }

        if (node->prev_sibling)
        {
            node->prev_sibling->next_sibling = node->next_sibling;
        }

        node->parent->child_count--;
    }
    else
    {
        tree->root = NULL;
    }

    // Recursively destroy subtree
    destroy_tree_node(node);
    tree->node_count--;

    return CQ_SUCCESS;
}

CQError dependency_tree_get_children(const DependencyTree *tree, uint32_t parent_id,
                                   TreeNode ***children, uint32_t *count)
{
    if (!tree || !children || !count)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    TreeNode *parent = (parent_id == 0) ? tree->root : dependency_tree_find_node(tree, parent_id);
    if (!parent)
    {
        *children = NULL;
        *count = 0;
        return CQ_SUCCESS;
    }

    *count = parent->child_count;
    if (*count == 0)
    {
        *children = NULL;
        return CQ_SUCCESS;
    }

    *children = (TreeNode **)malloc(sizeof(TreeNode *) * (*count));
    if (!(*children))
    {
        LOG_ERROR("Failed to allocate memory for children array");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    TreeNode *child = parent->first_child;
    uint32_t i = 0;
    while (child && i < *count)
    {
        (*children)[i++] = child;
        child = child->next_sibling;
    }

    return CQ_SUCCESS;
}

// Call Graph Implementation
CQError call_graph_init(CallGraph *graph, uint32_t initial_node_count)
{
    if (!graph || initial_node_count == 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    graph->edges = (CallEdge **)calloc(initial_node_count, sizeof(CallEdge *));
    if (!graph->edges)
    {
        LOG_ERROR("Failed to allocate memory for call graph edges");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    graph->node_count = initial_node_count;
    graph->edge_count = 0;

    return CQ_SUCCESS;
}

void call_graph_destroy(CallGraph *graph)
{
    if (!graph || !graph->edges)
    {
        return;
    }

    for (uint32_t i = 0; i < graph->node_count; i++)
    {
        CallEdge *edge = graph->edges[i];
        while (edge)
        {
            CallEdge *next = edge->next;
            destroy_call_edge(edge);
            edge = next;
        }
        graph->edges[i] = NULL;
    }

    free(graph->edges);
    graph->edges = NULL;
    graph->node_count = 0;
    graph->edge_count = 0;
}

CQError call_graph_add_edge(CallGraph *graph, uint32_t caller_id, uint32_t callee_id)
{
    if (!graph || caller_id >= graph->node_count || callee_id >= graph->node_count)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Check if edge already exists
    CallEdge *edge = graph->edges[caller_id];
    while (edge)
    {
        if (edge->callee_id == callee_id)
        {
            edge->call_count++;
            return CQ_SUCCESS;
        }
        edge = edge->next;
    }

    // Create new edge
    CallEdge *new_edge = create_call_edge(caller_id, callee_id);
    if (!new_edge)
    {
        LOG_ERROR("Failed to create call edge");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    new_edge->next = graph->edges[caller_id];
    graph->edges[caller_id] = new_edge;
    graph->edge_count++;

    return CQ_SUCCESS;
}

CQError call_graph_remove_edge(CallGraph *graph, uint32_t caller_id, uint32_t callee_id)
{
    if (!graph || caller_id >= graph->node_count)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    CallEdge *edge = graph->edges[caller_id];
    CallEdge *prev = NULL;

    while (edge)
    {
        if (edge->callee_id == callee_id)
        {
            if (prev)
            {
                prev->next = edge->next;
            }
            else
            {
                graph->edges[caller_id] = edge->next;
            }

            destroy_call_edge(edge);
            graph->edge_count--;
            return CQ_SUCCESS;
        }

        prev = edge;
        edge = edge->next;
    }

    return CQ_ERROR_INVALID_ARGUMENT; // Edge not found
}

uint32_t call_graph_get_call_count(const CallGraph *graph, uint32_t caller_id, uint32_t callee_id)
{
    if (!graph || caller_id >= graph->node_count)
    {
        return 0;
    }

    CallEdge *edge = graph->edges[caller_id];
    while (edge)
    {
        if (edge->callee_id == callee_id)
        {
            return edge->call_count;
        }
        edge = edge->next;
    }

    return 0;
}

CQError call_graph_get_callees(const CallGraph *graph, uint32_t caller_id,
                              uint32_t **callees, uint32_t *count)
{
    if (!graph || caller_id >= graph->node_count || !callees || !count)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Count callees first
    uint32_t callee_count = 0;
    CallEdge *edge = graph->edges[caller_id];
    while (edge)
    {
        callee_count++;
        edge = edge->next;
    }

    *count = callee_count;
    if (callee_count == 0)
    {
        *callees = NULL;
        return CQ_SUCCESS;
    }

    *callees = (uint32_t *)malloc(sizeof(uint32_t) * callee_count);
    if (!(*callees))
    {
        LOG_ERROR("Failed to allocate memory for callees array");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    edge = graph->edges[caller_id];
    uint32_t i = 0;
    while (edge && i < callee_count)
    {
        (*callees)[i++] = edge->callee_id;
        edge = edge->next;
    }

    return CQ_SUCCESS;
}

CQError call_graph_get_callers(const CallGraph *graph, uint32_t callee_id,
                             uint32_t **callers, uint32_t *count)
{
    if (!graph || !callers || !count)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Count callers first
    uint32_t caller_count = 0;
    for (uint32_t i = 0; i < graph->node_count; i++)
    {
        CallEdge *edge = graph->edges[i];
        while (edge)
        {
            if (edge->callee_id == callee_id)
            {
                caller_count++;
                break;
            }
            edge = edge->next;
        }
    }

    *count = caller_count;
    if (caller_count == 0)
    {
        *callers = NULL;
        return CQ_SUCCESS;
    }

    *callers = (uint32_t *)malloc(sizeof(uint32_t) * caller_count);
    if (!(*callers))
    {
        LOG_ERROR("Failed to allocate memory for callers array");
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    uint32_t i = 0;
    for (uint32_t j = 0; j < graph->node_count && i < caller_count; j++)
    {
        CallEdge *edge = graph->edges[j];
        while (edge && i < caller_count)
        {
            if (edge->callee_id == callee_id)
            {
                (*callers)[i++] = j;
                break;
            }
            edge = edge->next;
        }
    }

    return CQ_SUCCESS;
}

// Dependency Graph Implementation
CQError dependency_graph_init(DependencyGraph *graph, uint32_t initial_node_count)
{
    if (!graph)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    CQError err;

    err = dependency_list_init(&graph->include_deps);
    if (err != CQ_SUCCESS) return err;

    err = dependency_list_init(&graph->function_deps);
    if (err != CQ_SUCCESS) return err;

    err = dependency_list_init(&graph->type_deps);
    if (err != CQ_SUCCESS) return err;

    err = dependency_tree_init(&graph->hierarchy);
    if (err != CQ_SUCCESS) return err;

    err = call_graph_init(&graph->call_graph, initial_node_count);
    if (err != CQ_SUCCESS) return err;

    return CQ_SUCCESS;
}

void dependency_graph_destroy(DependencyGraph *graph)
{
    if (!graph)
    {
        return;
    }

    dependency_list_destroy(&graph->include_deps);
    dependency_list_destroy(&graph->function_deps);
    dependency_list_destroy(&graph->type_deps);
    dependency_tree_destroy(&graph->hierarchy);
    call_graph_destroy(&graph->call_graph);
}

// Helper functions implementation
static DependencyNode *create_dependency_node(uint32_t id, uint32_t name_id,
                                            uint32_t file_id, DependencyType type, void *data)
{
    DependencyNode *node = (DependencyNode *)malloc(sizeof(DependencyNode));
    if (!node)
    {
        return NULL;
    }

    node->id = id;
    node->name_id = name_id;
    node->file_id = file_id;
    node->type = type;
    node->data = data;
    node->next = NULL;

    return node;
}

static void destroy_dependency_node(DependencyNode *node)
{
    if (!node)
    {
        return;
    }

    // Note: data is owned by caller, don't free here
    free(node);
}

static TreeNode *create_tree_node(uint32_t id, uint32_t name_id, uint32_t file_id,
                                DependencyType type, void *data)
{
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    if (!node)
    {
        return NULL;
    }

    node->id = id;
    node->name_id = name_id;
    node->file_id = file_id;
    node->type = type;
    node->data = data;
    node->parent = NULL;
    node->first_child = NULL;
    node->next_sibling = NULL;
    node->prev_sibling = NULL;
    node->child_count = 0;

    return node;
}

static void destroy_tree_node(TreeNode *node)
{
    if (!node)
    {
        return;
    }

    // Recursively destroy children
    TreeNode *child = node->first_child;
    while (child)
    {
        TreeNode *next = child->next_sibling;
        destroy_tree_node(child);
        child = next;
    }

    // Note: data is owned by caller, don't free here
    free(node);
}

static CallEdge *create_call_edge(uint32_t caller_id, uint32_t callee_id)
{
    CallEdge *edge = (CallEdge *)malloc(sizeof(CallEdge));
    if (!edge)
    {
        return NULL;
    }

    edge->caller_id = caller_id;
    edge->callee_id = callee_id;
    edge->call_count = 1;
    edge->next = NULL;

    return edge;
}

static void destroy_call_edge(CallEdge *edge)
{
    if (!edge)
    {
        return;
    }

    free(edge);
}

// Placeholder implementations for remaining functions
CQError dependency_graph_add_include_dep(DependencyGraph *graph, uint32_t depender_id,
                                       uint32_t dependee_id, uint32_t name_id, uint32_t file_id)
{
    // Implementation would add to include_deps list
    return CQ_SUCCESS;
}

CQError dependency_graph_add_function_dep(DependencyGraph *graph, uint32_t caller_id,
                                        uint32_t callee_id, uint32_t name_id, uint32_t file_id)
{
    // Implementation would add to function_deps list and call_graph
    return CQ_SUCCESS;
}

CQError dependency_graph_add_type_dep(DependencyGraph *graph, uint32_t user_id,
                                    uint32_t type_id, uint32_t name_id, uint32_t file_id)
{
    // Implementation would add to type_deps list
    return CQ_SUCCESS;
}

CQError dependency_graph_build_hierarchy(DependencyGraph *graph, const Project *project)
{
    // Implementation would build tree from project structure
    return CQ_SUCCESS;
}

CQError dependency_graph_detect_cycles(const DependencyGraph *graph, uint32_t **cycles,
                                     uint32_t *cycle_count)
{
    // Implementation would detect cycles in dependency graph
    *cycles = NULL;
    *cycle_count = 0;
    return CQ_SUCCESS;
}

CQError dependency_graph_get_transitive_deps(const DependencyGraph *graph, uint32_t node_id,
                                           uint32_t **deps, uint32_t *count)
{
    // Implementation would find transitive dependencies
    *deps = NULL;
    *count = 0;
    return CQ_SUCCESS;
}

CQError dependency_graph_calculate_depth(const DependencyGraph *graph, uint32_t node_id,
                                       uint32_t *depth)
{
    // Implementation would calculate tree depth
    *depth = 0;
    return CQ_SUCCESS;
}

CQError dependency_graph_find_roots(const DependencyGraph *graph, uint32_t **roots,
                                  uint32_t *count)
{
    // Implementation would find root nodes
    *roots = NULL;
    *count = 0;
    return CQ_SUCCESS;
}

CQError dependency_graph_find_leaves(const DependencyGraph *graph, uint32_t **leaves,
                                   uint32_t *count)
{
    // Implementation would find leaf nodes
    *leaves = NULL;
    *count = 0;
    return CQ_SUCCESS;
}

bool dependency_graph_validate(const DependencyGraph *graph)
{
    if (!graph)
    {
        return false;
    }

    return dependency_list_validate(&graph->include_deps) &&
           dependency_list_validate(&graph->function_deps) &&
           dependency_list_validate(&graph->type_deps) &&
           dependency_tree_validate(&graph->hierarchy) &&
           call_graph_validate(&graph->call_graph);
}

bool dependency_list_validate(const DependencyList *list)
{
    if (!list)
    {
        return false;
    }

    uint32_t count = 0;
    DependencyNode *current = list->head;

    while (current)
    {
        count++;
        if (count > list->count)
        {
            return false; // Cycle detected
        }
        current = current->next;
    }

    return count == list->count;
}

bool dependency_tree_validate(const DependencyTree *tree)
{
    if (!tree)
    {
        return false;
    }

    if (tree->node_count == 0)
    {
        return tree->root == NULL;
    }

    // Basic validation - could be enhanced
    return tree->root != NULL;
}

bool call_graph_validate(const CallGraph *graph)
{
    if (!graph)
    {
        return false;
    }

    if (graph->node_count == 0)
    {
        return graph->edges == NULL;
    }

    return graph->edges != NULL;
}