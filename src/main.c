#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "cqanalyzer.h"
#include "utils/logger.h"
#include "utils/config.h"
#include "ui/cli_interface.h"
#include "ui/progress_display.h"
#include "parser/generic_parser.h"

/**
 * @brief Main entry point for CQAnalyzer
 *
 * This function initializes the application, parses command line arguments,
 * and orchestrates the code analysis and visualization process.
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return Exit status (0 for success, non-zero for error)
 */
int main(int argc, char *argv[])
{
    // Initialize logging system
    if (logger_init() != 0)
    {
        fprintf(stderr, "Failed to initialize logging system\n");
        return EXIT_FAILURE;
    }

    LOG_INFO("CQAnalyzer starting up...");

    // Initialize configuration system
    if (config_init() != 0)
    {
        LOG_ERROR("Failed to initialize configuration system");
        logger_shutdown();
        return EXIT_FAILURE;
    }

    // Try to load configuration from file
    const char *config_files[] = {"cqanalyzer.conf", ".cqanalyzer.conf", NULL};
    bool config_loaded = false;

    for (int i = 0; config_files[i] != NULL; i++)
    {
        if (config_load_from_file(config_files[i]) == CQ_SUCCESS)
        {
            LOG_INFO("Loaded configuration from: %s", config_files[i]);
            config_loaded = true;
            break;
        }
    }

    if (!config_loaded)
    {
        LOG_INFO("Using default configuration (no config file found)");
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

    // Parse command line arguments
    CLIArgs args;
    if (parse_cli_args(argc, argv, &args) != 0)
    {
        LOG_ERROR("Failed to parse command line arguments");
        shutdown_language_parsers();
        progress_display_shutdown();
        config_shutdown();
        logger_shutdown();
        return EXIT_FAILURE;
    }

    // Display version information if requested
    if (args.show_version)
    {
        printf("CQAnalyzer v%s\n", CQANALYZER_VERSION);
        printf("Code Quality Analyzer with 3D Visualization\n");
        shutdown_language_parsers();
        progress_display_shutdown();
        config_shutdown();
        logger_shutdown();
        return EXIT_SUCCESS;
    }

    // Display help if requested
    if (args.show_help)
    {
        display_help();
        shutdown_language_parsers();
        progress_display_shutdown();
        config_shutdown();
        logger_shutdown();
        return EXIT_SUCCESS;
    }

    // Validate arguments
    if (!args.project_path)
    {
        LOG_ERROR("Project path is required. Use -p or --project to specify.");
        display_help();
        shutdown_language_parsers();
        progress_display_shutdown();
        config_shutdown();
        logger_shutdown();
        return EXIT_FAILURE;
    }

    LOG_INFO("Analyzing project: %s", args.project_path);

    // Start progress tracking for the entire analysis pipeline
    progress_start("CQAnalyzer Project Analysis", 3); // 3 main phases

    // Phase 1: Parse the project
    progress_update(1, "Parsing project files...");
    void *project_ast = parse_project(args.project_path, 1000, (void (*)(int, int, const char *))progress_update);
    if (!project_ast)
    {
        LOG_ERROR("Failed to parse project");
        progress_display_error("Project parsing failed");
        shutdown_language_parsers();
        progress_display_shutdown();
        config_shutdown();
        logger_shutdown();
        return EXIT_FAILURE;
    }

    // Phase 2: Analyze code metrics (placeholder)
    progress_update(2, "Analyzing code metrics...");
    LOG_INFO("Code analysis phase - placeholder implementation");

    // Phase 3: Generate visualization data (placeholder)
    progress_update(3, "Generating visualization data...");
    LOG_INFO("Visualization generation phase - placeholder implementation");

    // Complete progress
    progress_complete("Analysis completed successfully");

    // TODO: In full implementation, pass project_ast to analyzer and visualizer modules
    // For now, just free the allocated memory
    if (project_ast)
    {
        // Free project AST (simplified cleanup)
        free(project_ast);
    }

    // Cleanup
    shutdown_language_parsers();
    progress_display_shutdown();
    config_shutdown();
    logger_shutdown();

    LOG_INFO("CQAnalyzer shutdown complete");
    return EXIT_SUCCESS;
}
