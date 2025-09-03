/**
 * @file dependency_manager.c
 * @brief Implementation of dependency detection and graceful degradation system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Include headers for dependency detection
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <clang-c/Index.h>

#ifdef CJSON_FOUND
#include <cjson/cJSON.h>
#endif

#ifdef PCRE2_FOUND
#include <pcre2.h>
#endif

#ifdef SQLITE3_FOUND
#include <sqlite3.h>
#endif

#include "dependency_manager.h"
#include "utils/logger.h"

// Static arrays to store dependency and feature information
static DependencyInfo dependencies[DEP_COUNT];
static FeatureInfo features[FEATURE_COUNT];
static bool initialized = false;

// Forward declarations for detection functions
static bool detect_opengl(void);
static bool detect_glfw3(void);
static bool detect_glew(void);
static bool detect_glm(void);
static bool detect_freetype(void);
static bool detect_libclang(void);
static bool detect_cjson(void);
static bool detect_pcre2(void);
static bool detect_sqlite3(void);

static const char *get_opengl_version(void);
static const char *get_glfw3_version(void);
static const char *get_glew_version(void);
static const char *get_glm_version(void);
static const char *get_freetype_version(void);
static const char *get_libclang_version(void);
static const char *get_cjson_version(void);
static const char *get_pcre2_version(void);
static const char *get_sqlite3_version(void);

/**
 * Initialize dependency information
 */
static void init_dependency_info(void)
{
    // OpenGL
    dependencies[DEP_OPENGL].name = "OpenGL";
    dependencies[DEP_OPENGL].description = "3D graphics rendering library";
    dependencies[DEP_OPENGL].available = false;
    dependencies[DEP_OPENGL].version = NULL;

    // GLFW3
    dependencies[DEP_GLFW3].name = "GLFW3";
    dependencies[DEP_GLFW3].description = "Window and input management library";
    dependencies[DEP_GLFW3].available = false;
    dependencies[DEP_GLFW3].version = NULL;

    // GLEW
    dependencies[DEP_GLEW].name = "GLEW";
    dependencies[DEP_GLEW].description = "OpenGL extension loading library";
    dependencies[DEP_GLEW].available = false;
    dependencies[DEP_GLEW].version = NULL;

    // GLM
    dependencies[DEP_GLM].name = "GLM";
    dependencies[DEP_GLM].description = "OpenGL mathematics library";
    dependencies[DEP_GLM].available = false;
    dependencies[DEP_GLM].version = NULL;

    // FreeType
    dependencies[DEP_FREETYPE].name = "FreeType";
    dependencies[DEP_FREETYPE].description = "Font rendering library";
    dependencies[DEP_FREETYPE].available = false;
    dependencies[DEP_FREETYPE].version = NULL;

    // libclang
    dependencies[DEP_LIBCLANG].name = "libclang";
    dependencies[DEP_LIBCLANG].description = "Clang compiler frontend library";
    dependencies[DEP_LIBCLANG].available = false;
    dependencies[DEP_LIBCLANG].version = NULL;

    // cJSON
    dependencies[DEP_CJSON].name = "cJSON";
    dependencies[DEP_CJSON].description = "JSON parsing library";
    dependencies[DEP_CJSON].available = false;
    dependencies[DEP_CJSON].version = NULL;

    // PCRE2
    dependencies[DEP_PCRE2].name = "PCRE2";
    dependencies[DEP_PCRE2].description = "Regular expression library";
    dependencies[DEP_PCRE2].available = false;
    dependencies[DEP_PCRE2].version = NULL;

    // SQLite3
    dependencies[DEP_SQLITE3].name = "SQLite3";
    dependencies[DEP_SQLITE3].description = "Embedded database library";
    dependencies[DEP_SQLITE3].available = false;
    dependencies[DEP_SQLITE3].version = NULL;
}

/**
 * Initialize feature information
 */
