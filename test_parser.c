#include <stdio.h>
#include <stdlib.h>
#include "cqanalyzer.h"
#include "parser/generic_parser.h"
#include "parser/language_support.h"
#include "parser/ast_parser.h"
#include "utils/logger.h"

int main()
{
    // Initialize logging
    if (logger_init() != 0) {
        fprintf(stderr, "Failed to initialize logging\n");
        return 1;
    }

    // Initialize parsers
    if (initialize_language_parsers() != CQ_SUCCESS) {
        LOG_ERROR("Failed to initialize parsers");
        logger_shutdown();
        return 1;
    }

    // Test language detection
    const char *test_files[] = {
        "test.py",
        "test.java",
        "test.c",
        "test.cpp",
        "test.js",
        "test.ts",
        NULL
    };

    for (int i = 0; test_files[i] != NULL; i++) {
        SupportedLanguage lang = detect_language(test_files[i]);
        printf("File: %s -> Language: %s\n", test_files[i], language_to_string(lang));

        // Try to parse
        void *ast_data = parse_source_file_with_detection(test_files[i]);
        if (ast_data) {
            printf("  Successfully parsed %s\n", test_files[i]);
            free_ast_data(ast_data);
        } else {
            printf("  Failed to parse %s\n", test_files[i]);
        }
    }

    // Cleanup
    shutdown_language_parsers();
    logger_shutdown();

    return 0;
}