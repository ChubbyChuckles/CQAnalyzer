#ifndef CLI_INTERFACE_H
#define CLI_INTERFACE_H

#include "cqanalyzer.h"

/**
 * @file cli_interface.h
 * @brief Command-line interface for CQAnalyzer
 *
 * Provides functionality to parse command line arguments and display help
 * information for the CQAnalyzer application.
 */

/**
 * @brief Parse command line arguments
 *
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param args Pointer to CLIArgs structure to fill
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError parse_cli_args(int argc, char *argv[], CLIArgs *args);

/**
 * @brief Display help information
 */
void display_help(void);

/**
 * @brief Display usage information
 */
void display_usage(void);

#endif // CLI_INTERFACE_H
