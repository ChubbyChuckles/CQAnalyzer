#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui/theme_manager.h"

/**
 * @brief Test theme manager initialization
 */
void test_theme_manager_init(void)
{
    ThemeManager manager;

    CU_ASSERT_TRUE(theme_manager_init(&manager));
    CU_ASSERT_EQUAL(theme_manager_get_theme_count(&manager), 6); // 6 predefined themes

    // Check that predefined themes exist
    CU_ASSERT_STRING_EQUAL(theme_manager_get_theme_name(&manager, 0), "Dark");
    CU_ASSERT_STRING_EQUAL(theme_manager_get_theme_name(&manager, 1), "Light");
    CU_ASSERT_STRING_EQUAL(theme_manager_get_theme_name(&manager, 2), "Classic");
    CU_ASSERT_STRING_EQUAL(theme_manager_get_theme_name(&manager, 3), "Modern");
    CU_ASSERT_STRING_EQUAL(theme_manager_get_theme_name(&manager, 4), "High Contrast");
    CU_ASSERT_STRING_EQUAL(theme_manager_get_theme_name(&manager, 5), "Minimal");

    theme_manager_shutdown(&manager);
}

/**
 * @brief Test theme switching functionality
 */
void test_theme_manager_switching(void)
{
    ThemeManager manager;

    CU_ASSERT_TRUE(theme_manager_init(&manager));

    // Test initial theme
    CU_ASSERT_EQUAL(manager.current_theme, 0);
    CU_ASSERT_STRING_EQUAL(theme_manager_get_current_theme_name(&manager), "Dark");

    // Test theme switching
    CU_ASSERT_TRUE(theme_manager_apply_theme(&manager, 1));
    CU_ASSERT_EQUAL(manager.current_theme, 1);
    CU_ASSERT_STRING_EQUAL(theme_manager_get_current_theme_name(&manager), "Light");

    // Test invalid theme index
    CU_ASSERT_FALSE(theme_manager_apply_theme(&manager, 999));
    CU_ASSERT_EQUAL(manager.current_theme, 1); // Should remain unchanged

    theme_manager_shutdown(&manager);
}

/**
 * @brief Test custom theme creation
 */
void test_theme_manager_custom_theme(void)
{
    ThemeManager manager;

    CU_ASSERT_TRUE(theme_manager_init(&manager));
    int initial_count = theme_manager_get_theme_count(&manager);

    // Create a custom theme
    CU_ASSERT_TRUE(theme_manager_create_theme(&manager, "Custom Test", &ImGui::GetStyle()));
    CU_ASSERT_EQUAL(theme_manager_get_theme_count(&manager), initial_count + 1);

    // Find the custom theme
    int custom_index = theme_manager_find_theme(&manager, "Custom Test");
    CU_ASSERT_TRUE(custom_index >= 0);
    CU_ASSERT_TRUE(manager.themes[custom_index].is_custom);

    // Test switching to custom theme
    CU_ASSERT_TRUE(theme_manager_apply_theme(&manager, custom_index));
    CU_ASSERT_EQUAL(manager.current_theme, custom_index);

    theme_manager_shutdown(&manager);
}

/**
 * @brief Test theme deletion
 */
void test_theme_manager_delete_theme(void)
{
    ThemeManager manager;

    CU_ASSERT_TRUE(theme_manager_init(&manager));

    // Create a custom theme first
    CU_ASSERT_TRUE(theme_manager_create_theme(&manager, "To Delete", &ImGui::GetStyle()));
    int count_after_create = theme_manager_get_theme_count(&manager);

    int delete_index = theme_manager_find_theme(&manager, "To Delete");
    CU_ASSERT_TRUE(delete_index >= 0);

    // Delete the custom theme
    CU_ASSERT_TRUE(theme_manager_delete_theme(&manager, delete_index));
    CU_ASSERT_EQUAL(theme_manager_get_theme_count(&manager), count_after_create - 1);

    // Try to delete a predefined theme (should fail)
    CU_ASSERT_FALSE(theme_manager_delete_theme(&manager, 0)); // Dark theme

    theme_manager_shutdown(&manager);
}

/**
 * @brief Test theme validation
 */
