#ifndef DUPLICATION_DETECTOR_H
#define DUPLICATION_DETECTOR_H

#include "cqanalyzer.h"

/**
 * @file duplication_detector.h
 * @brief Code duplication detection
 *
 * Provides functions to detect duplicate code patterns
 * and calculate duplication metrics.
 */

/**
 * @brief Detect code duplication in a file
 *
 * @param filepath Source file path
 * @param duplication_ratio Output duplication ratio (0.0 to 1.0)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError detect_file_duplication(const char* filepath, double* duplication_ratio);

/**
 * @brief Detect code duplication across multiple files
 *
 * @param filepaths Array of file paths
 * @param num_files Number of files
 * @param duplication_ratio Output duplication ratio (0.0 to 1.0)
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError detect_project_duplication(const char** filepaths, int num_files, double* duplication_ratio);

#endif // DUPLICATION_DETECTOR_H
