#ifndef VISUALIZATION_FILTERS_H
#define VISUALIZATION_FILTERS_H

#include "cqanalyzer.h"

/**
 * @file visualization_filters.h
 * @brief Metric filtering and display options for visualizations
 */

typedef enum
{
    FILTER_TYPE_NONE,
    FILTER_TYPE_RANGE,      // Filter by min/max range
    FILTER_TYPE_THRESHOLD,  // Filter by threshold (above/below)
    FILTER_TYPE_TOP_N,      // Show only top N values
    FILTER_TYPE_BOTTOM_N    // Show only bottom N values
} FilterType;

typedef enum
{
    THRESHOLD_ABOVE,
    THRESHOLD_BELOW,
    THRESHOLD_EQUAL
} ThresholdMode;

typedef struct
{
    char metric_name[64];
    FilterType type;
    union
    {
        struct
        {
            double min_value;
            double max_value;
        } range;
        struct
        {
            double value;
            ThresholdMode mode;
        } threshold;
        int count; // For TOP_N and BOTTOM_N
    } params;
} MetricFilter;

typedef struct
{
    int num_filters;
    MetricFilter filters[10]; // Maximum 10 filters
} VisualizationFilters;

typedef struct
{
    bool show_axes;
    bool show_labels;
    bool show_grid;
    bool show_points;
    bool show_connections;
    float point_size;
    float label_scale;
} DisplayOptions;

/**
 * @brief Initialize visualization filters
 *
 * @param filters Pointer to filters structure
 */
void visualization_filters_init(VisualizationFilters *filters);

/**
 * @brief Add a range filter
 *
 * @param filters Pointer to filters structure
 * @param metric_name Name of metric to filter
 * @param min_value Minimum value
 * @param max_value Maximum value
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError visualization_filters_add_range(VisualizationFilters *filters,
                                       const char *metric_name,
                                       double min_value,
                                       double max_value);

/**
 * @brief Add a threshold filter
 *
 * @param filters Pointer to filters structure
 * @param metric_name Name of metric to filter
 * @param value Threshold value
 * @param mode Threshold mode (above/below/equal)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError visualization_filters_add_threshold(VisualizationFilters *filters,
                                          const char *metric_name,
                                          double value,
                                          ThresholdMode mode);

/**
 * @brief Add a top N filter
 *
 * @param filters Pointer to filters structure
 * @param metric_name Name of metric to filter
 * @param count Number of top values to show
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError visualization_filters_add_top_n(VisualizationFilters *filters,
                                       const char *metric_name,
                                       int count);

/**
 * @brief Clear all filters
 *
 * @param filters Pointer to filters structure
 */
void visualization_filters_clear(VisualizationFilters *filters);

/**
 * @brief Check if a file passes all filters
 *
 * @param filters Pointer to filters structure
 * @param filepath File path to check
 * @return true if file passes all filters, false otherwise
 */
bool visualization_filters_check_file(const VisualizationFilters *filters,
                                    const char *filepath);

/**
 * @brief Initialize display options with defaults
 *
 * @param options Pointer to display options structure
 */
void display_options_init(DisplayOptions *options);

/**
 * @brief Toggle axes visibility
 *
 * @param options Pointer to display options structure
 */
void display_options_toggle_axes(DisplayOptions *options);

/**
 * @brief Toggle labels visibility
 *
 * @param options Pointer to display options structure
 */
void display_options_toggle_labels(DisplayOptions *options);

/**
 * @brief Toggle grid visibility
 *
 * @param options Pointer to display options structure
 */
void display_options_toggle_grid(DisplayOptions *options);

/**
 * @brief Toggle points visibility
 *
 * @param options Pointer to display options structure
 */
void display_options_toggle_points(DisplayOptions *options);

/**
 * @brief Toggle connections visibility
 *
 * @param options Pointer to display options structure
 */
void display_options_toggle_connections(DisplayOptions *options);

/**
 * @brief Set point size
 *
 * @param options Pointer to display options structure
 * @param size Point size
 */
void display_options_set_point_size(DisplayOptions *options, float size);

/**
 * @brief Set label scale
 *
 * @param options Pointer to display options structure
 * @param scale Label scale
 */
void display_options_set_label_scale(DisplayOptions *options, float scale);

#endif // VISUALIZATION_FILTERS_H