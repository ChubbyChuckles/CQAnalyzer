#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "visualizer/network_graph.h"
#include "visualizer/renderer.h"
#include "visualizer/color.h"
// Include dependency graph types but avoid conflicts
#include "data/dependency_graph.h"
#include "data/data_store.h"
#include "utils/logger.h"

#define MAX_NETWORK_NODES 1000
#define MAX_NETWORK_EDGES 5000
#define NODE_SPACING 2.0f
#define NODE_SIZE 0.1f
#define EDGE_THICKNESS 0.02f
#define FORCE_ITERATIONS 50
#define REPULSION_FORCE 1.0f
#define ATTRACTION_FORCE 0.1f
#define DAMPING 0.9f

typedef struct
{
    float x, y, z;
    float vx, vy, vz; // velocity for force-directed layout
    Color color;
    char label[256];
    uint32_t node_id;
    DependencyType type;
    char node_type[32];
} NetworkNode;

typedef struct
{
    int from_index;
    int to_index;
    Color color;
    float weight;
} NetworkEdge;

static NetworkNode network_nodes[MAX_NETWORK_NODES];
static NetworkEdge network_edges[MAX_NETWORK_EDGES];
static int num_network_nodes = 0;
static int num_network_edges = 0;

// Forward declarations
static void calculate_node_positions(const DependencyGraph *graph);
static void apply_forces(void);
static Color get_node_color(DependencyType type, const char *color_metric, uint32_t node_id);
static void add_edge(int from_index, int to_index, float weight);
static int find_node_index(uint32_t node_id);

