#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "visualizer/scatter_plot.h"
#include "visualizer/renderer.h"
#include "visualizer/color.h"
#include "visualizer/gradient.h"
#include "visualizer/picking.h"
#include "visualizer/visualization_filters.h"
#include "data/data_store.h"
#include "analyzer/metric_calculator.h"
#include "utils/logger.h"

#define MAX_SCATTER_POINTS 1000
#define AXIS_LENGTH 10.0f
#define POINT_SIZE 0.05f

typedef struct
{
    float x, y, z;
    Color color;
    char label[256];
} ScatterPoint;

static ScatterPoint scatter_points[MAX_SCATTER_POINTS];
static int num_scatter_points = 0;
static char x_axis_label[64] = "X";
static char y_axis_label[64] = "Y";
static char z_axis_label[64] = "Z";
static char color_axis_label[64] = "Color";

// Filtering and display options
static VisualizationFilters current_filters;
static DisplayOptions current_display_options;

CQError scatter_plot_create(const char *x_metric, const char *y_metric, const char *z_metric, const char *color_metric)
{
    // Initialize with default filters and options
    visualization_filters_init(&current_filters);
    display_options_init(&current_display_options);

    return scatter_plot_create_filtered(x_metric, y_metric, z_metric, color_metric,
                                      &current_filters, &current_display_options);
}

