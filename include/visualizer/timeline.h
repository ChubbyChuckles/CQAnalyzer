#ifndef TIMELINE_H
#define TIMELINE_H

#include "cqanalyzer.h"
#include "visualizer/color.h"

/**
 * @file timeline.h
 * @brief Timeline visualization for metric evolution over time
 *
 * Provides functions to create and render timeline charts
 * for showing how code metrics change over time (versions, commits, etc.).
 */

#define MAX_TIMELINE_POINTS 1000
#define MAX_TIMELINE_SERIES 10
#define MAX_TIMELINE_LABEL_LENGTH 64

/**
 * @brief Timeline data point
 */
typedef struct
{
    double timestamp;                    // Unix timestamp or version number
    double value;                        // Metric value at this point
    char label[MAX_TIMELINE_LABEL_LENGTH]; // Optional label (version, commit, etc.)
} TimelinePoint;

/**
 * @brief Timeline series (one line on the chart)
 */
typedef struct
{
    char name[MAX_TIMELINE_LABEL_LENGTH]; // Series name (metric name)
    TimelinePoint points[MAX_TIMELINE_POINTS];
    int point_count;
    Color color;                         // Line color
    bool show_points;                    // Whether to show data points
    bool show_line;                      // Whether to show connecting line
} TimelineSeries;

/**
 * @brief Timeline configuration
 */
typedef struct
{
    char title[128];                     // Chart title
    float width;                         // Chart width
    float height;                        // Chart height
    float line_width;                    // Width of timeline lines
    float point_size;                    // Size of data points
    bool show_grid;                      // Whether to show grid lines
    bool show_labels;                    // Whether to show point labels
    bool show_legend;                    // Whether to show legend
    bool auto_scale;                     // Whether to auto-scale Y axis
    double min_value;                    // Manual minimum Y value
    double max_value;                    // Manual maximum Y value
    double start_time;                   // Manual start time
    double end_time;                     // Manual end time
    Color background_color;              // Chart background color
    Color grid_color;                    // Grid line color
    Color text_color;                    // Text color
} TimelineConfig;

/**
 * @brief Initialize timeline system
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError timeline_init(void);

/**
 * @brief Shutdown timeline system
 */
void timeline_shutdown(void);

/**
 * @brief Create a new timeline chart
 *
 * @param title Chart title
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError timeline_create(const char *title);

/**
 * @brief Add a data series to the timeline
 *
 * @param name Series name (metric name)
 * @param color Series color (can be NULL for default)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError timeline_add_series(const char *name, const Color *color);

/**
 * @brief Add a data point to a series
 *
 * @param series_index Index of the series
 * @param timestamp Timestamp or version number
 * @param value Metric value
 * @param label Optional label for the point
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError timeline_add_point(int series_index, double timestamp, double value, const char *label);

/**
 * @brief Set timeline configuration
 *
 * @param config Configuration structure
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError timeline_set_config(const TimelineConfig *config);

/**
 * @brief Get current timeline configuration
 *
 * @param config Output configuration structure
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError timeline_get_config(TimelineConfig *config);

/**
 * @brief Render the timeline chart
 */
void timeline_render(void);

/**
 * @brief Clear all timeline data
 */
void timeline_clear(void);

/**
 * @brief Get the number of series in the timeline
 *
 * @return Number of series
 */
int timeline_get_series_count(void);

/**
 * @brief Get the number of points in a series
 *
 * @param series_index Series index
 * @return Number of points in the series
 */
int timeline_get_point_count(int series_index);

/**
 * @brief Get data for a specific point
 *
 * @param series_index Series index
 * @param point_index Point index
 * @param timestamp Pointer to store timestamp (can be NULL)
 * @param value Pointer to store value (can be NULL)
 * @param label Pointer to store label string (can be NULL)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError timeline_get_point(int series_index, int point_index, double *timestamp, double *value, char *label);

/**
 * @brief Set Y-axis range manually
 *
 * @param min_value Minimum value
 * @param max_value Maximum value
 */
void timeline_set_y_range(double min_value, double max_value);

/**
 * @brief Set time range manually
 *
 * @param start_time Start timestamp
 * @param end_time End timestamp
 */
void timeline_set_time_range(double start_time, double end_time);

/**
 * @brief Auto-scale Y-axis based on current data
 */
void timeline_auto_scale_y(void);

/**
 * @brief Auto-scale time axis based on current data
 */
void timeline_auto_scale_time(void);

/**
 * @brief Set default series color
 *
 * @param color Default color for new series
 */
void timeline_set_default_color(const Color *color);

#endif // TIMELINE_H