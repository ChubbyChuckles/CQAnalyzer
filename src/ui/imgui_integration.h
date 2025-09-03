#ifndef IMGUI_INTEGRATION_H
#define IMGUI_INTEGRATION_H

#include <stdbool.h>

// Forward declarations
typedef struct GLFWwindow GLFWwindow;

// Theme manager forward declaration
typedef struct ThemeManager ThemeManager;

// ImGui integration functions
bool imgui_init(GLFWwindow* window);
void imgui_shutdown(void);
void imgui_new_frame(void);
void imgui_render(void);

// Demo window functions
void imgui_show_demo_window(bool* show);
void imgui_show_metrics_window(bool* show);
void imgui_show_style_editor(bool* show);

// Metric configuration structure
typedef struct
{
    // Metric enable flags
    bool enable_cyclomatic_complexity;
    bool enable_lines_of_code;
    bool enable_halstead_metrics;
    bool enable_maintainability_index;
    bool enable_comment_density;
    bool enable_class_cohesion;
    bool enable_class_coupling;
    bool enable_dead_code_detection;
    bool enable_duplication_detection;

    // Metric thresholds
    float cyclomatic_complexity_threshold;
    float halstead_volume_threshold;
    float halstead_difficulty_threshold;
    float halstead_effort_threshold;
    float maintainability_index_threshold;
    float comment_density_threshold;
    float class_cohesion_threshold;
    float class_coupling_threshold;
    float dead_code_percentage_threshold;
    float duplication_percentage_threshold;

    // Metric weights for combined scoring
    float cyclomatic_complexity_weight;
    float halstead_metrics_weight;
    float maintainability_index_weight;
    float comment_density_weight;
    float class_cohesion_weight;
    float class_coupling_weight;
    float dead_code_weight;
    float duplication_weight;

    // Normalization settings
    int normalization_method; // 0: Min-Max, 1: Z-Score, 2: Robust
    bool auto_normalize;

    // Preset management
    char current_preset_name[64];
    bool show_metric_config_panel;
} MetricConfig;

// Camera control structure
typedef struct
{
    float position[3];      // Camera position (x, y, z)
    float target[3];        // Camera target (x, y, z)
    float up[3];           // Camera up vector (x, y, z)
    float yaw;             // Yaw angle in degrees
    float pitch;           // Pitch angle in degrees
    float distance;        // Distance from target
    float fov;             // Field of view
    float near_plane;      // Near clipping plane
    float far_plane;       // Far clipping plane
} CameraControls;

// Color scheme structure
typedef struct
{
    char name[64];
    float background_color[4];
    float grid_color[4];
    float axis_color[4];
    float point_color[4];
    float line_color[4];
    float text_color[4];
    float highlight_color[4];
} ColorScheme;

// Animation control structure
typedef struct
{
    bool enabled;
    float duration;        // Animation duration in seconds
    float speed;           // Animation speed multiplier
    bool loop;             // Loop animation
    int easing_type;       // 0: Linear, 1: EaseIn, 2: EaseOut, 3: EaseInOut
    bool auto_rotate;      // Auto-rotate camera
    float auto_rotate_speed;
} AnimationControls;

// General settings structure
typedef struct
{
    // Logging settings
    int log_level;         // 0: ERROR, 1: WARN, 2: INFO, 3: DEBUG
    bool log_to_file;
    char log_file_path[256];
    bool log_timestamps;
    int max_log_file_size; // MB

    // Performance settings
    int max_threads;
    bool enable_multithreading;
    int cache_size_mb;
    bool enable_gpu_acceleration;

    // UI preferences
    int theme;             // 0: Dark, 1: Light, 2: System
    float ui_scale;
    bool show_tooltips;
    bool auto_save_settings;
    int auto_save_interval; // minutes
} GeneralSettings;

// Analysis settings structure
typedef struct
{
    // Parsing options
    bool enable_incremental_parsing;
    int max_file_size_mb;
    bool follow_symbolic_links;
    int parsing_timeout_seconds;

    // Language support
    bool enable_c_support;
    bool enable_cpp_support;
    bool enable_java_support;
    bool enable_python_support;
    bool enable_javascript_support;
    bool enable_typescript_support;
    bool enable_custom_languages;

    // Analysis options
    bool enable_parallel_analysis;
    int analysis_batch_size;
    bool enable_caching;
} AnalysisSettings;

