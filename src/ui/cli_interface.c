#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "ui/cli_interface.h"
#include "utils/logger.h"

static struct option long_options[] = {
    {"project", required_argument, 0, 'p'},
    {"language", required_argument, 0, 'l'},
    {"output", required_argument, 0, 'o'},
    {"visualization", no_argument, 0, 'v'},
    {"no-visualization", no_argument, 0, 'V'},
    {"metrics", required_argument, 0, 'm'},
    {"verbosity", required_argument, 0, 'b'},
    {"config", required_argument, 0, 'c'},
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'e'},
    {"gui", no_argument, 0, 'g'},
    {0, 0, 0, 0}};

CQError parse_cli_args(int argc, char *argv[], CLIArgs *args)
{
    if (!args)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Initialize args structure
    memset(args, 0, sizeof(CLIArgs));
    args->language = LANG_UNKNOWN;
    args->enable_visualization = true;
    args->verbosity_level = 1;

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "p:l:o:vVm:b:c:heg",
                              long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'p':
            if (strlen(optarg) >= MAX_PATH_LENGTH)
            {
                LOG_ERROR("Project path too long");
                return CQ_ERROR_INVALID_ARGUMENT;
            }
            strcpy(args->project_path, optarg);
            break;

        case 'l':
            if (strcmp(optarg, "c") == 0 || strcmp(optarg, "C") == 0)
            {
                args->language = LANG_C;
            }
            else if (strcmp(optarg, "cpp") == 0 || strcmp(optarg, "C++") == 0)
            {
                args->language = LANG_CPP;
            }
            else if (strcmp(optarg, "java") == 0)
            {
                args->language = LANG_JAVA;
            }
            else if (strcmp(optarg, "python") == 0)
            {
                args->language = LANG_PYTHON;
            }
            else if (strcmp(optarg, "javascript") == 0 || strcmp(optarg, "js") == 0)
            {
                args->language = LANG_JAVASCRIPT;
            }
            else if (strcmp(optarg, "typescript") == 0 || strcmp(optarg, "ts") == 0)
            {
                args->language = LANG_TYPESCRIPT;
            }
            else
            {
                LOG_ERROR("Unsupported language: %s", optarg);
                return CQ_ERROR_INVALID_ARGUMENT;
            }
            break;

        case 'o':
            if (strlen(optarg) >= MAX_PATH_LENGTH)
            {
                LOG_ERROR("Output path too long");
                return CQ_ERROR_INVALID_ARGUMENT;
            }
            strcpy(args->output_path, optarg);
            break;

        case 'v':
            args->enable_visualization = true;
            break;

        case 'V':
            args->enable_visualization = false;
            break;

        case 'm':
            // Parse metrics string (comma-separated)
            {
                char *token = strtok(optarg, ",");
                while (token)
                {
                    if (strcmp(token, "complexity") == 0)
                    {
                        args->enable_metrics[0] = true;
                    }
                    else if (strcmp(token, "loc") == 0)
                    {
                        args->enable_metrics[1] = true;
                    }
                    else if (strcmp(token, "maintainability") == 0)
                    {
                        args->enable_metrics[2] = true;
                    }
                    else if (strcmp(token, "duplication") == 0)
                    {
                        args->enable_metrics[3] = true;
                    }
                    else if (strcmp(token, "halstead") == 0)
                    {
                        args->enable_metrics[4] = true;
                    }
                    token = strtok(NULL, ",");
                }
            }
            break;

        case 'b':
            args->verbosity_level = atoi(optarg);
            if (args->verbosity_level < 0 || args->verbosity_level > 3)
            {
                LOG_ERROR("Invalid verbosity level: %d", args->verbosity_level);
                return CQ_ERROR_INVALID_ARGUMENT;
            }
            break;

        case 'c':
            // Load configuration file
            // TODO: Implement config loading
            LOG_WARNING("Config file loading not yet implemented: %s", optarg);
            break;

        case 'h':
            args->show_help = true;
            break;

        case 'e':
            args->show_version = true;
            break;

        case 'g':
            args->use_gui = true;
            break;

        case '?':
            // getopt_long already printed an error message
            return CQ_ERROR_INVALID_ARGUMENT;

        default:
            LOG_ERROR("Unknown option: %c", opt);
            return CQ_ERROR_INVALID_ARGUMENT;
        }
    }

    // Check for non-option arguments
    if (optind < argc)
    {
        LOG_WARNING("Ignoring non-option arguments:");
        while (optind < argc)
        {
            LOG_WARNING("  %s", argv[optind++]);
        }
    }

    return CQ_SUCCESS;
}

void display_help(void)
{
    printf("CQAnalyzer v%s - Code Quality Analyzer with 3D Visualization\n", CQANALYZER_VERSION);
    printf("\n");
    display_usage();
    printf("\n");
    printf("OPTIONS:\n");
    printf("  -p, --project PATH       Path to the project directory to analyze (required)\n");
    printf("  -l, --language LANG      Programming language (c, cpp, java, python, javascript, typescript)\n");
    printf("  -o, --output PATH        Output directory for results and visualizations\n");
    printf("  -v, --visualization      Enable 3D visualization (default)\n");
    printf("  -V, --no-visualization   Disable 3D visualization\n");
    printf("  -m, --metrics LIST       Comma-separated list of metrics to compute\n");
    printf("                           Available: complexity, loc, maintainability, duplication, halstead\n");
    printf("  -b, --verbosity LEVEL    Set verbosity level (0-3, default: 1)\n");
    printf("  -c, --config FILE        Load configuration from file\n");
    printf("  -g, --gui                Launch graphical user interface\n");
    printf("  -h, --help               Display this help message\n");
    printf("  -e, --version            Display version information\n");
    printf("\n");
    printf("EXAMPLES:\n");
    printf("  cqanalyzer -p /path/to/project -l cpp -m complexity,loc\n");
    printf("  cqanalyzer -p /path/to/project -o /path/to/output --no-visualization\n");
    printf("  cqanalyzer --config myconfig.cfg -p /path/to/project\n");
    printf("\n");
    printf("For more information, visit: https://github.com/ChubbyChuckles/CQAnalyzer\n");
}

void display_usage(void)
{
    printf("Usage: cqanalyzer [OPTIONS]\n");
}
