#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "utils/localization.h"
#include "utils/logger.h"

// Global localization context
static LocalizationContext g_localization_ctx = {0};

// Language information
static const LanguageInfo g_language_info[] = {
    {UI_LANG_EN, "en", "English", false},
    {UI_LANG_DE, "de", "Deutsch", false},
    {UI_LANG_FR, "fr", "Français", false},
    {UI_LANG_ES, "es", "Español", false},
    {UI_LANG_IT, "it", "Italiano", false},
    {UI_LANG_PT, "pt", "Português", false},
    {UI_LANG_RU, "ru", "Русский", false},
    {UI_LANG_JA, "ja", "日本語", false},
    {UI_LANG_ZH, "zh", "中文", false},
    {UI_LANG_KO, "ko", "한국어", false},
    {UI_LANG_AR, "ar", "العربية", false},
    {UI_LANG_HI, "hi", "हिन्दी", false}
};

// English (default) message catalog
static const LocalizedMessage g_english_messages[] = {
    // Error messages
    {{"error.success", MSG_CATEGORY_ERROR, 0}, "Success"},
    {{"error.invalid_argument", MSG_CATEGORY_ERROR, 1001}, "Invalid argument provided"},
    {{"error.null_pointer", MSG_CATEGORY_ERROR, 1002}, "Null pointer encountered"},
    {{"error.buffer_overflow", MSG_CATEGORY_ERROR, 1003}, "Buffer overflow detected"},
    {{"error.invalid_state", MSG_CATEGORY_ERROR, 1004}, "Invalid system state"},
    {{"error.operation_not_supported", MSG_CATEGORY_ERROR, 1005}, "Operation not supported"},

    // Parsing errors
    {{"error.parsing_failed", MSG_CATEGORY_ERROR, 2001}, "Code parsing failed"},
    {{"error.syntax_error", MSG_CATEGORY_ERROR, 2002}, "Syntax error in source code"},
    {{"error.unsupported_language", MSG_CATEGORY_ERROR, 2003}, "Unsupported programming language"},
    {{"error.file_format_invalid", MSG_CATEGORY_ERROR, 2004}, "Invalid file format"},
    {{"error.encoding_unsupported", MSG_CATEGORY_ERROR, 2005}, "Unsupported text encoding"},
    {{"error.parser_initialization_failed", MSG_CATEGORY_ERROR, 2006}, "Parser initialization failed"},
    {{"error.ast_generation_failed", MSG_CATEGORY_ERROR, 2007}, "AST generation failed"},
    {{"error.tokenization_failed", MSG_CATEGORY_ERROR, 2008}, "Source code tokenization failed"},

    // Analysis errors
    {{"error.analysis_failed", MSG_CATEGORY_ERROR, 3001}, "Code analysis failed"},
    {{"error.metric_calculation_failed", MSG_CATEGORY_ERROR, 3002}, "Metric calculation failed"},
    {{"error.complexity_analysis_failed", MSG_CATEGORY_ERROR, 3003}, "Complexity analysis failed"},
    {{"error.dependency_analysis_failed", MSG_CATEGORY_ERROR, 3004}, "Dependency analysis failed"},
    {{"error.code_quality_assessment_failed", MSG_CATEGORY_ERROR, 3005}, "Code quality assessment failed"},
    {{"error.dead_code_detection_failed", MSG_CATEGORY_ERROR, 3006}, "Dead code detection failed"},
    {{"error.duplication_detection_failed", MSG_CATEGORY_ERROR, 3007}, "Code duplication detection failed"},

    // Visualization errors
    {{"error.rendering_failed", MSG_CATEGORY_ERROR, 4001}, "3D rendering failed"},
    {{"error.opengl_initialization_failed", MSG_CATEGORY_ERROR, 4002}, "OpenGL initialization failed"},
    {{"error.shader_compilation_failed", MSG_CATEGORY_ERROR, 4003}, "Shader compilation failed"},
    {{"error.texture_loading_failed", MSG_CATEGORY_ERROR, 4004}, "Texture loading failed"},
    {{"error.camera_setup_failed", MSG_CATEGORY_ERROR, 4005}, "Camera setup failed"},
    {{"error.data_transformation_failed", MSG_CATEGORY_ERROR, 4006}, "Data transformation failed"},
    {{"error.visualization_data_invalid", MSG_CATEGORY_ERROR, 4007}, "Invalid visualization data"},

    // UI errors
    {{"error.ui_initialization_failed", MSG_CATEGORY_ERROR, 5001}, "UI initialization failed"},
    {{"error.window_creation_failed", MSG_CATEGORY_ERROR, 5002}, "Window creation failed"},
    {{"error.gui_library_error", MSG_CATEGORY_ERROR, 5003}, "GUI library error"},
    {{"error.input_handling_failed", MSG_CATEGORY_ERROR, 5004}, "Input handling failed"},
    {{"error.dialog_creation_failed", MSG_CATEGORY_ERROR, 5005}, "Dialog creation failed"},

    // Configuration errors
    {{"error.config_invalid", MSG_CATEGORY_ERROR, 6001}, "Invalid configuration"},
    {{"error.config_file_not_found", MSG_CATEGORY_ERROR, 6002}, "Configuration file not found"},
    {{"error.config_parsing_failed", MSG_CATEGORY_ERROR, 6003}, "Configuration parsing failed"},
    {{"error.config_value_invalid", MSG_CATEGORY_ERROR, 6004}, "Invalid configuration value"},
    {{"error.config_save_failed", MSG_CATEGORY_ERROR, 6005}, "Configuration save failed"},

    // I/O errors
    {{"error.file_not_found", MSG_CATEGORY_ERROR, 7001}, "File not found"},
    {{"error.file_access_denied", MSG_CATEGORY_ERROR, 7002}, "File access denied"},
    {{"error.file_read_failed", MSG_CATEGORY_ERROR, 7003}, "File read failed"},
    {{"error.file_write_failed", MSG_CATEGORY_ERROR, 7004}, "File write failed"},
    {{"error.directory_not_found", MSG_CATEGORY_ERROR, 7005}, "Directory not found"},
    {{"error.path_too_long", MSG_CATEGORY_ERROR, 7006}, "Path too long"},
    {{"error.disk_full", MSG_CATEGORY_ERROR, 7007}, "Disk full"},

    // Memory errors
    {{"error.memory_allocation", MSG_CATEGORY_ERROR, 8001}, "Memory allocation failed"},
    {{"error.memory_corruption", MSG_CATEGORY_ERROR, 8002}, "Memory corruption detected"},
    {{"error.out_of_memory", MSG_CATEGORY_ERROR, 8003}, "Out of memory"},
    {{"error.memory_leak_detected", MSG_CATEGORY_ERROR, 8004}, "Memory leak detected"},

    // System errors
    {{"error.system_call_failed", MSG_CATEGORY_ERROR, 9001}, "System call failed"},
    {{"error.library_not_found", MSG_CATEGORY_ERROR, 9002}, "Required library not found"},
    {{"error.dependency_missing", MSG_CATEGORY_ERROR, 9003}, "Missing dependency"},
    {{"error.permission_denied", MSG_CATEGORY_ERROR, 9004}, "Permission denied"},
    {{"error.resource_busy", MSG_CATEGORY_ERROR, 9005}, "Resource busy"},
    {{"error.timeout", MSG_CATEGORY_ERROR, 9006}, "Operation timeout"},

    // UI strings
    {{"ui.language", MSG_CATEGORY_UI, 1}, "Language"},
    {{"ui.settings", MSG_CATEGORY_UI, 2}, "Settings"},
    {{"ui.help", MSG_CATEGORY_UI, 3}, "Help"},
    {{"ui.about", MSG_CATEGORY_UI, 4}, "About"},
    {{"ui.exit", MSG_CATEGORY_UI, 5}, "Exit"},
    {{"ui.file", MSG_CATEGORY_UI, 6}, "File"},
    {{"ui.open", MSG_CATEGORY_UI, 7}, "Open"},
    {{"ui.save", MSG_CATEGORY_UI, 8}, "Save"},
    {{"ui.analyze", MSG_CATEGORY_UI, 9}, "Analyze"},
    {{"ui.visualize", MSG_CATEGORY_UI, 10}, "Visualize"}
};

