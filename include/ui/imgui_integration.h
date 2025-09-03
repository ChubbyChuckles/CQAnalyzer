#ifndef IMGUI_INTEGRATION_H
#define IMGUI_INTEGRATION_H

#include <stdbool.h>
#include <GLFW/glfw3.h>

// ImGui initialization and shutdown
bool imgui_init(GLFWwindow* window);
void imgui_shutdown(void);

// Frame management
void imgui_new_frame(void);
void imgui_render(void);

// Demo and utility windows
void imgui_show_demo_window(bool* show);
void imgui_show_metrics_window(bool* show);
void imgui_show_style_editor(bool* show);

// CQAnalyzer specific GUI functions
void imgui_show_main_control_panel(bool* show);
void imgui_show_visualization_settings(bool* show);
void imgui_show_analysis_results(bool* show);

// Menu system
void imgui_show_main_menu_bar(void);
void imgui_show_file_menu(void);
void imgui_show_view_menu(void);
void imgui_show_tools_menu(void);
void imgui_show_help_menu(void);

// Menu state management
typedef struct {
    bool show_demo_window;
    bool show_metrics_window;
    bool show_style_editor;
    bool show_main_control_panel;
    bool show_visualization_settings;
    bool show_analysis_results;
    bool show_about_dialog;
    int visualization_mode;
    bool show_axes;
    bool show_grid;
    bool show_labels;
    bool enable_complexity_analysis;
    bool enable_dead_code_detection;
    bool enable_duplication_detection;
} MenuState;

extern MenuState menu_state;

// Menu state functions
void menu_state_init(void);
void menu_state_reset(void);

#endif // IMGUI_INTEGRATION_H