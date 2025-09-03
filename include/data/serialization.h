#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "cqanalyzer.h"

/**
 * @file serialization.h
 * @brief Data serialization and deserialization
 *
 * Provides functions to save and load analysis results
 * in various formats (JSON, CSV, binary).
 */

/**
 * @brief Serialize project data to JSON
 *
 * @param filepath Output file path
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError serialize_to_json(const char* filepath);

/**
 * @brief Deserialize project data from JSON
 *
 * @param filepath Input file path
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError deserialize_from_json(const char* filepath);

/**
 * @brief Export metrics to CSV
 *
 * @param filepath Output file path
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError export_to_csv(const char* filepath);

/**
 * @brief Save analysis results in binary format
 *
 * @param filepath Output file path
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError save_binary_results(const char* filepath);

/**
 * @brief Load analysis results from binary format
 *
 * @param filepath Input file path
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError load_binary_results(const char* filepath);

#endif // SERIALIZATION_H