// Export settings structure
typedef struct
{
    // Export formats
    bool enable_csv_export;
    bool enable_json_export;
    bool enable_xml_export;
    bool enable_html_export;
    bool enable_pdf_export;

    // Export destinations
    char default_export_path[256];
    bool auto_open_after_export;
    bool include_timestamps;

    // Templates
    char csv_template[256];
    char json_template[256];
    char html_template[256];
} ExportSettings;

// Display options structure
typedef struct
{
    bool show_axes;
    bool show_grid;
    bool show_labels;
    bool show_bounding_box;
    bool show_wireframe;
    bool enable_lighting;
    bool enable_shadows;
    bool enable_fog;
    float point_size;
    float line_width;
    float label_scale;
    int render_quality;    // 0: Low, 1: Medium, 2: High
} DisplayOptions;

// Menu state structure
typedef struct
{
    // Window visibility flags
    bool show_main_control_panel;
    bool show_visualization_settings;
    bool show_analysis_results;
    bool show_about_dialog;
    bool show_file_browser;
    bool show_project_selector;
    bool show_metric_config_panel;
    bool show_camera_controls;
    bool show_display_options;
    bool show_color_scheme;
    bool show_animation_controls;
    bool show_settings_dialog;
    bool show_export_dialog;

    // Help system flags
    bool show_help_keyboard_shortcuts;
    bool show_help_documentation;
    bool show_help_faq;
    bool show_help_system_info;

    // Visualization settings
    int visualization_mode;
    bool show_axes;
    bool show_grid;
    bool show_labels;

    // Analysis settings
    bool enable_complexity_analysis;
    bool enable_dead_code_detection;
    bool enable_duplication_detection;

    // File browser state
    char current_directory[4096];
    char selected_file[4096];
    bool file_browser_open;

    // Project selector state
    char recent_projects[10][4096];
    int recent_projects_count;
    char selected_project[4096];
    bool project_selector_open;

    // Metric configuration
    MetricConfig metric_config;

    // New control panel states
    CameraControls camera_controls;
    DisplayOptions display_options;
    ColorScheme color_schemes[10];
    int current_color_scheme;
    int num_color_schemes;
    AnimationControls animation_controls;

    // Settings structures
    GeneralSettings general_settings;
    AnalysisSettings analysis_settings;
    ExportSettings export_settings;

    // Theme management
    ThemeManager* theme_manager;
    bool show_theme_panel;
    int preview_theme_index;
} MenuState;

// CQAnalyzer specific GUI functions
void imgui_show_main_control_panel(bool* show);
void imgui_show_visualization_settings(bool* show);
void imgui_show_analysis_results(bool* show);
void imgui_show_metric_config_panel(bool* show);
void imgui_show_settings_dialog(bool* show);
void imgui_show_export_dialog(bool* show);

// Help system functions
void imgui_show_help_keyboard_shortcuts(bool* show);
void imgui_show_help_documentation(bool* show);
void imgui_show_help_faq(bool* show);
void imgui_show_help_system_info(bool* show);

// New control panel functions
void imgui_show_camera_control_panel(bool* show);
void imgui_show_display_options_panel(bool* show);
void imgui_show_color_scheme_panel(bool* show);
void imgui_show_animation_control_panel(bool* show);
void imgui_show_visualization_mode_panel(bool* show);

// Integration functions for applying control panel settings
void imgui_apply_camera_settings(void);
void imgui_apply_display_settings(void);
void imgui_apply_color_scheme(void);
int imgui_get_visualization_mode(void);
bool imgui_get_display_option(const char* option_name);

// File and project dialog functions
void imgui_show_file_browser_dialog(bool* show);
void imgui_show_project_selector_dialog(bool* show);
void imgui_init_file_browser_state(void);
void imgui_init_project_selector_state(void);

// Theme management functions
void imgui_init_theme_manager(void);
void imgui_shutdown_theme_manager(void);
void imgui_show_theme_panel(bool* show);
bool imgui_apply_theme(int theme_index);
bool imgui_create_custom_theme(const char* name);
bool imgui_delete_theme(int theme_index);
const char* imgui_get_current_theme_name(void);
int imgui_get_theme_count(void);
const char* imgui_get_theme_name(int index);
void imgui_next_theme(void);
void imgui_previous_theme(void);

#endif // IMGUI_INTEGRATION_H