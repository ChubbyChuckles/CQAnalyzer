#include "theme_manager.h"

// ImGui includes
#include "../../third_party/imgui/imgui.h"

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Theme file format version
#define THEME_FILE_VERSION 1

// Maximum number of themes
#define MAX_THEMES 20

bool theme_manager_init(ThemeManager* manager)
{
    if (!manager) return false;

    memset(manager, 0, sizeof(ThemeManager));
    manager->num_themes = 0;
    manager->current_theme = 0;
    strcpy(manager->theme_file_path, "themes.ini");

    // Create predefined themes
    theme_manager_create_dark_theme(&manager->themes[manager->num_themes++]);
    theme_manager_create_light_theme(&manager->themes[manager->num_themes++]);
    theme_manager_create_classic_theme(&manager->themes[manager->num_themes++]);
    theme_manager_create_modern_theme(&manager->themes[manager->num_themes++]);
    theme_manager_create_high_contrast_theme(&manager->themes[manager->num_themes++]);
    theme_manager_create_minimal_theme(&manager->themes[manager->num_themes++]);

    return true;
}

void theme_manager_shutdown(ThemeManager* manager)
{
    if (!manager) return;

    // Save themes before shutdown
    theme_manager_save_themes(manager, manager->theme_file_path);
}

bool theme_manager_load_themes(ThemeManager* manager, const char* filepath)
{
    if (!manager || !filepath) return false;

    FILE* file = fopen(filepath, "r");
    if (!file) {
        // File doesn't exist, use defaults
        return true;
    }

    char line[512];
    int version = 0;
    int theme_count = 0;

    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0 || line[0] == '#') continue;

        if (strcmp(line, "[ThemeFile]") == 0) {
            // Read version
            if (fgets(line, sizeof(line), file)) {
                if (sscanf(line, "version=%d", &version) == 1) {
                    if (version != THEME_FILE_VERSION) {
                        fclose(file);
                        return false; // Version mismatch
                    }
                }
            }
        }
        else if (strcmp(line, "[Theme]") == 0) {
            if (theme_count >= MAX_THEMES) continue;

            ImGuiTheme* theme = &manager->themes[theme_count];
            theme->is_custom = true;

            // Read theme data
            while (fgets(line, sizeof(line), file)) {
                line[strcspn(line, "\n")] = 0;

                if (strlen(line) == 0) continue;
                if (line[0] == '[') break; // Next section

                char key[64], value[256];
                if (sscanf(line, "%[^=]=%[^\n]", key, value) == 2) {
                    // Parse theme properties
                    if (strcmp(key, "name") == 0) {
                        strncpy(theme->name, value, sizeof(theme->name) - 1);
                    }
                    // Add more parsing for ImGuiStyle properties as needed
                }
            }

            theme_count++;
        }
    }

    fclose(file);
    manager->num_themes = theme_count;
    return true;
}

bool theme_manager_save_themes(ThemeManager* manager, const char* filepath)
{
    if (!manager || !filepath) return false;

    FILE* file = fopen(filepath, "w");
    if (!file) return false;

    fprintf(file, "[ThemeFile]\n");
    fprintf(file, "version=%d\n\n", THEME_FILE_VERSION);

    for (int i = 0; i < manager->num_themes; i++) {
        const ImGuiTheme* theme = &manager->themes[i];

        if (!theme->is_custom) continue; // Only save custom themes

        fprintf(file, "[Theme]\n");
        fprintf(file, "name=%s\n", theme->name);
        // Add more serialization for ImGuiStyle properties as needed
        fprintf(file, "\n");
    }

    fclose(file);
    return true;
}

bool theme_manager_apply_theme(ThemeManager* manager, int theme_index)
{
    if (!manager || theme_index < 0 || theme_index >= manager->num_themes) return false;

    const ImGuiTheme* theme = &manager->themes[theme_index];

    // Apply the theme to ImGui
    ImGui::GetStyle() = theme->style;
    manager->current_theme = theme_index;

    return true;
}

bool theme_manager_create_theme(ThemeManager* manager, const char* name, const ImGuiStyle* base_style)
{
    if (!manager || !name || !base_style || manager->num_themes >= MAX_THEMES) return false;

    ImGuiTheme* theme = &manager->themes[manager->num_themes];
    strncpy(theme->name, name, sizeof(theme->name) - 1);
    theme->style = *base_style;
    theme->is_custom = true;

    manager->num_themes++;
    return true;
}

