#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <stdbool.h>
#include <stdint.h>
#include "cqanalyzer.h"

/**
 * @file localization.h
 * @brief Internationalization and localization framework for CQAnalyzer
 *
 * This module provides comprehensive localization support with:
 * - Multiple language support
 * - Message catalogs for error messages and UI strings
 * - Fallback to English for missing translations
 * - Runtime language switching
 * - Integration with error handling system
 */

#define MAX_LANGUAGE_CODE_LENGTH 8
#define MAX_MESSAGE_KEY_LENGTH 128
#define MAX_LOCALIZED_MESSAGE_LENGTH 512
#define MAX_SUPPORTED_LANGUAGES 16

// Supported UI languages
typedef enum
{
    UI_LANG_EN = 0,    // English (default/fallback)
    UI_LANG_DE = 1,    // German
    UI_LANG_FR = 2,    // French
    UI_LANG_ES = 3,    // Spanish
    UI_LANG_IT = 4,    // Italian
    UI_LANG_PT = 5,    // Portuguese
    UI_LANG_RU = 6,    // Russian
    UI_LANG_JA = 7,    // Japanese
    UI_LANG_ZH = 8,    // Chinese
    UI_LANG_KO = 9,    // Korean
    UI_LANG_AR = 10,   // Arabic
    UI_LANG_HI = 11,   // Hindi
    UI_LANG_COUNT      // Number of supported languages
} UILanguage;

// Message categories for organization
typedef enum
{
    MSG_CATEGORY_ERROR = 0,
    MSG_CATEGORY_WARNING = 1,
    MSG_CATEGORY_INFO = 2,
    MSG_CATEGORY_UI = 3,
    MSG_CATEGORY_CONFIG = 4,
    MSG_CATEGORY_SYSTEM = 5,
    MSG_CATEGORY_COUNT
} MessageCategory;

// Message key structure
typedef struct
{
    char key[MAX_MESSAGE_KEY_LENGTH];
    MessageCategory category;
    uint32_t id;  // Unique identifier for the message
} MessageKey;

// Localized message entry
typedef struct
{
    MessageKey key;
    char message[MAX_LOCALIZED_MESSAGE_LENGTH];
} LocalizedMessage;

// Language info structure
typedef struct
{
    UILanguage code;
    char language_code[MAX_LANGUAGE_CODE_LENGTH];
    char display_name[MAX_LOCALIZED_MESSAGE_LENGTH];
    bool loaded;
} LanguageInfo;

// Localization context
typedef struct
{
    UILanguage current_language;
    LanguageInfo languages[MAX_SUPPORTED_LANGUAGES];
    LocalizedMessage *messages[UI_LANG_COUNT];
    size_t message_counts[UI_LANG_COUNT];
    bool initialized;
} LocalizationContext;

/**
 * @brief Initialize the localization system
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError localization_init(void);

/**
 * @brief Shutdown the localization system
 */
void localization_shutdown(void);

/**
 * @brief Load language data for a specific language
 *
 * @param language Language to load
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError localization_load_language(UILanguage language);

/**
 * @brief Set the current language
 *
 * @param language Language to set as current
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError localization_set_language(UILanguage language);

/**
 * @brief Get the current language
 *
 * @return Current language code
 */
UILanguage localization_get_current_language(void);

/**
 * @brief Get localized message for a message key
 *
 * @param key Message key
 * @return Localized message string, or English fallback if not found
 */
const char *localization_get_message(const char *key);

/**
 * @brief Get localized message for error code
 *
 * @param error_code Error code
 * @return Localized error message
 */
const char *localization_get_error_message(int error_code);

/**
 * @brief Get localized message for error code with category
 *
 * @param error_code Error code
 * @param category Message category
 * @return Localized error message
 */
const char *localization_get_error_message_categorized(int error_code, MessageCategory category);

/**
 * @brief Get language display name
 *
 * @param language Language code
 * @return Display name in current language
 */
const char *localization_get_language_name(UILanguage language);

/**
 * @brief Get list of available languages
 *
 * @param languages Array to fill with language codes
 * @param max_count Maximum number of languages to return
 * @return Number of languages returned
 */
size_t localization_get_available_languages(UILanguage *languages, size_t max_count);

/**
 * @brief Check if a language is loaded
 *
 * @param language Language to check
 * @return true if loaded, false otherwise
 */
bool localization_is_language_loaded(UILanguage language);

/**
 * @brief Get language code string from enum
 *
 * @param language Language enum value
 * @return Language code string (e.g., "en", "de")
 */
const char *localization_get_language_code(UILanguage language);

/**
 * @brief Get language enum from code string
 *
 * @param code Language code string
 * @return Language enum value, or UI_LANG_EN if not found
 */
UILanguage localization_get_language_from_code(const char *code);

/**
 * @brief Format a localized message with parameters
 *
 * @param key Message key
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @param ... Format arguments
 * @return Number of characters written, or -1 on error
 */
int localization_format_message(const char *key, char *buffer, size_t buffer_size, ...);

/**
 * @brief Reload all language data
 *
 * @return CQ_SUCCESS on success, error code on failure
 */
CQError localization_reload_languages(void);

// Convenience macros
#define LOCALIZE(key) localization_get_message(key)
#define LOCALIZE_ERROR(code) localization_get_error_message(code)
#define LOCALIZE_FORMAT(key, buffer, size, ...) localization_format_message(key, buffer, size, ##__VA_ARGS__)

#endif // LOCALIZATION_H