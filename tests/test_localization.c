#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../include/utils/localization.h"
#include "../include/cqanalyzer.h"

/**
 * @brief Test localization initialization
 */
void test_localization_init(void)
{
    printf("Testing localization initialization...\n");

    // Test initial state
    assert(localization_get_current_language() == UI_LANG_EN);
    assert(localization_is_language_loaded(UI_LANG_EN) == true);

    printf("✓ Localization initialization test passed\n");
}

/**
 * @brief Test language switching
 */
void test_language_switching(void)
{
    printf("Testing language switching...\n");

    // Test switching to German
    CQError result = localization_set_language(UI_LANG_DE);
    assert(result == CQ_SUCCESS);
    assert(localization_get_current_language() == UI_LANG_DE);
    assert(localization_is_language_loaded(UI_LANG_DE) == true);

    // Test switching back to English
    result = localization_set_language(UI_LANG_EN);
    assert(result == CQ_SUCCESS);
    assert(localization_get_current_language() == UI_LANG_EN);

    printf("✓ Language switching test passed\n");
}

/**
 * @brief Test message retrieval
 */
void test_message_retrieval(void)
{
    printf("Testing message retrieval...\n");

    // Test English messages
    const char *msg = localization_get_message("error.success");
    assert(msg != NULL);
    assert(strcmp(msg, "Success") == 0);

    msg = localization_get_message("error.invalid_argument");
    assert(msg != NULL);
    assert(strcmp(msg, "Invalid argument provided") == 0);

    // Test German messages
    localization_set_language(UI_LANG_DE);
    msg = localization_get_message("error.success");
    assert(msg != NULL);
    assert(strcmp(msg, "Erfolg") == 0);

    msg = localization_get_message("error.invalid_argument");
    assert(msg != NULL);
    assert(strcmp(msg, "Ungültiges Argument bereitgestellt") == 0);

    // Switch back to English
    localization_set_language(UI_LANG_EN);

    printf("✓ Message retrieval test passed\n");
}

/**
 * @brief Test error message localization
 */
void test_error_message_localization(void)
{
    printf("Testing error message localization...\n");

    // Test error code to message conversion
    const char *msg = localization_get_error_message(1001); // CQ_ERROR_INVALID_ARGUMENT_CODE
    assert(msg != NULL);
    assert(strcmp(msg, "Invalid argument provided") == 0);

    // Test with German
    localization_set_language(UI_LANG_DE);
    msg = localization_get_error_message(1001);
    assert(msg != NULL);
    assert(strcmp(msg, "Ungültiges Argument bereitgestellt") == 0);

    // Switch back to English
    localization_set_language(UI_LANG_EN);

    printf("✓ Error message localization test passed\n");
}

/**
 * @brief Test fallback to English
 */
void test_fallback_to_english(void)
{
    printf("Testing fallback to English...\n");

    // Test with German - should fallback to English for missing messages
    localization_set_language(UI_LANG_DE);
    const char *msg = localization_get_message("nonexistent.key");
    assert(msg != NULL);
    assert(strcmp(msg, "nonexistent.key") == 0); // Should return the key itself

    // Switch back to English
    localization_set_language(UI_LANG_EN);

    printf("✓ Fallback to English test passed\n");
}

/**
 * @brief Test language utilities
 */
void test_language_utilities(void)
{
    printf("Testing language utilities...\n");

    // Test language code conversion
    const char *code = localization_get_language_code(UI_LANG_EN);
    assert(code != NULL);
    assert(strcmp(code, "en") == 0);

    code = localization_get_language_code(UI_LANG_DE);
    assert(code != NULL);
    assert(strcmp(code, "de") == 0);

    // Test language from code conversion
    UILanguage lang = localization_get_language_from_code("en");
    assert(lang == UI_LANG_EN);

    lang = localization_get_language_from_code("de");
    assert(lang == UI_LANG_DE);

    lang = localization_get_language_from_code("invalid");
    assert(lang == UI_LANG_EN); // Should default to English

    // Test language name
    const char *name = localization_get_language_name(UI_LANG_EN);
    assert(name != NULL);
    assert(strcmp(name, "English") == 0);

    name = localization_get_language_name(UI_LANG_DE);
    assert(name != NULL);
    assert(strcmp(name, "Deutsch") == 0);

    printf("✓ Language utilities test passed\n");
}

/**
 * @brief Test available languages
 */
void test_available_languages(void)
{
    printf("Testing available languages...\n");

    UILanguage languages[16];
    size_t count = localization_get_available_languages(languages, 16);

    assert(count > 0);
    assert(count <= 16);

    // Should include English and German
    bool found_en = false, found_de = false;
    for (size_t i = 0; i < count; i++)
    {
        if (languages[i] == UI_LANG_EN) found_en = true;
        if (languages[i] == UI_LANG_DE) found_de = true;
    }

    assert(found_en == true);
    assert(found_de == true);

    printf("✓ Available languages test passed\n");
}

/**
 * @brief Test message formatting
 */
void test_message_formatting(void)
{
    printf("Testing message formatting...\n");

    // Create a test message key that supports formatting
    // Note: This would require adding a test message to the catalogs
    char buffer[256];

    // Test with a simple message
    int result = localization_format_message("error.success", buffer, sizeof(buffer));
    assert(result > 0);
    assert(strcmp(buffer, "Success") == 0);

    printf("✓ Message formatting test passed\n");
}

/**
 * @brief Main test function
 */
int main(void)
{
    printf("Running localization tests...\n\n");

    // Initialize localization
    CQError init_result = localization_init();
    if (init_result != CQ_SUCCESS)
    {
        printf("Failed to initialize localization system\n");
        return 1;
    }

    // Run tests
    test_localization_init();
    test_language_switching();
    test_message_retrieval();
    test_error_message_localization();
    test_fallback_to_english();
    test_language_utilities();
    test_available_languages();
    test_message_formatting();

    // Cleanup
    localization_shutdown();

    printf("\n✓ All localization tests passed!\n");
    return 0;
}