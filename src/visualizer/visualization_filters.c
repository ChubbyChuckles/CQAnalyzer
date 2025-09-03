#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "visualizer/visualization_filters.h"
#include "data/data_store.h"
#include "utils/logger.h"

void visualization_filters_init(VisualizationFilters *filters)
{
    if (!filters)
    {
        return;
    }

    filters->num_filters = 0;
    memset(filters->filters, 0, sizeof(filters->filters));
}

CQError visualization_filters_add_range(VisualizationFilters *filters,
                                       const char *metric_name,
                                       double min_value,
                                       double max_value)
{
    if (!filters || !metric_name)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (filters->num_filters >= 10)
    {
        LOG_ERROR("Maximum number of filters reached");
        return CQ_ERROR_UNKNOWN;
    }

    MetricFilter *filter = &filters->filters[filters->num_filters];
    strcpy(filter->metric_name, metric_name);
    filter->type = FILTER_TYPE_RANGE;
    filter->params.range.min_value = min_value;
    filter->params.range.max_value = max_value;

    filters->num_filters++;
    LOG_DEBUG("Added range filter for metric '%s': [%f, %f]", metric_name, min_value, max_value);

    return CQ_SUCCESS;
}

CQError visualization_filters_add_threshold(VisualizationFilters *filters,
                                          const char *metric_name,
                                          double value,
                                          ThresholdMode mode)
{
    if (!filters || !metric_name)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (filters->num_filters >= 10)
    {
        LOG_ERROR("Maximum number of filters reached");
        return CQ_ERROR_UNKNOWN;
    }

    MetricFilter *filter = &filters->filters[filters->num_filters];
    strcpy(filter->metric_name, metric_name);
    filter->type = FILTER_TYPE_THRESHOLD;
    filter->params.threshold.value = value;
    filter->params.threshold.mode = mode;

    filters->num_filters++;
    LOG_DEBUG("Added threshold filter for metric '%s': %f (%s)",
              metric_name, value,
              mode == THRESHOLD_ABOVE ? "above" :
              mode == THRESHOLD_BELOW ? "below" : "equal");

    return CQ_SUCCESS;
}

CQError visualization_filters_add_top_n(VisualizationFilters *filters,
                                       const char *metric_name,
                                       int count)
{
    if (!filters || !metric_name || count <= 0)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (filters->num_filters >= 10)
    {
        LOG_ERROR("Maximum number of filters reached");
        return CQ_ERROR_UNKNOWN;
    }

    MetricFilter *filter = &filters->filters[filters->num_filters];
    strcpy(filter->metric_name, metric_name);
    filter->type = FILTER_TYPE_TOP_N;
    filter->params.count = count;

    filters->num_filters++;
    LOG_DEBUG("Added top %d filter for metric '%s'", count, metric_name);

    return CQ_SUCCESS;
}

void visualization_filters_clear(VisualizationFilters *filters)
{
    if (!filters)
    {
        return;
    }

    filters->num_filters = 0;
    LOG_DEBUG("Cleared all visualization filters");
}

