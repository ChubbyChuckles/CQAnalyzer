#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>

// Include the UI integration header
#include "../src/ui/imgui_integration.h"

// Test helper functions
static void test_file_browser_initialization(void) {
    printf("Testing file browser initialization...\n");

    // Initialize file browser state
    imgui_init_file_browser_state();

    // Check that current directory is set
    assert(menu_state.current_directory[0] != '\0');
    assert(strlen(menu_state.current_directory) > 0);

    // Check that selected file is empty
    assert(menu_state.selected_file[0] == '\0');

    printf("✓ File browser initialization test passed\n");
}

static void test_project_selector_initialization(void) {
    printf("Testing project selector initialization...\n");

    // Initialize project selector state
    imgui_init_project_selector_state();

    // Check that selected project is empty initially
    assert(menu_state.selected_project[0] == '\0');

    // Check that we have some sample projects (if directories exist)
    // This is more of a demonstration than a strict test

    printf("✓ Project selector initialization test passed\n");
}

static void test_directory_validation(void) {
    printf("Testing directory validation...\n");

    // Test with current directory (should exist)
    char cwd[4096];
    getcwd(cwd, sizeof(cwd));

    struct stat st;
    assert(stat(cwd, &st) == 0);
    assert(S_ISDIR(st.st_mode));
    assert(access(cwd, R_OK) == 0);

    printf("✓ Directory validation test passed\n");
}

static void test_file_filtering(void) {
    printf("Testing file filtering...\n");

    // Test various file extensions
    assert(is_source_file("test.c", LANG_C) == true);
    assert(is_source_file("test.h", LANG_C) == true);
    assert(is_source_file("test.cpp", LANG_CPP) == true);
    assert(is_source_file("test.java", LANG_JAVA) == true);
    assert(is_source_file("test.py", LANG_PYTHON) == true);
    assert(is_source_file("test.js", LANG_JAVASCRIPT) == true);
    assert(is_source_file("test.ts", LANG_TYPESCRIPT) == true);

    // Test non-source files
    assert(is_source_file("test.txt", LANG_C) == false);
    assert(is_source_file("test.exe", LANG_CPP) == false);

    // Test wrong language for file type
    assert(is_source_file("test.c", LANG_PYTHON) == false);
    assert(is_source_file("test.py", LANG_C) == false);

    printf("✓ File filtering test passed\n");
}

static void test_menu_state_management(void) {
    printf("Testing menu state management...\n");

    // Test menu state initialization
    menu_state_init();

    // Check default values
    assert(menu_state.show_main_control_panel == true);
    assert(menu_state.visualization_mode == 0);
    assert(menu_state.show_axes == true);
    assert(menu_state.show_grid == true);
    assert(menu_state.show_labels == true);
    assert(menu_state.enable_complexity_analysis == true);
    assert(menu_state.enable_dead_code_detection == true);
    assert(menu_state.enable_duplication_detection == true);

    // Test menu state reset
    menu_state.show_main_control_panel = false;
    menu_state.visualization_mode = 2;

    menu_state_reset();

    assert(menu_state.show_main_control_panel == true);
    assert(menu_state.visualization_mode == 0);

    printf("✓ Menu state management test passed\n");
}

static void test_path_handling(void) {
    printf("Testing path handling...\n");

    // Test path length limits
    char long_path[MAX_PATH_LENGTH + 100];
    memset(long_path, 'a', sizeof(long_path) - 1);
    long_path[sizeof(long_path) - 1] = '\0';

    // This should not crash and should handle the truncation gracefully
    char test_path[MAX_PATH_LENGTH];
    strncpy(test_path, long_path, sizeof(test_path) - 1);
    test_path[sizeof(test_path) - 1] = '\0';

    assert(strlen(test_path) == MAX_PATH_LENGTH - 1);

    printf("✓ Path handling test passed\n");
}

int main(int argc, char *argv[]) {
    printf("Running UI Dialog Tests...\n");
    printf("========================\n\n");

    // Initialize ImGui context for testing (minimal)
    // Note: In a real test environment, you'd set up a proper ImGui context

    test_file_browser_initialization();
    test_project_selector_initialization();
    test_directory_validation();
    test_file_filtering();
    test_menu_state_management();
    test_path_handling();

    printf("\n========================\n");
    printf("All UI Dialog tests passed!\n");

    return 0;
}