CQError scatter_plot_create_filtered(const char *x_metric, const char *y_metric, const char *z_metric,
                                   const char *color_metric, const VisualizationFilters *filters,
                                   const DisplayOptions *options)
{
    if (!x_metric || !y_metric || !z_metric)
    {
        LOG_ERROR("Invalid metric names for scatter plot");
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Store filters and options
    if (filters)
    {
        current_filters = *filters;
    }
    else
    {
        visualization_filters_init(&current_filters);
    }

    if (options)
    {
        current_display_options = *options;
    }
    else
    {
        display_options_init(&current_display_options);
    }

    // Get all files from data store
    char filepaths[MAX_SCATTER_POINTS][MAX_PATH_LENGTH];
    int num_files = data_store_get_all_files(filepaths, MAX_SCATTER_POINTS);

    if (num_files == 0)
    {
        LOG_WARNING("No files found in data store for scatter plot");
        return CQ_ERROR_UNKNOWN;
    }

    // Collect metric values for all files that pass filters
    double x_values[MAX_SCATTER_POINTS];
    double y_values[MAX_SCATTER_POINTS];
    double z_values[MAX_SCATTER_POINTS];
    double color_values[MAX_SCATTER_POINTS];
    char filtered_filepaths[MAX_SCATTER_POINTS][MAX_PATH_LENGTH];
    int valid_points = 0;

    for (int i = 0; i < num_files && valid_points < MAX_SCATTER_POINTS; i++)
    {
        // Apply filters first
        if (filters && !visualization_filters_check_file(filters, filepaths[i]))
        {
            continue; // Skip files that don't pass filters
        }

        double x_val = data_store_get_metric(filepaths[i], x_metric);
        double y_val = data_store_get_metric(filepaths[i], y_metric);
        double z_val = data_store_get_metric(filepaths[i], z_metric);
        double color_val = color_metric ? data_store_get_metric(filepaths[i], color_metric) : 0.0;

        // Skip files that don't have all required metrics
        if (x_val < 0.0 || y_val < 0.0 || z_val < 0.0)
        {
            continue;
        }

        x_values[valid_points] = x_val;
        y_values[valid_points] = y_val;
        z_values[valid_points] = z_val;
        color_values[valid_points] = color_val;

        // Store filepath as label
        strcpy(filtered_filepaths[valid_points], filepaths[i]);
        valid_points++;
    }

    if (valid_points == 0)
    {
        LOG_WARNING("No valid data points found for scatter plot");
        return CQ_ERROR_UNKNOWN;
    }

    // Normalize values to [-5, 5] range for visualization
    double x_normalized[valid_points];
    double y_normalized[valid_points];
    double z_normalized[valid_points];
    double color_normalized[valid_points];

    normalize_metric_array(x_values, valid_points, NORMALIZATION_MIN_MAX, x_normalized);
    normalize_metric_array(y_values, valid_points, NORMALIZATION_MIN_MAX, y_normalized);
    normalize_metric_array(z_values, valid_points, NORMALIZATION_MIN_MAX, z_normalized);

    if (color_metric)
    {
        normalize_metric_array(color_values, valid_points, NORMALIZATION_MIN_MAX, color_normalized);
    }

    // Create scatter points
    num_scatter_points = valid_points;
    for (int i = 0; i < valid_points; i++)
    {
        scatter_points[i].x = (float)scale_metric(x_normalized[i], -5.0, 5.0);
        scatter_points[i].y = (float)scale_metric(y_normalized[i], -5.0, 5.0);
        scatter_points[i].z = (float)scale_metric(z_normalized[i], -5.0, 5.0);

        // Color based on color metric or default
        if (color_metric)
        {
            Color point_color = gradient_get_color(NULL, color_normalized[i]); // Use default gradient
            scatter_points[i].color = point_color;
        }
        else
        {
            scatter_points[i].color.r = 0.5f;
            scatter_points[i].color.g = 0.7f;
            scatter_points[i].color.b = 1.0f;
            scatter_points[i].color.a = 1.0f;
        }
    }

    // Register scatter points for picking
    picking_clear_objects(); // Clear any previous objects
    for (int i = 0; i < valid_points; i++)
    {
        PickableObject pickable_obj;
        pickable_obj.object_id = i; // Use index as ID
        pickable_obj.type = OBJECT_TYPE_SPHERE;
        pickable_obj.position[0] = scatter_points[i].x;
        pickable_obj.position[1] = scatter_points[i].y;
        pickable_obj.position[2] = scatter_points[i].z;
        pickable_obj.radius = POINT_SIZE;
        pickable_obj.size[0] = pickable_obj.size[1] = pickable_obj.size[2] = POINT_SIZE * 2.0f;
        strcpy(pickable_obj.label, scatter_points[i].label);

        picking_register_object(&pickable_obj);
    }

    // Set axis labels
    strcpy(x_axis_label, x_metric);
    strcpy(y_axis_label, y_metric);
    strcpy(z_axis_label, z_metric);
    if (color_metric)
    {
        strcpy(color_axis_label, color_metric);
    }
    else
    {
        strcpy(color_axis_label, "Default");
    }

    LOG_INFO("Created scatter plot with %d points", num_scatter_points);
    return CQ_SUCCESS;
}

void scatter_plot_render(void)
{
    if (num_scatter_points == 0)
    {
        return;
    }

    // Draw axes if enabled
    if (current_display_options.show_axes)
    {
        scatter_plot_draw_axes();
    }

    // Draw scatter points if enabled
    if (current_display_options.show_points)
    {
        for (int i = 0; i < num_scatter_points; i++)
        {
            Color render_color = scatter_points[i].color;

            // Check if this point is selected and highlight it
            if (picking_is_selected(i))
            {
                Color highlight_color;
                picking_get_highlight_color(&highlight_color);
                render_color = highlight_color;
            }

            renderer_draw_sphere_color(
                scatter_points[i].x,
                scatter_points[i].y,
                scatter_points[i].z,
                current_display_options.point_size,
                &render_color
            );
        }
    }

    // Draw labels if enabled
    if (current_display_options.show_labels)
    {
        int label_interval = num_scatter_points > 20 ? num_scatter_points / 10 : 1;
        for (int i = 0; i < num_scatter_points; i += label_interval)
        {
            // Extract filename from path for cleaner labels
            char *filename = strrchr(scatter_points[i].label, '/');
            if (!filename)
            {
                filename = scatter_points[i].label;
            }
            else
            {
                filename++; // Skip the '/'
            }

            Color label_color = {1.0f, 1.0f, 1.0f, 1.0f};
            renderer_draw_text_3d(
                filename,
                scatter_points[i].x + current_display_options.point_size,
                scatter_points[i].y + current_display_options.point_size,
                scatter_points[i].z + current_display_options.point_size,
                current_display_options.label_scale,
                &label_color
            );
        }
    }
}

void scatter_plot_draw_axes(void)
{
    Color axis_color = {0.7f, 0.7f, 0.7f, 1.0f};
    Color label_color = {1.0f, 1.0f, 1.0f, 1.0f};

    // X axis
    renderer_draw_line_color(-AXIS_LENGTH/2, 0.0f, 0.0f, AXIS_LENGTH/2, 0.0f, 0.0f, &axis_color);
    if (current_display_options.show_labels)
    {
        renderer_draw_text_3d(x_axis_label, AXIS_LENGTH/2 + 0.5f, 0.0f, 0.0f, 0.8f, &label_color);
    }

    // Y axis
    renderer_draw_line_color(0.0f, -AXIS_LENGTH/2, 0.0f, 0.0f, AXIS_LENGTH/2, 0.0f, &axis_color);
    if (current_display_options.show_labels)
    {
        renderer_draw_text_3d(y_axis_label, 0.0f, AXIS_LENGTH/2 + 0.5f, 0.0f, 0.8f, &label_color);
    }

    // Z axis
    renderer_draw_line_color(0.0f, 0.0f, -AXIS_LENGTH/2, 0.0f, 0.0f, AXIS_LENGTH/2, &axis_color);
    if (current_display_options.show_labels)
    {
        renderer_draw_text_3d(z_axis_label, 0.0f, 0.0f, AXIS_LENGTH/2 + 0.5f, 0.8f, &label_color);
    }

    // Draw grid lines for reference if enabled
    if (current_display_options.show_grid)
    {
        Color grid_color = {0.3f, 0.3f, 0.3f, 0.5f};
        for (float i = -5.0f; i <= 5.0f; i += 1.0f)
        {
            // XY plane grid
            renderer_draw_line_color(i, -5.0f, 0.0f, i, 5.0f, 0.0f, &grid_color);
            renderer_draw_line_color(-5.0f, i, 0.0f, 5.0f, i, 0.0f, &grid_color);

            // XZ plane grid
            renderer_draw_line_color(i, 0.0f, -5.0f, i, 0.0f, 5.0f, &grid_color);
            renderer_draw_line_color(-5.0f, 0.0f, i, 5.0f, 0.0f, i, &grid_color);

            // YZ plane grid
            renderer_draw_line_color(0.0f, i, -5.0f, 0.0f, i, 5.0f, &grid_color);
            renderer_draw_line_color(0.0f, -5.0f, i, 0.0f, 5.0f, i, &grid_color);
        }
    }
}

void scatter_plot_clear(void)
{
    num_scatter_points = 0;
    LOG_DEBUG("Scatter plot cleared");
}

int scatter_plot_get_point_count(void)
{
    return num_scatter_points;
}

CQError scatter_plot_get_point(int index, float *x, float *y, float *z, Color *color, char *label)
{
    if (index < 0 || index >= num_scatter_points)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (x) *x = scatter_points[index].x;
    if (y) *y = scatter_points[index].y;
    if (z) *z = scatter_points[index].z;
    if (color) *color = scatter_points[index].color;
    if (label) strcpy(label, scatter_points[index].label);

    return CQ_SUCCESS;
}

void scatter_plot_set_display_options(const DisplayOptions *options)
{
    if (options)
    {
        current_display_options = *options;
        LOG_DEBUG("Display options updated for scatter plot");
    }
}

void scatter_plot_get_display_options(DisplayOptions *options)
{
    if (options)
    {
        *options = current_display_options;
    }
}

void scatter_plot_toggle_axes(void)
{
    display_options_toggle_axes(&current_display_options);
}

void scatter_plot_toggle_labels(void)
{
    display_options_toggle_labels(&current_display_options);
}

void scatter_plot_toggle_grid(void)
{
    display_options_toggle_grid(&current_display_options);
}

void scatter_plot_toggle_points(void)
{
    display_options_toggle_points(&current_display_options);
}