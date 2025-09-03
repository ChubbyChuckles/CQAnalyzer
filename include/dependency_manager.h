/**
 * @file dependency_manager.h
 * @brief Dependency detection and graceful degradation system for CQAnalyzer
 *
 * This module provides functionality to detect the availability of optional
 * dependencies and manage graceful degradation when libraries are missing.
 */

#ifndef DEPENDENCY_MANAGER_H
#define DEPENDENCY_MANAGER_H

#include "cqanalyzer.h"

// Dependency types
typedef enum {
    DEP_OPENGL,
    DEP_GLFW3,
    DEP_GLEW,
    DEP_GLM,
    DEP_FREETYPE,
    DEP_LIBCLANG,
    DEP_CJSON,
    DEP_PCRE2,
    DEP_SQLITE3,
    DEP_COUNT  // Must be last
} DependencyType;

// Feature availability flags
typedef enum {
    FEATURE_GUI,
    FEATURE_3D_VISUALIZATION,
    FEATURE_TEXT_RENDERING,
    FEATURE_CODE_PARSING,
    FEATURE_JSON_CONFIG,
    FEATURE_REGEX,
    FEATURE_DATABASE,
    FEATURE_COUNT  // Must be last
} FeatureType;

// Dependency status structure
typedef struct {
    const char *name;
    const char *description;
    bool available;
    const char *version;  // NULL if not available
} DependencyInfo;

// Feature status structure
typedef struct {
    const char *name;
    const char *description;
    bool available;
    DependencyType required_deps[5];  // Max 5 dependencies per feature
    int dep_count;
} FeatureInfo;

/**
 * @brief Initialize the dependency manager
 *
 * This function detects all dependencies and populates the internal state.
 * Must be called before using any other dependency manager functions.
 *
 * @return CQ_SUCCESS on success, CQ_ERROR_* on failure
 */
CQError dependency_manager_init(void);

/**
 * @brief Shutdown the dependency manager
 *
 * Cleans up any resources used by the dependency manager.
 */
void dependency_manager_shutdown(void);

/**
 * @brief Check if a specific dependency is available
 *
 * @param dep The dependency to check
 * @return true if available, false otherwise
 */
bool dependency_is_available(DependencyType dep);

/**
 * @brief Get information about a specific dependency
 *
 * @param dep The dependency to query
 * @return Pointer to DependencyInfo structure, or NULL if invalid
 */
const DependencyInfo *dependency_get_info(DependencyType dep);

/**
 * @brief Check if a specific feature is available
 *
 * @param feature The feature to check
 * @return true if available, false otherwise
 */
bool feature_is_available(FeatureType feature);

/**
 * @brief Get information about a specific feature
 *
 * @param feature The feature to query
 * @return Pointer to FeatureInfo structure, or NULL if invalid
 */
const FeatureInfo *feature_get_info(FeatureType feature);

/**
 * @brief Get a list of missing dependencies for a feature
 *
 * @param feature The feature to check
 * @param missing_deps Array to store missing dependency types (output)
 * @param max_deps Maximum number of dependencies to store
 * @return Number of missing dependencies found
 */
int feature_get_missing_dependencies(FeatureType feature, DependencyType *missing_deps, int max_deps);

/**
 * @brief Print a summary of dependency status to console
 */
void dependency_print_status(void);

/**
 * @brief Get a human-readable description of missing features
 *
 * @param buffer Buffer to store the description
 * @param buffer_size Size of the buffer
 * @return CQ_SUCCESS on success, CQ_ERROR_* on failure
 */
CQError dependency_get_missing_features_description(char *buffer, size_t buffer_size);

/**
 * @brief Check if the application can run in CLI-only mode
 *
 * @return true if CLI mode is available, false otherwise
 */
bool dependency_can_run_cli_only(void);

/**
 * @brief Get the recommended run mode based on available dependencies
 *
 * @return String describing the recommended mode ("gui", "cli", "limited")
 */
const char *dependency_get_recommended_mode(void);

#endif // DEPENDENCY_MANAGER_H