#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <stdbool.h>

// Forward declaration for ImGuiStyle
typedef struct ImGuiStyle ImGuiStyle;

// Theme structure
typedef struct
{
    char name[64];
    ImGuiStyle style;
    bool is_custom;
} ImGuiTheme;

// Theme manager structure
typedef struct
{
    ImGuiTheme themes[20];
    int num_themes;
    int current_theme;
    char theme_file_path[256];
} ThemeManager;

// Theme manager functions
bool theme_manager_init(ThemeManager* manager);
void theme_manager_shutdown(ThemeManager* manager);
bool theme_manager_load_themes(ThemeManager* manager, const char* filepath);
bool theme_manager_save_themes(ThemeManager* manager, const char* filepath);
bool theme_manager_apply_theme(ThemeManager* manager, int theme_index);
bool theme_manager_create_theme(ThemeManager* manager, const char* name, const ImGuiStyle* base_style);
bool theme_manager_delete_theme(ThemeManager* manager, int theme_index);
int theme_manager_find_theme(ThemeManager* manager, const char* name);
const char* theme_manager_get_current_theme_name(ThemeManager* manager);
int theme_manager_get_theme_count(ThemeManager* manager);
const char* theme_manager_get_theme_name(ThemeManager* manager, int index);

// Predefined theme creation functions
void theme_manager_create_dark_theme(ImGuiTheme* theme);
void theme_manager_create_light_theme(ImGuiTheme* theme);
void theme_manager_create_classic_theme(ImGuiTheme* theme);
void theme_manager_create_modern_theme(ImGuiTheme* theme);
void theme_manager_create_high_contrast_theme(ImGuiTheme* theme);
void theme_manager_create_minimal_theme(ImGuiTheme* theme);

// Theme utilities
void theme_manager_copy_style(ImGuiStyle* dest, const ImGuiStyle* src);
bool theme_manager_validate_theme(const ImGuiTheme* theme);

#endif // THEME_MANAGER_H