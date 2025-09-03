#ifndef PROGRESS_DISPLAY_H
#define PROGRESS_DISPLAY_H

#include "cqanalyzer.h"

/**
 * @file progress_display.h
 * @brief Progress display and status reporting
 *
 * Provides functions to display analysis progress and status
 * information to the user.
 */

/**
 * @brief Initialize progress display
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError progress_display_init(void);

/**
 * @brief Shutdown progress display
 */
void progress_display_shutdown(void);

/**
 * @brief Start progress tracking
 *
 * @param title Progress title
 * @param total_items Total number of items to process
 */
void progress_start(const char* title, int total_items);

/**
 * @brief Update progress
 *
 * @param current_item Current item being processed
 * @param status Optional status message
 */
void progress_update(int current_item, const char* status);

/**
 * @brief Complete progress
 *
 * @param message Completion message
 */
void progress_complete(const char* message);

/**
 * @brief Display error message
 *
 * @param message Error message
 */
void progress_display_error(const char* message);

/**
 * @brief Display warning message
 *
 * @param message Warning message
 */
void progress_display_warning(const char* message);

/**
 * @brief Display info message
 *
 * @param message Info message
 */
void progress_display_info(const char* message);

#endif // PROGRESS_DISPLAY_H