static void init_feature_info(void)
{
    // GUI Feature
    features[FEATURE_GUI].name = "GUI Mode";
    features[FEATURE_GUI].description = "Graphical user interface with 3D visualization";
    features[FEATURE_GUI].available = false;
    features[FEATURE_GUI].required_deps[0] = DEP_OPENGL;
    features[FEATURE_GUI].required_deps[1] = DEP_GLFW3;
    features[FEATURE_GUI].required_deps[2] = DEP_GLEW;
    features[FEATURE_GUI].dep_count = 3;

    // 3D Visualization Feature
    features[FEATURE_3D_VISUALIZATION].name = "3D Visualization";
    features[FEATURE_3D_VISUALIZATION].description = "3D rendering and visualization capabilities";
    features[FEATURE_3D_VISUALIZATION].available = false;
    features[FEATURE_3D_VISUALIZATION].required_deps[0] = DEP_OPENGL;
    features[FEATURE_3D_VISUALIZATION].required_deps[1] = DEP_GLFW3;
    features[FEATURE_3D_VISUALIZATION].required_deps[2] = DEP_GLEW;
    features[FEATURE_3D_VISUALIZATION].required_deps[3] = DEP_GLM;
    features[FEATURE_3D_VISUALIZATION].dep_count = 4;

    // Text Rendering Feature
    features[FEATURE_TEXT_RENDERING].name = "Text Rendering";
    features[FEATURE_TEXT_RENDERING].description = "Font rendering for UI and labels";
    features[FEATURE_TEXT_RENDERING].available = false;
    features[FEATURE_TEXT_RENDERING].required_deps[0] = DEP_FREETYPE;
    features[FEATURE_TEXT_RENDERING].dep_count = 1;

    // Code Parsing Feature
    features[FEATURE_CODE_PARSING].name = "Code Parsing";
    features[FEATURE_CODE_PARSING].description = "Source code analysis and parsing";
    features[FEATURE_CODE_PARSING].available = false;
    features[FEATURE_CODE_PARSING].required_deps[0] = DEP_LIBCLANG;
    features[FEATURE_CODE_PARSING].dep_count = 1;

    // JSON Config Feature
    features[FEATURE_JSON_CONFIG].name = "JSON Configuration";
    features[FEATURE_JSON_CONFIG].description = "JSON-based configuration files";
    features[FEATURE_JSON_CONFIG].available = false;
    features[FEATURE_JSON_CONFIG].required_deps[0] = DEP_CJSON;
    features[FEATURE_JSON_CONFIG].dep_count = 1;

    // Regex Feature
    features[FEATURE_REGEX].name = "Regular Expressions";
    features[FEATURE_REGEX].description = "Pattern matching and text processing";
    features[FEATURE_REGEX].available = false;
    features[FEATURE_REGEX].required_deps[0] = DEP_PCRE2;
    features[FEATURE_REGEX].dep_count = 1;

    // Database Feature
    features[FEATURE_DATABASE].name = "Database Support";
    features[FEATURE_DATABASE].description = "Persistent data storage and querying";
    features[FEATURE_DATABASE].available = false;
    features[FEATURE_DATABASE].required_deps[0] = DEP_SQLITE3;
    features[FEATURE_DATABASE].dep_count = 1;
}

/**
 * Detect all dependencies
 */
static void detect_all_dependencies(void)
{
    LOG_INFO("Detecting system dependencies...");

    // Detect each dependency
    dependencies[DEP_OPENGL].available = detect_opengl();
    if (dependencies[DEP_OPENGL].available) {
        dependencies[DEP_OPENGL].version = get_opengl_version();
    }

    dependencies[DEP_GLFW3].available = detect_glfw3();
    if (dependencies[DEP_GLFW3].available) {
        dependencies[DEP_GLFW3].version = get_glfw3_version();
    }

    dependencies[DEP_GLEW].available = detect_glew();
    if (dependencies[DEP_GLEW].available) {
        dependencies[DEP_GLEW].version = get_glew_version();
    }

    dependencies[DEP_GLM].available = detect_glm();
    if (dependencies[DEP_GLM].available) {
        dependencies[DEP_GLM].version = get_glm_version();
    }

    dependencies[DEP_FREETYPE].available = detect_freetype();
    if (dependencies[DEP_FREETYPE].available) {
        dependencies[DEP_FREETYPE].version = get_freetype_version();
    }

    dependencies[DEP_LIBCLANG].available = detect_libclang();
    if (dependencies[DEP_LIBCLANG].available) {
        dependencies[DEP_LIBCLANG].version = get_libclang_version();
    }

    dependencies[DEP_CJSON].available = detect_cjson();
    if (dependencies[DEP_CJSON].available) {
        dependencies[DEP_CJSON].version = get_cjson_version();
    }

    dependencies[DEP_PCRE2].available = detect_pcre2();
    if (dependencies[DEP_PCRE2].available) {
        dependencies[DEP_PCRE2].version = get_pcre2_version();
    }

    dependencies[DEP_SQLITE3].available = detect_sqlite3();
    if (dependencies[DEP_SQLITE3].available) {
        dependencies[DEP_SQLITE3].version = get_sqlite3_version();
    }

    LOG_INFO("Dependency detection completed");
}