void test_theme_manager_validation(void)
{
    ImGuiTheme valid_theme;
    strcpy(valid_theme.name, "Valid Theme");
    valid_theme.is_custom = true;

    ImGuiTheme invalid_theme1 = {0}; // Empty name
    ImGuiTheme invalid_theme2;
    strcpy(invalid_theme2.name, ""); // Empty name
    invalid_theme2.is_custom = true;

    CU_ASSERT_TRUE(theme_manager_validate_theme(&valid_theme));
    CU_ASSERT_FALSE(theme_manager_validate_theme(&invalid_theme1));
    CU_ASSERT_FALSE(theme_manager_validate_theme(&invalid_theme2));
}

/**
 * @brief Test theme persistence (save/load)
 */
void test_theme_manager_persistence(void)
{
    ThemeManager manager1, manager2;
    const char* test_file = "test_themes.ini";

    CU_ASSERT_TRUE(theme_manager_init(&manager1));

    // Create a custom theme
    CU_ASSERT_TRUE(theme_manager_create_theme(&manager1, "Persistent Test", &ImGui::GetStyle()));

    // Save themes
    CU_ASSERT_TRUE(theme_manager_save_themes(&manager1, test_file));

    // Load themes into another manager
    CU_ASSERT_TRUE(theme_manager_init(&manager2));
    CU_ASSERT_TRUE(theme_manager_load_themes(&manager2, test_file));

    // Check that custom theme was loaded
    int loaded_index = theme_manager_find_theme(&manager2, "Persistent Test");
    CU_ASSERT_TRUE(loaded_index >= 0);

    theme_manager_shutdown(&manager1);
    theme_manager_shutdown(&manager2);

    // Clean up test file
    remove(test_file);
}

/**
 * @brief Test theme navigation (next/previous)
 */
void test_theme_manager_navigation(void)
{
    ThemeManager manager;

    CU_ASSERT_TRUE(theme_manager_init(&manager));

    // Test next theme
    int initial_theme = manager.current_theme;
    theme_manager_next_theme(&manager);
    CU_ASSERT_EQUAL(manager.current_theme, (initial_theme + 1) % theme_manager_get_theme_count(&manager));

    // Test previous theme
    theme_manager_previous_theme(&manager);
    CU_ASSERT_EQUAL(manager.current_theme, initial_theme);

    theme_manager_shutdown(&manager);
}

/**
 * @brief Test theme manager with NULL parameters
 */
void test_theme_manager_null_params(void)
{
    // Test with NULL manager
    CU_ASSERT_FALSE(theme_manager_init(NULL));
    CU_ASSERT_FALSE(theme_manager_apply_theme(NULL, 0));
    CU_ASSERT_FALSE(theme_manager_create_theme(NULL, "test", NULL));
    CU_ASSERT_FALSE(theme_manager_delete_theme(NULL, 0));
    CU_ASSERT_EQUAL(theme_manager_find_theme(NULL, "test"), -1);
    CU_ASSERT_PTR_NULL(theme_manager_get_current_theme_name(NULL));
    CU_ASSERT_EQUAL(theme_manager_get_theme_count(NULL), 0);
    CU_ASSERT_PTR_NULL(theme_manager_get_theme_name(NULL, 0));
}

/**
 * @brief Add theme manager tests to suite
 */
void add_theme_manager_tests(CU_pSuite suite)
{
    CU_add_test(suite, "Theme Manager Init Test", test_theme_manager_init);
    CU_add_test(suite, "Theme Manager Switching Test", test_theme_manager_switching);
    CU_add_test(suite, "Theme Manager Custom Theme Test", test_theme_manager_custom_theme);
    CU_add_test(suite, "Theme Manager Delete Theme Test", test_theme_manager_delete_theme);
    CU_add_test(suite, "Theme Manager Validation Test", test_theme_manager_validation);
    CU_add_test(suite, "Theme Manager Persistence Test", test_theme_manager_persistence);
    CU_add_test(suite, "Theme Manager Navigation Test", test_theme_manager_navigation);
    CU_add_test(suite, "Theme Manager Null Params Test", test_theme_manager_null_params);
}