// German message catalog
static const LocalizedMessage g_german_messages[] = {
    // Error messages
    {{"error.success", MSG_CATEGORY_ERROR, 0}, "Erfolg"},
    {{"error.invalid_argument", MSG_CATEGORY_ERROR, 1001}, "Ungültiges Argument bereitgestellt"},
    {{"error.null_pointer", MSG_CATEGORY_ERROR, 1002}, "Null-Zeiger gefunden"},
    {{"error.buffer_overflow", MSG_CATEGORY_ERROR, 1003}, "Pufferüberlauf erkannt"},
    {{"error.invalid_state", MSG_CATEGORY_ERROR, 1004}, "Ungültiger Systemzustand"},
    {{"error.operation_not_supported", MSG_CATEGORY_ERROR, 1005}, "Operation nicht unterstützt"},

    // Parsing errors
    {{"error.parsing_failed", MSG_CATEGORY_ERROR, 2001}, "Code-Parsing fehlgeschlagen"},
    {{"error.syntax_error", MSG_CATEGORY_ERROR, 2002}, "Syntaxfehler im Quellcode"},
    {{"error.unsupported_language", MSG_CATEGORY_ERROR, 2003}, "Nicht unterstützte Programmiersprache"},
    {{"error.file_format_invalid", MSG_CATEGORY_ERROR, 2004}, "Ungültiges Dateiformat"},
    {{"error.encoding_unsupported", MSG_CATEGORY_ERROR, 2005}, "Nicht unterstützte Textkodierung"},
    {{"error.parser_initialization_failed", MSG_CATEGORY_ERROR, 2006}, "Parser-Initialisierung fehlgeschlagen"},
    {{"error.ast_generation_failed", MSG_CATEGORY_ERROR, 2007}, "AST-Generierung fehlgeschlagen"},
    {{"error.tokenization_failed", MSG_CATEGORY_ERROR, 2008}, "Quellcode-Tokenisierung fehlgeschlagen"},

    // UI strings
    {{"ui.language", MSG_CATEGORY_UI, 1}, "Sprache"},
    {{"ui.settings", MSG_CATEGORY_UI, 2}, "Einstellungen"},
    {{"ui.help", MSG_CATEGORY_UI, 3}, "Hilfe"},
    {{"ui.about", MSG_CATEGORY_UI, 4}, "Über"},
    {{"ui.exit", MSG_CATEGORY_UI, 5}, "Beenden"},
    {{"ui.file", MSG_CATEGORY_UI, 6}, "Datei"},
    {{"ui.open", MSG_CATEGORY_UI, 7}, "Öffnen"},
    {{"ui.save", MSG_CATEGORY_UI, 8}, "Speichern"},
    {{"ui.analyze", MSG_CATEGORY_UI, 9}, "Analysieren"},
    {{"ui.visualize", MSG_CATEGORY_UI, 10}, "Visualisieren"}
};

