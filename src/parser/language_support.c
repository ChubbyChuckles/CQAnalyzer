#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "parser/language_support.h"
#include "utils/logger.h"

// File extensions for each language
static const char* c_extensions[] = {".c", ".h", NULL};
static const char* cpp_extensions[] = {".cpp", ".cc", ".cxx", ".c++", ".hpp", ".hh", ".hxx", ".h++", NULL};
static const char* java_extensions[] = {".java", NULL};
static const char* python_extensions[] = {".py", ".pyw", NULL};
static const char* javascript_extensions[] = {".js", ".mjs", NULL};
static const char* typescript_extensions[] = {".ts", ".tsx", NULL};

SupportedLanguage detect_language(const char* filename) {
    if (!filename) {
        return LANG_UNKNOWN;
    }

    // Find the last dot in the filename
    const char* ext = strrchr(filename, '.');
    if (!ext) {
        return LANG_UNKNOWN;
    }

    // Convert to lowercase for case-insensitive comparison
    char ext_lower[10];
    size_t ext_len = strlen(ext);
    if (ext_len >= sizeof(ext_lower)) {
        return LANG_UNKNOWN;
    }

    for (size_t i = 0; i < ext_len; i++) {
        ext_lower[i] = tolower(ext[i]);
    }
    ext_lower[ext_len] = '\0';

    // Check against known extensions
    if (strcmp(ext_lower, ".c") == 0 || strcmp(ext_lower, ".h") == 0) {
        return LANG_C;
    } else if (strcmp(ext_lower, ".cpp") == 0 || strcmp(ext_lower, ".cc") == 0 ||
               strcmp(ext_lower, ".cxx") == 0 || strcmp(ext_lower, ".c++") == 0 ||
               strcmp(ext_lower, ".hpp") == 0 || strcmp(ext_lower, ".hh") == 0 ||
               strcmp(ext_lower, ".hxx") == 0 || strcmp(ext_lower, ".h++") == 0) {
        return LANG_CPP;
    } else if (strcmp(ext_lower, ".java") == 0) {
        return LANG_JAVA;
    } else if (strcmp(ext_lower, ".py") == 0 || strcmp(ext_lower, ".pyw") == 0) {
        return LANG_PYTHON;
    } else if (strcmp(ext_lower, ".js") == 0 || strcmp(ext_lower, ".mjs") == 0) {
        return LANG_JAVASCRIPT;
    } else if (strcmp(ext_lower, ".ts") == 0 || strcmp(ext_lower, ".tsx") == 0) {
        return LANG_TYPESCRIPT;
    }

    return LANG_UNKNOWN;
}

const char** get_language_extensions(SupportedLanguage language) {
    switch (language) {
        case LANG_C:
            return c_extensions;
        case LANG_CPP:
            return cpp_extensions;
        case LANG_JAVA:
            return java_extensions;
        case LANG_PYTHON:
            return python_extensions;
        case LANG_JAVASCRIPT:
            return javascript_extensions;
        case LANG_TYPESCRIPT:
            return typescript_extensions;
        default:
            return NULL;
    }
}

bool is_language_supported(SupportedLanguage language) {
    return language != LANG_UNKNOWN;
}