bool visualization_filters_check_file(const VisualizationFilters *filters,
                                    const char *filepath)
{
    if (!filters || !filepath)
    {
        return true; // No filters means all files pass
    }

    for (int i = 0; i < filters->num_filters; i++)
    {
        const MetricFilter *filter = &filters->filters[i];
        double value = data_store_get_metric(filepath, filter->metric_name);

        if (value < 0.0)
        {
            // Metric not found for this file
            return false;
        }

        switch (filter->type)
        {
        case FILTER_TYPE_RANGE:
            if (value < filter->params.range.min_value ||
                value > filter->params.range.max_value)
            {
                return false;
            }
            break;

        case FILTER_TYPE_THRESHOLD:
            switch (filter->params.threshold.mode)
            {
            case THRESHOLD_ABOVE:
                if (value <= filter->params.threshold.value)
                {
                    return false;
                }
                break;
            case THRESHOLD_BELOW:
                if (value >= filter->params.threshold.value)
                {
                    return false;
                }
                break;
            case THRESHOLD_EQUAL:
                if (value != filter->params.threshold.value)
                {
                    return false;
                }
                break;
            }
            break;

        case FILTER_TYPE_TOP_N:
            // For top N, we need to check against all values
            // This is a simplified implementation - in practice, you'd want to
            // pre-sort and cache the top N values
            {
                double all_values[1000];
                int num_values = data_store_get_all_metric_values(filter->metric_name,
                                                                all_values, 1000);
                if (num_values > 0)
                {
                    // Sort in descending order to find top N
                    for (int j = 0; j < num_values - 1; j++)
                    {
                        for (int k = j + 1; k < num_values; k++)
                        {
                            if (all_values[j] < all_values[k])
                            {
                                double temp = all_values[j];
                                all_values[j] = all_values[k];
                                all_values[k] = temp;
                            }
                        }
                    }

                    // Check if value is in top N
                    bool in_top_n = false;
                    for (int j = 0; j < filter->params.count && j < num_values; j++)
                    {
                        if (value >= all_values[j])
                        {
                            in_top_n = true;
                            break;
                        }
                    }
                    if (!in_top_n)
                    {
                        return false;
                    }
                }
            }
            break;

        case FILTER_TYPE_BOTTOM_N:
            // Similar to top N but for bottom values
            {
                double all_values[1000];
                int num_values = data_store_get_all_metric_values(filter->metric_name,
                                                                all_values, 1000);
                if (num_values > 0)
                {
                    // Sort in ascending order to find bottom N
                    for (int j = 0; j < num_values - 1; j++)
                    {
                        for (int k = j + 1; k < num_values; k++)
                        {
                            if (all_values[j] > all_values[k])
                            {
                                double temp = all_values[j];
                                all_values[j] = all_values[k];
                                all_values[k] = temp;
                            }
                        }
                    }

                    // Check if value is in bottom N
                    bool in_bottom_n = false;
                    for (int j = 0; j < filter->params.count && j < num_values; j++)
                    {
                        if (value <= all_values[j])
                        {
                            in_bottom_n = true;
                            break;
                        }
                    }
                    if (!in_bottom_n)
                    {
                        return false;
                    }
                }
            }
            break;

        default:
            break;
        }
    }

    return true;
}

void display_options_init(DisplayOptions *options)
{
    if (!options)
    {
        return;
    }

    options->show_axes = true;
    options->show_labels = true;
    options->show_grid = true;
    options->show_points = true;
    options->show_connections = true;
    options->point_size = 0.05f;
    options->label_scale = 0.5f;
}

void display_options_toggle_axes(DisplayOptions *options)
{
    if (!options)
    {
        return;
    }

    options->show_axes = !options->show_axes;
    LOG_INFO("Axes display %s", options->show_axes ? "enabled" : "disabled");
}

void display_options_toggle_labels(DisplayOptions *options)
{
    if (!options)
    {
        return;
    }

    options->show_labels = !options->show_labels;
    LOG_INFO("Labels display %s", options->show_labels ? "enabled" : "disabled");
}

void display_options_toggle_grid(DisplayOptions *options)
{
    if (!options)
    {
        return;
    }

    options->show_grid = !options->show_grid;
    LOG_INFO("Grid display %s", options->show_grid ? "enabled" : "disabled");
}

void display_options_toggle_points(DisplayOptions *options)
{
    if (!options)
    {
        return;
    }

    options->show_points = !options->show_points;
    LOG_INFO("Points display %s", options->show_points ? "enabled" : "disabled");
}

void display_options_toggle_connections(DisplayOptions *options)
{
    if (!options)
    {
        return;
    }

    options->show_connections = !options->show_connections;
    LOG_INFO("Connections display %s", options->show_connections ? "enabled" : "disabled");
}

void display_options_set_point_size(DisplayOptions *options, float size)
{
    if (!options || size <= 0.0f)
    {
        return;
    }

    options->point_size = size;
    LOG_DEBUG("Point size set to %f", size);
}

void display_options_set_label_scale(DisplayOptions *options, float scale)
{
    if (!options || scale <= 0.0f)
    {
        return;
    }

    options->label_scale = scale;
    LOG_DEBUG("Label scale set to %f", scale);
}