/**
 * Update feature availability based on dependencies
 */
static void update_feature_availability(void)
{
    for (int i = 0; i < FEATURE_COUNT; i++) {
        bool all_deps_available = true;

        for (int j = 0; j < features[i].dep_count; j++) {
            if (!dependencies[features[i].required_deps[j]].available) {
                all_deps_available = false;
                break;
            }
        }

        features[i].available = all_deps_available;
    }
}

CQError dependency_manager_init(void)
{
    if (initialized) {
        return CQ_SUCCESS;
    }

    LOG_INFO("Initializing dependency manager...");

    // Initialize dependency and feature information
    init_dependency_info();
    init_feature_info();

    // Detect all dependencies
    detect_all_dependencies();

    // Update feature availability
    update_feature_availability();

    initialized = true;
    LOG_INFO("Dependency manager initialized successfully");

    return CQ_SUCCESS;
}

void dependency_manager_shutdown(void)
{
    if (!initialized) {
        return;
    }

    LOG_INFO("Shutting down dependency manager...");
    initialized = false;
    LOG_INFO("Dependency manager shutdown complete");
}

bool dependency_is_available(DependencyType dep)
{
    if (!initialized || dep >= DEP_COUNT) {
        return false;
    }

    return dependencies[dep].available;
}

const DependencyInfo *dependency_get_info(DependencyType dep)
{
    if (!initialized || dep >= DEP_COUNT) {
        return NULL;
    }

    return &dependencies[dep];
}

bool feature_is_available(FeatureType feature)
{
    if (!initialized || feature >= FEATURE_COUNT) {
        return false;
    }

    return features[feature].available;
}

const FeatureInfo *feature_get_info(FeatureType feature)
{
    if (!initialized || feature >= FEATURE_COUNT) {
        return NULL;
    }

    return &features[feature];
}

int feature_get_missing_dependencies(FeatureType feature, DependencyType *missing_deps, int max_deps)
{
    if (!initialized || feature >= FEATURE_COUNT || !missing_deps || max_deps <= 0) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < features[feature].dep_count && count < max_deps; i++) {
        DependencyType dep = features[feature].required_deps[i];
        if (!dependencies[dep].available) {
            missing_deps[count++] = dep;
        }
    }

    return count;
}

void dependency_print_status(void)
{
    if (!initialized) {
        printf("Dependency manager not initialized\n");
        return;
    }

    printf("\n=== CQAnalyzer Dependency Status ===\n");

    for (int i = 0; i < DEP_COUNT; i++) {
        const DependencyInfo *info = &dependencies[i];
        printf("%-12s: %s", info->name,
               info->available ? "Available" : "Not Available");

        if (info->version) {
            printf(" (%s)", info->version);
        }

        if (info->available) {
            printf(" ✓");
        } else {
            printf(" ✗");
        }
        printf("\n");
    }

    printf("\n=== Feature Availability ===\n");

    for (int i = 0; i < FEATURE_COUNT; i++) {
        const FeatureInfo *info = &features[i];
        printf("%-18s: %s", info->name,
               info->available ? "Available" : "Not Available");

        if (info->available) {
            printf(" ✓");
        } else {
            printf(" ✗");
        }
        printf("\n");
    }

    printf("===================================\n\n");
}