CQError network_graph_create(const DependencyGraph *dependency_graph, const char *color_metric)
{
    if (!dependency_graph)
    {
        LOG_ERROR("Invalid dependency graph for network visualization");
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Clear previous data
    network_graph_clear();

    // Calculate positions for all nodes
    calculate_node_positions(dependency_graph);

    LOG_INFO("Created network graph with %d nodes and %d edges", num_network_nodes, num_network_edges);
    return CQ_SUCCESS;
}

static void calculate_node_positions(const DependencyGraph *graph)
{
    if (!graph)
    {
        return;
    }

    // First pass: collect all unique nodes from various sources
    uint32_t node_ids[MAX_NETWORK_NODES];
    int node_count = 0;

    // Add nodes from include dependencies
    DependencyNode *dep_node = graph->include_deps.head;
    while (dep_node && node_count < MAX_NETWORK_NODES)
    {
        if (find_node_index(dep_node->id) == -1)
        {
            node_ids[node_count++] = dep_node->id;
        }
        dep_node = dep_node->next;
    }

    // Add nodes from function dependencies
    dep_node = graph->function_deps.head;
    while (dep_node && node_count < MAX_NETWORK_NODES)
    {
        if (find_node_index(dep_node->id) == -1)
        {
            node_ids[node_count++] = dep_node->id;
        }
        dep_node = dep_node->next;
    }

    // Add nodes from type dependencies
    dep_node = graph->type_deps.head;
    while (dep_node && node_count < MAX_NETWORK_NODES)
    {
        if (find_node_index(dep_node->id) == -1)
        {
            node_ids[node_count++] = dep_node->id;
        }
        dep_node = dep_node->next;
    }

    // Initialize nodes with random positions
    srand(time(NULL));
    for (int i = 0; i < node_count && i < MAX_NETWORK_NODES; i++)
    {
        network_nodes[i].x = ((float)rand() / RAND_MAX - 0.5f) * 10.0f;
        network_nodes[i].y = ((float)rand() / RAND_MAX - 0.5f) * 10.0f;
        network_nodes[i].z = ((float)rand() / RAND_MAX - 0.5f) * 10.0f;
        network_nodes[i].vx = 0.0f;
        network_nodes[i].vy = 0.0f;
        network_nodes[i].vz = 0.0f;
        network_nodes[i].node_id = node_ids[i];
        network_nodes[i].type = DEPENDENCY_MODULE; // Default
        network_nodes[i].color = get_node_color(DEPENDENCY_MODULE, NULL, node_ids[i]);
        sprintf(network_nodes[i].label, "Node_%u", node_ids[i]);
        strcpy(network_nodes[i].node_type, "module");
    }
    num_network_nodes = node_count;

    // Build edges from dependency relationships
    // Include dependencies
    dep_node = graph->include_deps.head;
    while (dep_node)
    {
        int from_idx = find_node_index(dep_node->id);
        if (from_idx != -1 && dep_node->next)
        {
            int to_idx = find_node_index(dep_node->next->id);
            if (to_idx != -1)
            {
                add_edge(from_idx, to_idx, 1.0f);
            }
        }
        dep_node = dep_node->next;
    }

    // Function call edges from call graph
    for (uint32_t i = 0; i < graph->call_graph.node_count; i++)
    {
        CallEdge *edge = graph->call_graph.edges[i];
        while (edge)
        {
            int from_idx = find_node_index(edge->caller_id);
            int to_idx = find_node_index(edge->callee_id);
            if (from_idx != -1 && to_idx != -1)
            {
                add_edge(from_idx, to_idx, (float)edge->call_count);
            }
            edge = edge->next;
        }
    }

    // Apply force-directed layout
    for (int iter = 0; iter < FORCE_ITERATIONS; iter++)
    {
        apply_forces();
    }
}

static void apply_forces(void)
{
    // Reset forces
    for (int i = 0; i < num_network_nodes; i++)
    {
        network_nodes[i].vx = 0.0f;
        network_nodes[i].vy = 0.0f;
        network_nodes[i].vz = 0.0f;
    }

    // Repulsion forces between all pairs
    for (int i = 0; i < num_network_nodes; i++)
    {
        for (int j = i + 1; j < num_network_nodes; j++)
        {
            float dx = network_nodes[j].x - network_nodes[i].x;
            float dy = network_nodes[j].y - network_nodes[i].y;
            float dz = network_nodes[j].z - network_nodes[i].z;
            float distance = sqrtf(dx*dx + dy*dy + dz*dz);

            if (distance > 0.1f)
            {
                float force = REPULSION_FORCE / (distance * distance);
                float fx = force * dx / distance;
                float fy = force * dy / distance;
                float fz = force * dz / distance;

                network_nodes[i].vx -= fx;
                network_nodes[i].vy -= fy;
                network_nodes[i].vz -= fz;
                network_nodes[j].vx += fx;
                network_nodes[j].vy += fy;
                network_nodes[j].vz += fz;
            }
        }
    }

    // Attraction forces along edges
    for (int i = 0; i < num_network_edges; i++)
    {
        int from = network_edges[i].from_index;
        int to = network_edges[i].to_index;

        float dx = network_nodes[to].x - network_nodes[from].x;
        float dy = network_nodes[to].y - network_nodes[from].y;
        float dz = network_nodes[to].z - network_nodes[from].z;
        float distance = sqrtf(dx*dx + dy*dy + dz*dz);

        if (distance > 0.1f)
        {
            float force = ATTRACTION_FORCE * network_edges[i].weight * distance;
            float fx = force * dx / distance;
            float fy = force * dy / distance;
            float fz = force * dz / distance;

            network_nodes[from].vx += fx;
            network_nodes[from].vy += fy;
            network_nodes[from].vz += fz;
            network_nodes[to].vx -= fx;
            network_nodes[to].vy -= fy;
            network_nodes[to].vz -= fz;
        }
    }

    // Update positions
    for (int i = 0; i < num_network_nodes; i++)
    {
        network_nodes[i].x += network_nodes[i].vx * DAMPING;
        network_nodes[i].y += network_nodes[i].vy * DAMPING;
        network_nodes[i].z += network_nodes[i].vz * DAMPING;
    }
}

static void add_edge(int from_index, int to_index, float weight)
{
    if (num_network_edges >= MAX_NETWORK_EDGES)
    {
        return;
    }

    network_edges[num_network_edges].from_index = from_index;
    network_edges[num_network_edges].to_index = to_index;
    network_edges[num_network_edges].weight = weight;
    network_edges[num_network_edges].color = COLOR_GRAY;
    num_network_edges++;
}

static int find_node_index(uint32_t node_id)
{
    for (int i = 0; i < num_network_nodes; i++)
    {
        if (network_nodes[i].node_id == node_id)
        {
            return i;
        }
    }
    return -1;
}

static Color get_node_color(DependencyType type, const char *color_metric, uint32_t node_id)
{
    // Color based on node type
    switch (type)
    {
    case DEPENDENCY_FUNCTION_CALL:
        return COLOR_BLUE;
    case DEPENDENCY_INHERITANCE:
        return COLOR_RED;
    case DEPENDENCY_COMPOSITION:
        return COLOR_GREEN;
    case DEPENDENCY_TYPE:
        return COLOR_YELLOW;
    case DEPENDENCY_MODULE:
        return COLOR_PURPLE;
    default:
        return COLOR_GRAY;
    }
}

void network_graph_render(void)
{
    if (num_network_nodes == 0)
    {
        return;
    }

    // Draw edges first (behind nodes)
    for (int i = 0; i < num_network_edges; i++)
    {
        int from = network_edges[i].from_index;
        int to = network_edges[i].to_index;

        renderer_draw_line_color(
            network_nodes[from].x, network_nodes[from].y, network_nodes[from].z,
            network_nodes[to].x, network_nodes[to].y, network_nodes[to].z,
            &network_edges[i].color
        );
    }

    // Draw nodes
    for (int i = 0; i < num_network_nodes; i++)
    {
        renderer_draw_sphere_color(
            network_nodes[i].x,
            network_nodes[i].y,
            network_nodes[i].z,
            NODE_SIZE,
            &network_nodes[i].color
        );

        // Draw labels for some nodes to avoid clutter
        if (i % 10 == 0) // Label every 10th node
        {
            Color label_color = {1.0f, 1.0f, 1.0f, 1.0f};
            renderer_draw_text_3d(
                network_nodes[i].label,
                network_nodes[i].x + NODE_SIZE * 1.5f,
                network_nodes[i].y + NODE_SIZE * 1.5f,
                network_nodes[i].z,
                0.3f,
                &label_color
            );
        }
    }

    // Draw legend
    network_graph_draw_legend();
}

void network_graph_draw_legend(void)
{
    float legend_x = -8.0f;
    float legend_y = 4.0f;
    float legend_z = 0.0f;
    float spacing = 0.5f;

    Color text_color = {1.0f, 1.0f, 1.0f, 1.0f};

    // Title
    renderer_draw_text_3d("Dependencies:", legend_x, legend_y, legend_z, 0.4f, &text_color);

    // Function calls
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_BLUE);
    renderer_draw_text_3d("Functions", legend_x, legend_y - spacing, legend_z, 0.3f, &text_color);

    // Inheritance
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - 2 * spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_RED);
    renderer_draw_text_3d("Inheritance", legend_x, legend_y - 2 * spacing, legend_z, 0.3f, &text_color);

    // Composition
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - 3 * spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_GREEN);
    renderer_draw_text_3d("Composition", legend_x, legend_y - 3 * spacing, legend_z, 0.3f, &text_color);

    // Types
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - 4 * spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_YELLOW);
    renderer_draw_text_3d("Types", legend_x, legend_y - 4 * spacing, legend_z, 0.3f, &text_color);

    // Modules
    renderer_draw_sphere_color(legend_x - 0.3f, legend_y - 5 * spacing, legend_z, NODE_SIZE * 0.8f, &COLOR_PURPLE);
    renderer_draw_text_3d("Modules", legend_x, legend_y - 5 * spacing, legend_z, 0.3f, &text_color);
}

void network_graph_clear(void)
{
    num_network_nodes = 0;
    num_network_edges = 0;
    LOG_DEBUG("Network graph cleared");
}

int network_graph_get_node_count(void)
{
    return num_network_nodes;
}

int network_graph_get_edge_count(void)
{
    return num_network_edges;
}

CQError network_graph_get_node(int index, float *x, float *y, float *z,
                              Color *color, char *label)
{
    if (index < 0 || index >= num_network_nodes)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (x) *x = network_nodes[index].x;
    if (y) *y = network_nodes[index].y;
    if (z) *z = network_nodes[index].z;
    if (color) *color = network_nodes[index].color;
    if (label) strcpy(label, network_nodes[index].label);

    return CQ_SUCCESS;
}

CQError network_graph_get_edge(int index, int *from_node_index, int *to_node_index,
                              Color *color)
{
    if (index < 0 || index >= num_network_edges)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (from_node_index) *from_node_index = network_edges[index].from_index;
    if (to_node_index) *to_node_index = network_edges[index].to_index;
    if (color) *color = network_edges[index].color;

    return CQ_SUCCESS;
}