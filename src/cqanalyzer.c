#include "cqanalyzer.h"

/**
 * @brief Convert error code to string
 */
const char *cq_error_to_string(CQError error)
{
    switch (error)
    {
    case CQ_SUCCESS:
        return "Success";
    case CQ_ERROR_INVALID_ARGUMENT:
        return "Invalid argument";
    case CQ_ERROR_FILE_NOT_FOUND:
        return "File not found";
    case CQ_ERROR_MEMORY_ALLOCATION:
        return "Memory allocation failed";
    case CQ_ERROR_PARSING_FAILED:
        return "Parsing failed";
    case CQ_ERROR_ANALYSIS_FAILED:
        return "Analysis failed";
    case CQ_ERROR_RENDERING_FAILED:
        return "Rendering failed";
    case CQ_ERROR_CONFIG_INVALID:
        return "Configuration invalid";
    case CQ_ERROR_UNKNOWN:
    default:
        return "Unknown error";
    }
}

/**
 * @brief Convert language enum to string
 */
const char *language_to_string(SupportedLanguage lang)
{
    switch (lang)
    {
    case LANG_C:
        return "C";
    case LANG_CPP:
        return "C++";
    case LANG_JAVA:
        return "Java";
    case LANG_PYTHON:
        return "Python";
    case LANG_JAVASCRIPT:
        return "JavaScript";
    case LANG_TYPESCRIPT:
        return "TypeScript";
    case LANG_UNKNOWN:
    default:
        return "Unknown";
    }
}