CQError dependency_get_missing_features_description(char *buffer, size_t buffer_size)
{
    if (!initialized || !buffer || buffer_size == 0) {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    buffer[0] = '\0';
    size_t offset = 0;

    for (int i = 0; i < FEATURE_COUNT; i++) {
        if (!features[i].available) {
            size_t needed = strlen(features[i].name) + 4; // " - \n\0"

            if (offset + needed >= buffer_size) {
                // Truncate if buffer is too small
                if (offset < buffer_size - 4) {
                    strcpy(buffer + offset, "...");
                }
                break;
            }

            if (offset > 0) {
                strcpy(buffer + offset, "\n");
                offset += 1;
            }

            strcpy(buffer + offset, features[i].name);
            offset += strlen(features[i].name);
            strcpy(buffer + offset, " - ");
            offset += 3;
            strcpy(buffer + offset, features[i].description);
            offset += strlen(features[i].description);
        }
    }

    return CQ_SUCCESS;
}

bool dependency_can_run_cli_only(void)
{
    if (!initialized) {
        return false;
    }

    // CLI mode requires at least libclang for code parsing
    return dependencies[DEP_LIBCLANG].available;
}

const char *dependency_get_recommended_mode(void)
{
    if (!initialized) {
        return "unknown";
    }

    if (feature_is_available(FEATURE_GUI)) {
        return "gui";
    } else if (dependency_can_run_cli_only()) {
        return "cli";
    } else {
        return "limited";
    }
}

// Detection function implementations
static bool detect_opengl(void)
{
#ifdef __APPLE__
    return true; // OpenGL is always available on macOS
#else
    // Try to create a dummy OpenGL context to test availability
    // This is a simplified check - in practice, you'd use glxinfo or similar
    return true; // Assume available for now
#endif
}

static bool detect_glfw3(void)
{
    // Try to initialize GLFW
    if (glfwInit() == GLFW_TRUE) {
        glfwTerminate();
        return true;
    }
    return false;
}

static bool detect_glew(void)
{
    // GLEW detection is tricky without a context
    // We'll assume it's available if GLFW is available
    return detect_glfw3();
}

static bool detect_glm(void)
{
    // GLM is header-only, so we check if the headers are available
    // This is a compile-time check, so we'll assume it's available
    return true;
}

static bool detect_freetype(void)
{
    // Try to initialize FreeType
    FT_Library library;
    if (FT_Init_FreeType(&library) == 0) {
        FT_Done_FreeType(library);
        return true;
    }
    return false;
}

static bool detect_libclang(void)
{
    // Try to create a clang index
    CXIndex index = clang_createIndex(0, 0);
    if (index) {
        clang_disposeIndex(index);
        return true;
    }
    return false;
}

static bool detect_cjson(void)
{
    // Check if cJSON functions are available
    // This would typically be done at compile time with preprocessor checks
#ifdef CJSON_FOUND
    return true;
#else
    return false;
#endif
}

static bool detect_pcre2(void)
{
    // Check if PCRE2 functions are available
#ifdef PCRE2_FOUND
    return true;
#else
    return false;
#endif
}

static bool detect_sqlite3(void)
{
    // Check if SQLite3 functions are available
#ifdef SQLITE3_FOUND
    return true;
#else
    return false;
#endif
}

// Version detection functions
static const char *get_opengl_version(void)
{
    return "OpenGL 3.3+"; // Simplified
}

static const char *get_glfw3_version(void)
{
    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d", major, minor, rev);
    return version;
}

static const char *get_glew_version(void)
{
    return glewGetString(GLEW_VERSION);
}

static const char *get_glm_version(void)
{
    return "0.9.9+"; // GLM version
}

static const char *get_freetype_version(void)
{
    static char version[32];
    FT_Library library;
    if (FT_Init_FreeType(&library) == 0) {
        int major, minor, patch;
        FT_Library_Version(library, &major, &minor, &patch);
        snprintf(version, sizeof(version), "%d.%d.%d", major, minor, patch);
        FT_Done_FreeType(library);
        return version;
    }
    return "Unknown";
}

static const char *get_libclang_version(void)
{
    return clang_getClangVersion();
}

static const char *get_cjson_version(void)
{
    return "1.7.15"; // cJSON version
}

static const char *get_pcre2_version(void)
{
    return "10.42"; // PCRE2 version
}

static const char *get_sqlite3_version(void)
{
    return sqlite3_libversion();
}