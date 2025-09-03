#include "visualizer/timeline.h"
#include "visualizer/renderer.h"
#include "visualizer/color.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

/**
 * @file timeline.c
 * @brief Timeline visualization implementation
 */

static TimelineSeries series[MAX_TIMELINE_SERIES];
static int series_count = 0;
static TimelineConfig config;
static Color default_color;
static bool initialized = false;

/**
 * @brief Initialize timeline system
 */
CQError timeline_init(void)
{
    if (initialized)
        return CQ_SUCCESS;

    // Initialize default configuration
    memset(&config, 0, sizeof(TimelineConfig));
    strcpy(config.title, "Timeline Chart");
    config.width = 20.0f;
    config.height = 10.0f;
    config.line_width = 2.0f;
    config.point_size = 0.1f;
    config.show_grid = true;
    config.show_labels = false;
    config.show_legend = true;
    config.auto_scale = true;
    config.min_value = 0.0;
    config.max_value = 100.0;
    config.start_time = 0.0;
    config.end_time = 100.0;
    config.background_color = color_create(0.1f, 0.1f, 0.1f, 1.0f);
    config.grid_color = color_create(0.3f, 0.3f, 0.3f, 1.0f);
    config.text_color = color_create(1.0f, 1.0f, 1.0f, 1.0f);

    // Set default series color
    default_color = color_create(0.2f, 0.6f, 1.0f, 1.0f);

    series_count = 0;
    initialized = true;

    return CQ_SUCCESS;
}

/**
 * @brief Shutdown timeline system
 */
void timeline_shutdown(void)
{
    timeline_clear();
    initialized = false;
}

/**
 * @brief Create a new timeline chart
 */
CQError timeline_create(const char *title)
{
    if (!initialized)
        return CQ_ERROR_UNKNOWN;

    timeline_clear();

    if (title)
        strncpy(config.title, title, sizeof(config.title) - 1);

    return CQ_SUCCESS;
}

/**
 * @brief Add a data series to the timeline
 */
CQError timeline_add_series(const char *name, const Color *color)
{
    if (!initialized)
        return CQ_ERROR_UNKNOWN;

    if (series_count >= MAX_TIMELINE_SERIES)
        return CQ_ERROR_MEMORY_ALLOCATION;

    TimelineSeries *s = &series[series_count];
    memset(s, 0, sizeof(TimelineSeries));

    if (name)
        strncpy(s->name, name, sizeof(s->name) - 1);
    else
        sprintf(s->name, "Series %d", series_count + 1);

    if (color)
        s->color = *color;
    else
        s->color = default_color;

    s->show_points = true;
    s->show_line = true;
    s->point_count = 0;

    series_count++;
    return CQ_SUCCESS;
}

/**
 * @brief Add a data point to a series
 */
CQError timeline_add_point(int series_index, double timestamp, double value, const char *label)
{
    if (!initialized)
        return CQ_ERROR_UNKNOWN;

    if (series_index < 0 || series_index >= series_count)
        return CQ_ERROR_INVALID_ARGUMENT;

    TimelineSeries *s = &series[series_index];

    if (s->point_count >= MAX_TIMELINE_POINTS)
        return CQ_ERROR_MEMORY_ALLOCATION;

    TimelinePoint *point = &s->points[s->point_count];
    point->timestamp = timestamp;
    point->value = value;

    if (label)
        strncpy(point->label, label, sizeof(point->label) - 1);
    else
        point->label[0] = '\0';

    s->point_count++;
    return CQ_SUCCESS;
}

/**
 * @brief Set timeline configuration
 */
CQError timeline_set_config(const TimelineConfig *new_config)
{
    if (!initialized)
        return CQ_ERROR_UNKNOWN;

    if (!new_config)
        return CQ_ERROR_INVALID_ARGUMENT;

    config = *new_config;
    return CQ_SUCCESS;
}

/**
 * @brief Get current timeline configuration
 */
CQError timeline_get_config(TimelineConfig *out_config)
{
    if (!initialized)
        return CQ_ERROR_UNKNOWN;

    if (!out_config)
        return CQ_ERROR_INVALID_ARGUMENT;

    *out_config = config;
    return CQ_SUCCESS;
}

