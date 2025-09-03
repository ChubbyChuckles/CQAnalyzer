#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "cqanalyzer.h"
#include "utils/logger.h"
#include "utils/config.h"
#include "utils/dependency_manager.h"
#include "ui/progress_display.h"
#include "parser/generic_parser.h"
#include "visualizer/renderer.h"
#include "ui/input_handler.h"

/**
 * @brief Main entry point for CQAnalyzer GUI
 *
 * This function initializes the application and launches the GUI
 * with the menu system and 3D visualization.
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return Exit status (0 for success, non-zero for error)
 */
int main_gui(int argc, char *argv[])
{
    // Initialize logging system
    if (logger_init() != 0)
    {
        fprintf(stderr, "Failed to initialize logging system\n");
        return EXIT_FAILURE;
    }

    LOG_INFO("CQAnalyzer GUI starting up...");

    // Check if GUI dependencies are available
    if (!feature_is_available(FEATURE_GUI))
    {
        LOG_ERROR("GUI dependencies are not available. Cannot start GUI mode.");
        LOG_ERROR("Missing dependencies for GUI:");
        DependencyType missing_deps[10];
        int num_missing = feature_get_missing_dependencies(FEATURE_GUI, missing_deps, 10);
        for (int i = 0; i < num_missing; i++) {
            const DependencyInfo *dep_info = dependency_get_info(missing_deps[i]);
            if (dep_info) {
                LOG_ERROR("  - %s: %s", dep_info->name, dep_info->description);
            }
        }
        LOG_ERROR("Please install the missing dependencies and try again.");
        logger_shutdown();
        return EXIT_FAILURE;
    }

    // Initialize configuration system
    if (config_init() != 0)
    {
        LOG_ERROR("Failed to initialize configuration system");
        logger_shutdown();
        return EXIT_FAILURE;
    }

    // Try to load configuration from file
    const char *config_files[] = {"cqanalyzer.conf", ".cqanalyzer.conf", NULL};

    for (int i = 0; config_files[i] != NULL; i++)
    {
        if (config_load_from_file(config_files[i]) == CQ_SUCCESS)
        {
            LOG_INFO("Loaded configuration from: %s", config_files[i]);
            break;
        }
    }

    // Initialize progress display
    if (progress_display_init() != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to initialize progress display");
        config_shutdown();
        logger_shutdown();
        return EXIT_FAILURE;
    }

    // Initialize language parsers
    if (initialize_language_parsers() != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to initialize language parsers");
        progress_display_shutdown();
        config_shutdown();
        logger_shutdown();
        return EXIT_FAILURE;
    }

    // Initialize input handler
    if (input_handler_init() != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to initialize input handler");
        shutdown_language_parsers();
        progress_display_shutdown();
        config_shutdown();
        logger_shutdown();
        return EXIT_FAILURE;
    }

    // Initialize renderer with GUI window
    if (renderer_init(1200, 800, "CQAnalyzer - Code Quality Analyzer") != CQ_SUCCESS)
    {
        LOG_ERROR("Failed to initialize renderer");
        input_handler_shutdown();
        shutdown_language_parsers();
        progress_display_shutdown();
        config_shutdown();
        logger_shutdown();
        return EXIT_FAILURE;
    }

    LOG_INFO("CQAnalyzer GUI initialized successfully");

    // Main application loop
    while (renderer_is_running())
    {
        // Update input handler
        input_handler_update();

        // Update renderer (handles camera, input, etc.)
        renderer_update();

        // Render frame
        renderer_render();

        // Present frame
        renderer_present();
    }

    // Shutdown
    LOG_INFO("Shutting down CQAnalyzer GUI...");

    renderer_shutdown();
    input_handler_shutdown();
    shutdown_language_parsers();
    progress_display_shutdown();
    config_shutdown();
    logger_shutdown();

    LOG_INFO("CQAnalyzer GUI shutdown complete");
    return EXIT_SUCCESS;
}