// French message catalog
static const LocalizedMessage g_french_messages[] = {
    // Error messages
    {{"error.success", MSG_CATEGORY_ERROR, 0}, "Succès"},
    {{"error.invalid_argument", MSG_CATEGORY_ERROR, 1001}, "Argument invalide fourni"},
    {{"error.null_pointer", MSG_CATEGORY_ERROR, 1002}, "Pointeur nul rencontré"},
    {{"error.buffer_overflow", MSG_CATEGORY_ERROR, 1003}, "Dépassement de tampon détecté"},
    {{"error.invalid_state", MSG_CATEGORY_ERROR, 1004}, "État système invalide"},
    {{"error.operation_not_supported", MSG_CATEGORY_ERROR, 1005}, "Opération non supportée"},

    // UI strings
    {{"ui.language", MSG_CATEGORY_UI, 1}, "Langue"},
    {{"ui.settings", MSG_CATEGORY_UI, 2}, "Paramètres"},
    {{"ui.help", MSG_CATEGORY_UI, 3}, "Aide"},
    {{"ui.about", MSG_CATEGORY_UI, 4}, "À propos"},
    {{"ui.exit", MSG_CATEGORY_UI, 5}, "Quitter"},
    {{"ui.file", MSG_CATEGORY_UI, 6}, "Fichier"},
    {{"ui.open", MSG_CATEGORY_UI, 7}, "Ouvrir"},
    {{"ui.save", MSG_CATEGORY_UI, 8}, "Enregistrer"},
    {{"ui.analyze", MSG_CATEGORY_UI, 9}, "Analyser"},
    {{"ui.visualize", MSG_CATEGORY_UI, 10}, "Visualiser"}
};

