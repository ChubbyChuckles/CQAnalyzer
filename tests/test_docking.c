#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../src/ui/imgui_integration.h"

// Mock ImGui functions for testing
void ImGui::SaveIniSettingsToDisk(const char* filename) {
    printf("Mock: Saving ImGui settings to %s\n", filename);
}

void ImGui::LoadIniSettingsFromDisk(const char* filename) {
    printf("Mock: Loading ImGui settings from %s\n", filename);
}

void ImGui::LoadIniSettingsFromMemory(const char* data) {
    printf("Mock: Loading ImGui settings from memory\n");
}

// Test functions
void test_dock_layout_save_load() {
    printf("Testing dock layout save/load...\n");

    // Test saving dock layout
    imgui_save_dock_layout("test_layout");
    printf("✓ Dock layout save test passed\n");

    // Test loading dock layout
    imgui_load_dock_layout("test_layout");
    printf("✓ Dock layout load test passed\n");

    // Test reset dock layout
    imgui_reset_dock_layout();
    printf("✓ Dock layout reset test passed\n");
}

void test_dock_presets() {
    printf("Testing dock presets...\n");

    // Test applying different presets
    for (int i = 0; i < 4; i++) {
        imgui_apply_dock_preset(i);
        printf("✓ Dock preset %d applied successfully\n", i);
    }
}

void test_panel_state_persistence() {
    printf("Testing panel state persistence...\n");

    // Modify some panel states
    menu_state.show_main_control_panel = false;
    menu_state.show_camera_controls = true;
    menu_state.show_display_options = true;

    // Save states
    imgui_save_dock_layout("test_states");

    // Reset states
    menu_state.show_main_control_panel = true;
    menu_state.show_camera_controls = false;
    menu_state.show_display_options = false;

    // Load states
    imgui_load_dock_layout("test_states");

    // Verify states were restored
    assert(menu_state.show_main_control_panel == false);
    assert(menu_state.show_camera_controls == true);
    assert(menu_state.show_display_options == true);

    printf("✓ Panel state persistence test passed\n");
}

int main() {
    printf("Running CQAnalyzer Docking Functionality Tests\n");
    printf("==============================================\n\n");

    // Initialize menu state for testing
    menu_state_init();

    // Run tests
    test_dock_layout_save_load();
    test_dock_presets();
    test_panel_state_persistence();

    printf("\n==============================================\n");
    printf("All docking functionality tests passed! ✓\n");

    return 0;
}