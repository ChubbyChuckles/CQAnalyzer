#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "visualizer/scatter_plot.h"
#include "visualizer/renderer.h"
#include "visualizer/color.h"
#include "visualizer/gradient.h"
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

CQError scatter_plot_create(const char *x_metric, const char *y_metric, const char *z_metric, const char *color_metric)
{
    if (!x_metric || !y_metric || !z_metric)
    {
        LOG_ERROR("Invalid metric names for scatter plot");
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Get all files from data store
    char filepaths[MAX_SCATTER_POINTS][MAX_PATH_LENGTH];
    int num_files = data_store_get_all_files((char *)filepaths, MAX_SCATTER_POINTS);

    if (num_files == 0)
    {
        LOG_WARNING("No files found in data store for scatter plot");
        return CQ_ERROR_UNKNOWN;
    }

    // Collect metric values for all files
    double x_values[MAX_SCATTER_POINTS];
    double y_values[MAX_SCATTER_POINTS];
    double z_values[MAX_SCATTER_POINTS];
    double color_values[MAX_SCATTER_POINTS];
    int valid_points = 0;

    for (int i = 0; i < num_files && valid_points < MAX_SCATTER_POINTS; i++)
    {
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
        strcpy(scatter_points[valid_points].label, filepaths[i]);
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

    // Draw axes
    scatter_plot_draw_axes();

    // Draw scatter points
    for (int i = 0; i < num_scatter_points; i++)
    {
        renderer_draw_sphere_color(
            scatter_points[i].x,
            scatter_points[i].y,
            scatter_points[i].z,
            POINT_SIZE,
            &scatter_points[i].color
        );
    }

    // Draw labels for a few points (to avoid clutter)
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
            scatter_points[i].x + POINT_SIZE,
            scatter_points[i].y + POINT_SIZE,
            scatter_points[i].z + POINT_SIZE,
            0.5f,
            &label_color
        );
    }
}

void scatter_plot_draw_axes(void)
{
    Color axis_color = {0.7f, 0.7f, 0.7f, 1.0f};
    Color label_color = {1.0f, 1.0f, 1.0f, 1.0f};

    // X axis
    renderer_draw_line_color(-AXIS_LENGTH/2, 0.0f, 0.0f, AXIS_LENGTH/2, 0.0f, 0.0f, &axis_color);
    renderer_draw_text_3d(x_axis_label, AXIS_LENGTH/2 + 0.5f, 0.0f, 0.0f, 0.8f, &label_color);

    // Y axis
    renderer_draw_line_color(0.0f, -AXIS_LENGTH/2, 0.0f, 0.0f, AXIS_LENGTH/2, 0.0f, &axis_color);
    renderer_draw_text_3d(y_axis_label, 0.0f, AXIS_LENGTH/2 + 0.5f, 0.0f, 0.8f, &label_color);

    // Z axis
    renderer_draw_line_color(0.0f, 0.0f, -AXIS_LENGTH/2, 0.0f, 0.0f, AXIS_LENGTH/2, &axis_color);
    renderer_draw_text_3d(z_axis_label, 0.0f, 0.0f, AXIS_LENGTH/2 + 0.5f, 0.8f, &label_color);

    // Draw grid lines for reference
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