// Spanish message catalog
static const LocalizedMessage g_spanish_messages[] = {
    // Error messages
    {{"error.success", MSG_CATEGORY_ERROR, 0}, "Éxito"},
    {{"error.invalid_argument", MSG_CATEGORY_ERROR, 1001}, "Argumento inválido proporcionado"},
    {{"error.null_pointer", MSG_CATEGORY_ERROR, 1002}, "Puntero nulo encontrado"},
    {{"error.buffer_overflow", MSG_CATEGORY_ERROR, 1003}, "Desbordamiento de búfer detectado"},
    {{"error.invalid_state", MSG_CATEGORY_ERROR, 1004}, "Estado del sistema inválido"},
    {{"error.operation_not_supported", MSG_CATEGORY_ERROR, 1005}, "Operación no soportada"},

    // UI strings
    {{"ui.language", MSG_CATEGORY_UI, 1}, "Idioma"},
    {{"ui.settings", MSG_CATEGORY_UI, 2}, "Configuración"},
    {{"ui.help", MSG_CATEGORY_UI, 3}, "Ayuda"},
    {{"ui.about", MSG_CATEGORY_UI, 4}, "Acerca de"},
    {{"ui.exit", MSG_CATEGORY_UI, 5}, "Salir"},
    {{"ui.file", MSG_CATEGORY_UI, 6}, "Archivo"},
    {{"ui.open", MSG_CATEGORY_UI, 7}, "Abrir"},
    {{"ui.save", MSG_CATEGORY_UI, 8}, "Guardar"},
    {{"ui.analyze", MSG_CATEGORY_UI, 9}, "Analizar"},
    {{"ui.visualize", MSG_CATEGORY_UI, 10}, "Visualizar"}
};

// Function to get message catalog for a language
static const LocalizedMessage *get_message_catalog(UILanguage language, size_t *count)
{
    switch (language)
    {
    case UI_LANG_EN:
        *count = sizeof(g_english_messages) / sizeof(g_english_messages[0]);
        return g_english_messages;
    case UI_LANG_DE:
        *count = sizeof(g_german_messages) / sizeof(g_german_messages[0]);
        return g_german_messages;
    case UI_LANG_FR:
        *count = sizeof(g_french_messages) / sizeof(g_french_messages[0]);
        return g_french_messages;
    case UI_LANG_ES:
        *count = sizeof(g_spanish_messages) / sizeof(g_spanish_messages[0]);
        return g_spanish_messages;
    default:
        *count = sizeof(g_english_messages) / sizeof(g_english_messages[0]);
        return g_english_messages;
    }
}

// Binary search for message by key
static const LocalizedMessage *find_message_by_key(const LocalizedMessage *messages, size_t count, const char *key)
{
    size_t left = 0;
    size_t right = count - 1;

    while (left <= right)
    {
        size_t mid = left + (right - left) / 2;
        int cmp = strcmp(messages[mid].key.key, key);

        if (cmp == 0)
        {
            return &messages[mid];
        }
        else if (cmp < 0)
        {
            left = mid + 1;
        }
        else
        {
            if (mid == 0) break;
            right = mid - 1;
        }
    }

    return NULL;
}