/**
 * @brief Render the timeline chart
 */
void timeline_render(void)
{
    if (!initialized || series_count == 0)
        return;

    // Calculate data ranges
    double global_min_time = DBL_MAX;
    double global_max_time = DBL_MIN;
    double global_min_value = DBL_MAX;
    double global_max_value = DBL_MIN;

    for (int i = 0; i < series_count; i++)
    {
        TimelineSeries *s = &series[i];
        for (int j = 0; j < s->point_count; j++)
        {
            TimelinePoint *point = &s->points[j];
            if (point->timestamp < global_min_time) global_min_time = point->timestamp;
            if (point->timestamp > global_max_time) global_max_time = point->timestamp;
            if (point->value < global_min_value) global_min_value = point->value;
            if (point->value > global_max_value) global_max_value = point->value;
        }
    }

    // Handle empty data
    if (global_min_time == DBL_MAX || global_max_time == DBL_MIN)
        return;

    // Set ranges
    double min_time, max_time, min_value, max_value;

    if (config.auto_scale)
    {
        min_time = global_min_time;
        max_time = global_max_time;
        min_value = global_min_value;
        max_value = global_max_value;
    }
    else
    {
        min_time = config.start_time;
        max_time = config.end_time;
        min_value = config.min_value;
        max_value = config.max_value;
    }

    // Ensure valid ranges
    if (min_time >= max_time) max_time = min_time + 1.0;
    if (min_value >= max_value) max_value = min_value + 1.0;

    // Chart position and size
    float chart_left = -config.width / 2.0f;
    float chart_right = config.width / 2.0f;
    float chart_bottom = -config.height / 2.0f;
    float chart_top = config.height / 2.0f;

    // Draw grid if enabled
    if (config.show_grid)
    {
        // Draw background plane
        renderer_draw_cube_color(0.0f, 0.0f, -0.05f, config.width + 1.0f, &config.grid_color);

        // Draw grid lines
        int num_vertical_lines = 10;
        int num_horizontal_lines = 8;

        // Vertical grid lines (time axis)
        for (int i = 0; i <= num_vertical_lines; i++)
        {
            float t = (float)i / (float)num_vertical_lines;
            float x = chart_left + t * config.width;
            renderer_draw_line_color(x, chart_bottom, 0.0f, x, chart_top, 0.0f, &config.grid_color);
        }

        // Horizontal grid lines (value axis)
        for (int i = 0; i <= num_horizontal_lines; i++)
        {
            float t = (float)i / (float)num_horizontal_lines;
            float y = chart_bottom + t * config.height;
            renderer_draw_line_color(chart_left, y, 0.0f, chart_right, y, 0.0f, &config.grid_color);
        }
    }

    // Draw series
    for (int series_idx = 0; series_idx < series_count; series_idx++)
    {
        TimelineSeries *s = &series[series_idx];

        if (s->point_count == 0)
            continue;

        // Sort points by timestamp for proper line drawing
        // Simple bubble sort for now
        for (int i = 0; i < s->point_count - 1; i++)
        {
            for (int j = 0; j < s->point_count - i - 1; j++)
            {
                if (s->points[j].timestamp > s->points[j + 1].timestamp)
                {
                    TimelinePoint temp = s->points[j];
                    s->points[j] = s->points[j + 1];
                    s->points[j + 1] = temp;
                }
            }
        }

        // Draw connecting lines
        if (s->show_line && s->point_count > 1)
        {
            for (int i = 0; i < s->point_count - 1; i++)
            {
                TimelinePoint *p1 = &s->points[i];
                TimelinePoint *p2 = &s->points[i + 1];

                // Transform to chart coordinates
                float x1 = chart_left + ((p1->timestamp - min_time) / (max_time - min_time)) * config.width;
                float y1 = chart_bottom + ((p1->value - min_value) / (max_value - min_value)) * config.height;

                float x2 = chart_left + ((p2->timestamp - min_time) / (max_time - min_time)) * config.width;
                float y2 = chart_bottom + ((p2->value - min_value) / (max_value - min_value)) * config.height;

                renderer_draw_line_color(x1, y1, 0.0f, x2, y2, 0.0f, &s->color);
            }
        }

        // Draw data points
        if (s->show_points)
        {
            for (int i = 0; i < s->point_count; i++)
            {
                TimelinePoint *point = &s->points[i];

                // Transform to chart coordinates
                float x = chart_left + ((point->timestamp - min_time) / (max_time - min_time)) * config.width;
                float y = chart_bottom + ((point->value - min_value) / (max_value - min_value)) * config.height;

                // Draw point as small sphere
                renderer_draw_sphere_color(x, y, 0.0f, config.point_size, &s->color);

                // Draw label if enabled and label exists
                if (config.show_labels && strlen(point->label) > 0)
                {
                    renderer_draw_text_3d(point->label, x, y + 0.3f, 0.0f, 0.3f, &s->color);
                }
            }
        }
    }

    // Draw chart title
    if (strlen(config.title) > 0)
    {
        renderer_draw_text(config.title, -config.width / 2.0f, config.height / 2.0f + 1.0f, 0.8f, &config.text_color);
    }

    // Draw legend if enabled
    if (config.show_legend && series_count > 0)
    {
        float legend_x = chart_right + 1.0f;
        float legend_y = chart_top;

        for (int i = 0; i < series_count; i++)
        {
            TimelineSeries *s = &series[i];

            // Draw legend color box
            renderer_draw_cube_color(legend_x, legend_y - (float)i * 0.8f, 0.0f, 0.3f, &s->color);

            // Draw legend text
            renderer_draw_text(s->name, legend_x + 0.5f, legend_y - (float)i * 0.8f - 0.1f, 0.4f, &config.text_color);
        }
    }

    // Draw axis labels
    renderer_draw_text("Time", 0.0f, chart_bottom - 1.0f, 0.6f, &config.text_color);
    renderer_draw_text_3d("Value", chart_left - 1.0f, 0.0f, 0.0f, 0.6f, &config.text_color);
}