bool theme_manager_delete_theme(ThemeManager* manager, int theme_index)
{
    if (!manager || theme_index < 0 || theme_index >= manager->num_themes) return false;

    // Don't delete predefined themes
    if (!manager->themes[theme_index].is_custom) return false;

    // Shift remaining themes
    for (int i = theme_index; i < manager->num_themes - 1; i++) {
        manager->themes[i] = manager->themes[i + 1];
    }

    manager->num_themes--;

    // Adjust current theme index if necessary
    if (manager->current_theme >= manager->num_themes) {
        manager->current_theme = 0;
    }

    return true;
}

int theme_manager_find_theme(ThemeManager* manager, const char* name)
{
    if (!manager || !name) return -1;

    for (int i = 0; i < manager->num_themes; i++) {
        if (strcmp(manager->themes[i].name, name) == 0) {
            return i;
        }
    }

    return -1;
}

const char* theme_manager_get_current_theme_name(ThemeManager* manager)
{
    if (!manager || manager->current_theme < 0 || manager->current_theme >= manager->num_themes) {
        return "Unknown";
    }

    return manager->themes[manager->current_theme].name;
}

int theme_manager_get_theme_count(ThemeManager* manager)
{
    return manager ? manager->num_themes : 0;
}

const char* theme_manager_get_theme_name(ThemeManager* manager, int index)
{
    if (!manager || index < 0 || index >= manager->num_themes) {
        return "Unknown";
    }

    return manager->themes[index].name;
}

void theme_manager_copy_style(ImGuiStyle* dest, const ImGuiStyle* src)
{
    if (!dest || !src) return;
    *dest = *src;
}

bool theme_manager_validate_theme(const ImGuiTheme* theme)
{
    if (!theme) return false;
    if (strlen(theme->name) == 0) return false;

    // Add more validation as needed
    return true;
}

// Predefined theme creation functions

void theme_manager_create_dark_theme(ImGuiTheme* theme)
{
    if (!theme) return;

    strcpy(theme->name, "Dark");
    theme->is_custom = false;

    ImGuiStyle& style = theme->style;
    ImGui::StyleColorsDark(&style);

    // Customize dark theme
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    // Adjust colors for better contrast
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
}

void theme_manager_create_light_theme(ImGuiTheme* theme)
{
    if (!theme) return;

    strcpy(theme->name, "Light");
    theme->is_custom = false;

    ImGuiStyle& style = theme->style;
    ImGui::StyleColorsLight(&style);

    // Customize light theme
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    // Adjust colors for better readability
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
}

void theme_manager_create_classic_theme(ImGuiTheme* theme)
{
    if (!theme) return;

    strcpy(theme->name, "Classic");
    theme->is_custom = false;

    ImGuiStyle& style = theme->style;
    ImGui::StyleColorsClassic(&style);

    // Classic theme with minimal modifications
    style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
}

void theme_manager_create_modern_theme(ImGuiTheme* theme)
{
    if (!theme) return;

    strcpy(theme->name, "Modern");
    theme->is_custom = false;

    ImGuiStyle& style = theme->style;
    ImGui::StyleColorsDark(&style);

    // Modern theme with rounded corners and subtle colors
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;

    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;

    // Modern color palette
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.21f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.32f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.30f, 0.38f, 1.00f);
}

void theme_manager_create_high_contrast_theme(ImGuiTheme* theme)
{
    if (!theme) return;

    strcpy(theme->name, "High Contrast");
    theme->is_custom = false;

    ImGuiStyle& style = theme->style;
    ImGui::StyleColorsDark(&style);

    // High contrast theme
    style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;

    style.WindowBorderSize = 2.0f;
    style.FrameBorderSize = 2.0f;
    style.PopupBorderSize = 2.0f;

    // High contrast colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
}

void theme_manager_create_minimal_theme(ImGuiTheme* theme)
{
    if (!theme) return;

    strcpy(theme->name, "Minimal");
    theme->is_custom = false;

    ImGuiStyle& style = theme->style;
    ImGui::StyleColorsDark(&style);

    // Minimal theme with very subtle styling
    style.WindowRounding = 2.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.TabRounding = 2.0f;

    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;

    // Minimal colors - very subtle
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
}