// Binary search for message by ID
static const LocalizedMessage *find_message_by_id(const LocalizedMessage *messages, size_t count, uint32_t id)
{
    size_t left = 0;
    size_t right = count - 1;

    while (left <= right)
    {
        size_t mid = left + (right - left) / 2;

        if (messages[mid].key.id == id)
        {
            return &messages[mid];
        }
        else if (messages[mid].key.id < id)
        {
            left = mid + 1;
        }
        else
        {
            if (mid == 0) break;
            right = mid - 1;
        }
    }

    return NULL;
}

CQError localization_init(void)
{
    if (g_localization_ctx.initialized)
    {
        return CQ_SUCCESS;
    }

    // Initialize language info
    memcpy(g_localization_ctx.languages, g_language_info, sizeof(g_language_info));

    // Load English by default
    CQError result = localization_load_language(UI_LANG_EN);
    if (result != CQ_SUCCESS)
    {
        return result;
    }

    // Set English as default language
    g_localization_ctx.current_language = UI_LANG_EN;
    g_localization_ctx.initialized = true;

    LOG_INFO("Localization system initialized with %d supported languages", UI_LANG_COUNT);
    return CQ_SUCCESS;
}

void localization_shutdown(void)
{
    if (!g_localization_ctx.initialized)
    {
        return;
    }

    // Free allocated message arrays
    for (int i = 0; i < UI_LANG_COUNT; i++)
    {
        if (g_localization_ctx.messages[i])
        {
            free(g_localization_ctx.messages[i]);
            g_localization_ctx.messages[i] = NULL;
            g_localization_ctx.message_counts[i] = 0;
        }
        g_localization_ctx.languages[i].loaded = false;
    }

    g_localization_ctx.initialized = false;
    LOG_INFO("Localization system shutdown");
}

CQError localization_load_language(UILanguage language)
{
    if (language >= UI_LANG_COUNT)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (g_localization_ctx.languages[language].loaded)
    {
        return CQ_SUCCESS; // Already loaded
    }

    size_t count;
    const LocalizedMessage *catalog = get_message_catalog(language, &count);

    if (!catalog || count == 0)
    {
        LOG_WARNING("No message catalog available for language %d", language);
        return CQ_ERROR_FILE_NOT_FOUND;
    }

    // Allocate memory for messages
    LocalizedMessage *messages = (LocalizedMessage *)malloc(count * sizeof(LocalizedMessage));
    if (!messages)
    {
        return CQ_ERROR_MEMORY_ALLOCATION;
    }

    // Copy messages
    memcpy(messages, catalog, count * sizeof(LocalizedMessage));

    // Store in context
    g_localization_ctx.messages[language] = messages;
    g_localization_ctx.message_counts[language] = count;
    g_localization_ctx.languages[language].loaded = true;

    LOG_INFO("Loaded %zu messages for language %s", count,
             g_localization_ctx.languages[language].language_code);

    return CQ_SUCCESS;
}

CQError localization_set_language(UILanguage language)
{
    if (!g_localization_ctx.initialized)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    if (language >= UI_LANG_COUNT)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    // Load language if not already loaded
    if (!g_localization_ctx.languages[language].loaded)
    {
        CQError result = localization_load_language(language);
        if (result != CQ_SUCCESS)
        {
            return result;
        }
    }

    g_localization_ctx.current_language = language;
    LOG_INFO("Language switched to %s", g_localization_ctx.languages[language].language_code);

    return CQ_SUCCESS;
}

UILanguage localization_get_current_language(void)
{
    return g_localization_ctx.current_language;
}

const char *localization_get_message(const char *key)
{
    if (!g_localization_ctx.initialized || !key)
    {
        return "Localization not initialized";
    }

    UILanguage lang = g_localization_ctx.current_language;

    // Try current language first
    if (g_localization_ctx.messages[lang])
    {
        const LocalizedMessage *msg = find_message_by_key(
            g_localization_ctx.messages[lang],
            g_localization_ctx.message_counts[lang],
            key
        );

        if (msg)
        {
            return msg->message;
        }
    }

    // Fallback to English
    if (lang != UI_LANG_EN && g_localization_ctx.messages[UI_LANG_EN])
    {
        const LocalizedMessage *msg = find_message_by_key(
            g_localization_ctx.messages[UI_LANG_EN],
            g_localization_ctx.message_counts[UI_LANG_EN],
            key
        );

        if (msg)
        {
            return msg->message;
        }
    }

    // Return key if not found
    return key;
}