/**
 * @brief Clear all timeline data
 */
void timeline_clear(void)
{
    series_count = 0;
    memset(series, 0, sizeof(series));
}

/**
 * @brief Get the number of series in the timeline
 */
int timeline_get_series_count(void)
{
    return series_count;
}

/**
 * @brief Get the number of points in a series
 */
int timeline_get_point_count(int series_index)
{
    if (series_index < 0 || series_index >= series_count)
        return 0;

    return series[series_index].point_count;
}

/**
 * @brief Get data for a specific point
 */
CQError timeline_get_point(int series_index, int point_index, double *timestamp, double *value, char *label)
{
    if (!initialized)
        return CQ_ERROR_UNKNOWN;

    if (series_index < 0 || series_index >= series_count)
        return CQ_ERROR_INVALID_ARGUMENT;

    TimelineSeries *s = &series[series_index];

    if (point_index < 0 || point_index >= s->point_count)
        return CQ_ERROR_INVALID_ARGUMENT;

    TimelinePoint *point = &s->points[point_index];

    if (timestamp)
        *timestamp = point->timestamp;

    if (value)
        *value = point->value;

    if (label)
        strcpy(label, point->label);

    return CQ_SUCCESS;
}

/**
 * @brief Set Y-axis range manually
 */
void timeline_set_y_range(double min_value, double max_value)
{
    if (min_value < max_value)
    {
        config.min_value = min_value;
        config.max_value = max_value;
        config.auto_scale = false;
    }
}

/**
 * @brief Set time range manually
 */
void timeline_set_time_range(double start_time, double end_time)
{
    if (start_time < end_time)
    {
        config.start_time = start_time;
        config.end_time = end_time;
        config.auto_scale = false;
    }
}

/**
 * @brief Auto-scale Y-axis based on current data
 */
void timeline_auto_scale_y(void)
{
    config.auto_scale = true;
}

/**
 * @brief Auto-scale time axis based on current data
 */
void timeline_auto_scale_time(void)
{
    config.auto_scale = true;
}

/**
 * @brief Set default series color
 */
void timeline_set_default_color(const Color *color)
{
    if (color)
        default_color = *color;
}