const char *localization_get_error_message(int error_code)
{
    return localization_get_error_message_categorized(error_code, MSG_CATEGORY_ERROR);
}

const char *localization_get_error_message_categorized(int error_code, MessageCategory category)
{
    if (!g_localization_ctx.initialized)
    {
        return "Localization not initialized";
    }

    UILanguage lang = g_localization_ctx.current_language;

    // Try current language first
    if (g_localization_ctx.messages[lang])
    {
        const LocalizedMessage *msg = find_message_by_id(
            g_localization_ctx.messages[lang],
            g_localization_ctx.message_counts[lang],
            (uint32_t)error_code
        );

        if (msg)
        {
            return msg->message;
        }
    }

    // Fallback to English
    if (lang != UI_LANG_EN && g_localization_ctx.messages[UI_LANG_EN])
    {
        const LocalizedMessage *msg = find_message_by_id(
            g_localization_ctx.messages[UI_LANG_EN],
            g_localization_ctx.message_counts[UI_LANG_EN],
            (uint32_t)error_code
        );

        if (msg)
        {
            return msg->message;
        }
    }

    // Return generic message
    static char buffer[64];
    snprintf(buffer, sizeof(buffer), "Error %d", error_code);
    return buffer;
}

const char *localization_get_language_name(UILanguage language)
{
    if (language >= UI_LANG_COUNT)
    {
        return "Unknown";
    }

    return g_localization_ctx.languages[language].display_name;
}

size_t localization_get_available_languages(UILanguage *languages, size_t max_count)
{
    size_t count = 0;

    for (int i = 0; i < UI_LANG_COUNT && count < max_count; i++)
    {
        if (languages)
        {
            languages[count] = (UILanguage)i;
        }
        count++;
    }

    return count;
}

bool localization_is_language_loaded(UILanguage language)
{
    if (language >= UI_LANG_COUNT)
    {
        return false;
    }

    return g_localization_ctx.languages[language].loaded;
}

const char *localization_get_language_code(UILanguage language)
{
    if (language >= UI_LANG_COUNT)
    {
        return "unknown";
    }

    return g_localization_ctx.languages[language].language_code;
}

UILanguage localization_get_language_from_code(const char *code)
{
    if (!code)
    {
        return UI_LANG_EN;
    }

    for (int i = 0; i < UI_LANG_COUNT; i++)
    {
        if (strcmp(g_localization_ctx.languages[i].language_code, code) == 0)
        {
            return (UILanguage)i;
        }
    }

    return UI_LANG_EN; // Default fallback
}

int localization_format_message(const char *key, char *buffer, size_t buffer_size, ...)
{
    if (!buffer || buffer_size == 0)
    {
        return -1;
    }

    const char *template = localization_get_message(key);
    if (!template)
    {
        return -1;
    }

    va_list args;
    va_start(args, buffer_size);
    int result = vsnprintf(buffer, buffer_size, template, args);
    va_end(args);

    return result;
}

CQError localization_reload_languages(void)
{
    if (!g_localization_ctx.initialized)
    {
        return CQ_ERROR_INVALID_ARGUMENT;
    }

    LOG_INFO("Reloading all languages...");

    // Reload all loaded languages
    for (int i = 0; i < UI_LANG_COUNT; i++)
    {
        if (g_localization_ctx.languages[i].loaded)
        {
            // Free current messages
            if (g_localization_ctx.messages[i])
            {
                free(g_localization_ctx.messages[i]);
                g_localization_ctx.messages[i] = NULL;
                g_localization_ctx.message_counts[i] = 0;
            }

            g_localization_ctx.languages[i].loaded = false;

            // Reload
            localization_load_language((UILanguage)i);
        }
    }

    return CQ_SUCCESS;
}