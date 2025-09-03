#include "imgui_integration.h"

// ImGui includes
#include "../../third_party/imgui/imgui.h"
#include "../../third_party/imgui/backends/imgui_impl_glfw.h"
#include "../../third_party/imgui/backends/imgui_impl_opengl3.h"

// Standard includes
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// CQAnalyzer includes
#include "../../include/cqanalyzer.h"
#include "../parser/file_scanner.h"
#include "metric_config.h"
#include "metric_applicator.h"
#include "theme_manager.h"

// Static variables
static bool imgui_initialized = false;

// Global theme manager
static ThemeManager global_theme_manager;

// Global menu state
MenuState menu_state;

bool imgui_init(GLFWwindow* window)
{
    if (imgui_initialized) {
        return true;
    }

    if (!window) {
        fprintf(stderr, "ImGui initialization failed: Invalid GLFW window\n");
        return false;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Initialize theme manager
    imgui_init_theme_manager();

    // Apply default theme (Dark)
    imgui_apply_theme(0);

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        fprintf(stderr, "ImGui GLFW backend initialization failed\n");
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        fprintf(stderr, "ImGui OpenGL3 backend initialization failed\n");
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    imgui_initialized = true;
    printf("ImGui initialized successfully\n");
    return true;
}

void imgui_shutdown(void)
{
    if (!imgui_initialized) {
        return;
    }

    // Save dock layout and panel states before shutdown
    imgui_save_dock_layout("shutdown");

    // Shutdown theme manager
    imgui_shutdown_theme_manager();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    imgui_initialized = false;
    printf("ImGui shutdown complete\n");
}

void imgui_new_frame(void)
{
    if (!imgui_initialized) {
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Handle keyboard shortcuts
    imgui_handle_keyboard_shortcuts();
}

void imgui_render(void)
{
    if (!imgui_initialized) {
        return;
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

// Demo window functions
void imgui_show_demo_window(bool* show)
{
    if (*show) {
        ImGui::ShowDemoWindow(show);
    }
}

void imgui_show_metrics_window(bool* show)
{
    if (*show) {
        ImGui::ShowMetricsWindow(show);
    }
}

void imgui_show_style_editor(bool* show)
{
    if (*show) {
        ImGui::ShowStyleEditor();
    }
}

// CQAnalyzer specific GUI functions
void imgui_show_main_control_panel(bool* show)
{
    if (!*show) return;

    ImGui::Begin("CQAnalyzer Control Panel", show, ImGuiWindowFlags_None);

    if (ImGui::CollapsingHeader("Visualization Settings")) {
        static int visualization_mode = 0;
        const char* modes[] = { "Scatter Plot", "Tree", "Network", "Heatmap" };
        ImGui::Combo("Mode", &visualization_mode, modes, IM_ARRAYSIZE(modes));

        static bool show_axes = true;
        static bool show_grid = true;
        static bool show_labels = true;
        ImGui::Checkbox("Show Axes", &show_axes);
        ImGui::Checkbox("Show Grid", &show_grid);
        ImGui::Checkbox("Show Labels", &show_labels);
    }

    if (ImGui::CollapsingHeader("Analysis Options")) {
        static bool enable_complexity_analysis = true;
        static bool enable_dead_code_detection = true;
        static bool enable_duplication_detection = true;

        ImGui::Checkbox("Complexity Analysis", &enable_complexity_analysis);
        ImGui::Checkbox("Dead Code Detection", &enable_dead_code_detection);
        ImGui::Checkbox("Duplication Detection", &enable_duplication_detection);

        if (ImGui::Button("Configure Metrics...")) {
            menu_state.show_metric_config_panel = true;
        }

        if (ImGui::Button("Run Analysis")) {
            // TODO: Trigger analysis
            printf("Analysis triggered\n");
        }
    }

    if (ImGui::CollapsingHeader("Performance")) {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    }

    ImGui::End();
}

// File browser dialog functions
void imgui_init_file_browser_state(void)
{
    // Initialize current directory to user's home directory or current working directory
    const char* home_dir = getenv("HOME");
    if (home_dir) {
        strncpy(menu_state.current_directory, home_dir, sizeof(menu_state.current_directory) - 1);
    } else {
        getcwd(menu_state.current_directory, sizeof(menu_state.current_directory));
    }
    menu_state.current_directory[sizeof(menu_state.current_directory) - 1] = '\0';
    menu_state.selected_file[0] = '\0';
    menu_state.file_browser_open = false;
}

// Helper function to get directory contents
static int get_directory_contents(const char* path, char*** files, char*** dirs, int* file_count, int* dir_count)
{
    DIR* dir = opendir(path);
    if (!dir) {
        return -1;
    }

    *file_count = 0;
    *dir_count = 0;
    *files = NULL;
    *dirs = NULL;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if (lstat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // Add to directories
                *dirs = realloc(*dirs, (*dir_count + 1) * sizeof(char*));
                if (*dirs) {
                    (*dirs)[*dir_count] = strdup(entry->d_name);
                    (*dir_count)++;
                }
            } else if (S_ISREG(st.st_mode)) {
                // Add to files
                *files = realloc(*files, (*file_count + 1) * sizeof(char*));
                if (*files) {
                    (*files)[*file_count] = strdup(entry->d_name);
                    (*file_count)++;
                }
            }
        }
    }

    closedir(dir);
    return 0;
}

// Helper function to free directory contents
static void free_directory_contents(char** files, char** dirs, int file_count, int dir_count)
{
    for (int i = 0; i < file_count; i++) {
        free(files[i]);
    }
    for (int i = 0; i < dir_count; i++) {
        free(dirs[i]);
    }
    free(files);
    free(dirs);
}

void imgui_show_file_browser_dialog(bool* show)
{
    if (!*show) return;

    ImGui::OpenPopup("File Browser");

    if (ImGui::BeginPopupModal("File Browser", show, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // Current directory display
        ImGui::Text("Current Directory: %s", menu_state.current_directory);
        ImGui::Separator();

        // Directory navigation buttons
        if (ImGui::Button("Up")) {
            // Navigate to parent directory
            char* last_slash = strrchr(menu_state.current_directory, '/');
            if (last_slash && last_slash != menu_state.current_directory) {
                *last_slash = '\0';
            } else if (strcmp(menu_state.current_directory, "/") != 0) {
                strcpy(menu_state.current_directory, "/");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Home")) {
            const char* home_dir = getenv("HOME");
            if (home_dir) {
                strncpy(menu_state.current_directory, home_dir, sizeof(menu_state.current_directory) - 1);
                menu_state.current_directory[sizeof(menu_state.current_directory) - 1] = '\0';
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Refresh")) {
            // Directory will be refreshed on next frame
        }

        ImGui::Separator();

        // Get directory contents
        char** files = NULL;
        char** dirs = NULL;
        int file_count = 0;
        int dir_count = 0;

        if (get_directory_contents(menu_state.current_directory, &files, &dirs, &file_count, &dir_count) == 0) {
            // File filtering options
            static bool show_source_files_only = true;
            static int selected_language = 0;
            const char* languages[] = { "All", "C/C++", "Java", "Python", "JavaScript", "TypeScript" };

            ImGui::Checkbox("Show source files only", &show_source_files_only);
            if (show_source_files_only) {
                ImGui::SameLine();
                ImGui::Combo("Language", &selected_language, languages, IM_ARRAYSIZE(languages));
            }

            ImGui::Separator();
            ImGui::BeginChild("DirectoryContents", ImVec2(400, 300), true);

            // Show directories first
            for (int i = 0; i < dir_count; i++) {
                if (ImGui::Selectable(dirs[i], false, ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        // Navigate into directory
                        char new_path[MAX_PATH_LENGTH];
                        snprintf(new_path, sizeof(new_path), "%s/%s", menu_state.current_directory, dirs[i]);
                        strncpy(menu_state.current_directory, new_path, sizeof(menu_state.current_directory) - 1);
                        menu_state.current_directory[sizeof(menu_state.current_directory) - 1] = '\0';
                    }
                }
            }

            // Show files
            for (int i = 0; i < file_count; i++) {
                bool should_show = true;

                if (show_source_files_only) {
                    SupportedLanguage lang = LANG_UNKNOWN;
                    switch (selected_language) {
                        case 0: lang = LANG_UNKNOWN; break; // All languages
                        case 1: lang = LANG_C; break;
                        case 2: lang = LANG_JAVA; break;
                        case 3: lang = LANG_PYTHON; break;
                        case 4: lang = LANG_JAVASCRIPT; break;
                        case 5: lang = LANG_TYPESCRIPT; break;
                    }

                    if (lang != LANG_UNKNOWN) {
                        should_show = is_source_file(files[i], lang);
                    } else {
                        // Check if it's any supported source file
                        SupportedLanguage langs[] = {LANG_C, LANG_CPP, LANG_JAVA, LANG_PYTHON, LANG_JAVASCRIPT, LANG_TYPESCRIPT};
                        should_show = false;
                        for (size_t j = 0; j < sizeof(langs) / sizeof(langs[0]); j++) {
                            if (is_source_file(files[i], langs[j])) {
                                should_show = true;
                                break;
                            }
                        }
                    }
                }

                if (should_show) {
                    char full_path[MAX_PATH_LENGTH];
                    snprintf(full_path, sizeof(full_path), "%s/%s", menu_state.current_directory, files[i]);

                    if (ImGui::Selectable(files[i], strcmp(menu_state.selected_file, full_path) == 0)) {
                        strncpy(menu_state.selected_file, full_path, sizeof(menu_state.selected_file) - 1);
                        menu_state.selected_file[sizeof(menu_state.selected_file) - 1] = '\0';
                    }
                }
            }

            ImGui::EndChild();

            // Free allocated memory
            free_directory_contents(files, dirs, file_count, dir_count);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: Cannot access directory");
        }

        // Selected file display
        if (menu_state.selected_file[0] != '\0') {
            ImGui::Separator();
            ImGui::Text("Selected: %s", strrchr(menu_state.selected_file, '/') ? strrchr(menu_state.selected_file, '/') + 1 : menu_state.selected_file);
        }

        ImGui::Separator();

        // Dialog buttons
        if (ImGui::Button("Open", ImVec2(80, 0))) {
            if (menu_state.selected_file[0] != '\0') {
                // TODO: Handle file opening
                printf("Opening file: %s\n", menu_state.selected_file);
                *show = false;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            *show = false;
        }

        ImGui::EndPopup();
    }
}

// Project selector dialog functions
void imgui_init_project_selector_state(void)
{
    menu_state.recent_projects_count = 0;
    menu_state.selected_project[0] = '\0';
    menu_state.project_selector_open = false;

    // Add some sample recent projects for demonstration
    const char* sample_projects[] = {
        "/home/user/projects/myapp",
        "/home/user/projects/web-frontend",
        "/home/user/projects/api-server",
        "/usr/local/src/linux-kernel",
        "/home/user/downloads/sample-code"
    };

    for (int i = 0; i < 5 && i < 10; i++) {
        if (access(sample_projects[i], F_OK) == 0) {  // Check if directory exists
            strncpy(menu_state.recent_projects[menu_state.recent_projects_count],
                   sample_projects[i], sizeof(menu_state.recent_projects[0]) - 1);
            menu_state.recent_projects[menu_state.recent_projects_count]
                [sizeof(menu_state.recent_projects[0]) - 1] = '\0';
            menu_state.recent_projects_count++;
        }
    }

    // Load recent projects from configuration (placeholder)
    // In a real implementation, this would load from a config file
}

void imgui_show_project_selector_dialog(bool* show)
{
    if (!*show) return;

    ImGui::OpenPopup("Select Project");

    if (ImGui::BeginPopupModal("Select Project", show, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Choose a project to analyze:");
        ImGui::Separator();

        // Recent projects list
        if (menu_state.recent_projects_count > 0) {
            ImGui::Text("Recent Projects:");
            for (int i = 0; i < menu_state.recent_projects_count; i++) {
                if (ImGui::Selectable(menu_state.recent_projects[i], false)) {
                    strncpy(menu_state.selected_project, menu_state.recent_projects[i],
                           sizeof(menu_state.selected_project) - 1);
                    menu_state.selected_project[sizeof(menu_state.selected_project) - 1] = '\0';
                }
            }
            ImGui::Separator();
        } else {
            ImGui::Text("No recent projects found.");
            ImGui::Separator();
        }

        // Manual project path input
        static char project_path[4096] = "";
        static char error_message[256] = "";
        static bool show_error = false;

        ImGui::Text("Or enter project path:");
        ImGui::InputText("Project Path", project_path, sizeof(project_path));

        if (ImGui::Button("Browse...")) {
            // Open file browser for directory selection
            menu_state.show_file_browser = true;
        }

        // Show error message if any
        if (show_error && error_message[0] != '\0') {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", error_message);
        }

        ImGui::Separator();

        // Dialog buttons
        if (ImGui::Button("Analyze", ImVec2(80, 0))) {
            const char* selected_path = NULL;

            if (project_path[0] != '\0') {
                selected_path = project_path;
            } else if (menu_state.selected_project[0] != '\0') {
                selected_path = menu_state.selected_project;
            }

            if (selected_path) {
                // Validate directory exists and is accessible
                struct stat st;
                if (stat(selected_path, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        // Check if directory is readable
                        if (access(selected_path, R_OK) == 0) {
                            strncpy(menu_state.selected_project, selected_path,
                                   sizeof(menu_state.selected_project) - 1);
                            menu_state.selected_project[sizeof(menu_state.selected_project) - 1] = '\0';

                            // Add to recent projects if not already there
                            bool already_exists = false;
                            for (int i = 0; i < menu_state.recent_projects_count; i++) {
                                if (strcmp(menu_state.recent_projects[i], selected_path) == 0) {
                                    already_exists = true;
                                    break;
                                }
                            }

                            if (!already_exists && menu_state.recent_projects_count < 10) {
                                strncpy(menu_state.recent_projects[menu_state.recent_projects_count],
                                       selected_path, sizeof(menu_state.recent_projects[0]) - 1);
                                menu_state.recent_projects[menu_state.recent_projects_count]
                                    [sizeof(menu_state.recent_projects[0]) - 1] = '\0';
                                menu_state.recent_projects_count++;
                            }

                            // TODO: Start analysis
                            printf("Starting analysis of project: %s\n", menu_state.selected_project);
                            show_error = false;
                            error_message[0] = '\0';
                            *show = false;
                        } else {
                            show_error = true;
                            strcpy(error_message, "Directory is not readable");
                        }
                    } else {
                        show_error = true;
                        strcpy(error_message, "Path is not a directory");
                    }
                } else {
                    show_error = true;
                    strcpy(error_message, "Directory does not exist");
                }
            } else {
                show_error = true;
                strcpy(error_message, "Please select or enter a project path");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            *show = false;
        }

        ImGui::EndPopup();
    }
}

// Menu state management functions
void menu_state_init(void)
{
    memset(&menu_state, 0, sizeof(MenuState));

    // Set default values
    menu_state.show_main_control_panel = true;
    menu_state.visualization_mode = 0; // Scatter Plot
    menu_state.show_axes = true;
    menu_state.show_grid = true;
    menu_state.show_labels = true;
    menu_state.enable_complexity_analysis = true;
    menu_state.enable_dead_code_detection = true;
    menu_state.enable_duplication_detection = true;

    // Initialize new control panel visibility
    menu_state.show_camera_controls = false;
    menu_state.show_display_options = false;
    menu_state.show_color_scheme = false;
    menu_state.show_animation_controls = false;
    menu_state.show_export_dialog = false;
    menu_state.show_theme_panel = false;
    menu_state.preview_theme_index = 0;

    // Initialize help system flags
    menu_state.show_help_keyboard_shortcuts = false;
    menu_state.show_help_documentation = false;
    menu_state.show_help_faq = false;
    menu_state.show_help_system_info = false;

    // Initialize camera controls
    menu_state.camera_controls.position[0] = 0.0f;
    menu_state.camera_controls.position[1] = 0.0f;
    menu_state.camera_controls.position[2] = 5.0f;
    menu_state.camera_controls.target[0] = 0.0f;
    menu_state.camera_controls.target[1] = 0.0f;
    menu_state.camera_controls.target[2] = 0.0f;
    menu_state.camera_controls.up[0] = 0.0f;
    menu_state.camera_controls.up[1] = 1.0f;
    menu_state.camera_controls.up[2] = 0.0f;
    menu_state.camera_controls.yaw = 0.0f;
    menu_state.camera_controls.pitch = 0.0f;
    menu_state.camera_controls.distance = 5.0f;
    menu_state.camera_controls.fov = 45.0f;
    menu_state.camera_controls.near_plane = 0.1f;
    menu_state.camera_controls.far_plane = 100.0f;

    // Initialize display options
    menu_state.display_options.show_axes = true;
    menu_state.display_options.show_grid = true;
    menu_state.display_options.show_labels = true;
    menu_state.display_options.show_bounding_box = false;
    menu_state.display_options.show_wireframe = false;
    menu_state.display_options.enable_lighting = true;
    menu_state.display_options.enable_shadows = false;
    menu_state.display_options.enable_fog = false;
    menu_state.display_options.point_size = 5.0f;
    menu_state.display_options.line_width = 2.0f;
    menu_state.display_options.label_scale = 1.0f;
    menu_state.display_options.render_quality = 1; // Medium

    // Initialize color schemes
    menu_state.current_color_scheme = 0;
    menu_state.num_color_schemes = 3;

    // Default color scheme
    strcpy(menu_state.color_schemes[0].name, "Default");
    menu_state.color_schemes[0].background_color[0] = 0.1f;
    menu_state.color_schemes[0].background_color[1] = 0.1f;
    menu_state.color_schemes[0].background_color[2] = 0.1f;
    menu_state.color_schemes[0].background_color[3] = 1.0f;
    menu_state.color_schemes[0].grid_color[0] = 0.3f;
    menu_state.color_schemes[0].grid_color[1] = 0.3f;
    menu_state.color_schemes[0].grid_color[2] = 0.3f;
    menu_state.color_schemes[0].grid_color[3] = 1.0f;
    menu_state.color_schemes[0].axis_color[0] = 0.7f;
    menu_state.color_schemes[0].axis_color[1] = 0.7f;
    menu_state.color_schemes[0].axis_color[2] = 0.7f;
    menu_state.color_schemes[0].axis_color[3] = 1.0f;
    menu_state.color_schemes[0].point_color[0] = 0.2f;
    menu_state.color_schemes[0].point_color[1] = 0.6f;
    menu_state.color_schemes[0].point_color[2] = 1.0f;
    menu_state.color_schemes[0].point_color[3] = 1.0f;
    menu_state.color_schemes[0].line_color[0] = 1.0f;
    menu_state.color_schemes[0].line_color[1] = 1.0f;
    menu_state.color_schemes[0].line_color[2] = 1.0f;
    menu_state.color_schemes[0].line_color[3] = 1.0f;
    menu_state.color_schemes[0].text_color[0] = 1.0f;
    menu_state.color_schemes[0].text_color[1] = 1.0f;
    menu_state.color_schemes[0].text_color[2] = 1.0f;
    menu_state.color_schemes[0].text_color[3] = 1.0f;
    menu_state.color_schemes[0].highlight_color[0] = 1.0f;
    menu_state.color_schemes[0].highlight_color[1] = 0.5f;
    menu_state.color_schemes[0].highlight_color[2] = 0.0f;
    menu_state.color_schemes[0].highlight_color[3] = 1.0f;

    // Dark color scheme
    strcpy(menu_state.color_schemes[1].name, "Dark");
    menu_state.color_schemes[1].background_color[0] = 0.05f;
    menu_state.color_schemes[1].background_color[1] = 0.05f;
    menu_state.color_schemes[1].background_color[2] = 0.05f;
    menu_state.color_schemes[1].background_color[3] = 1.0f;
    menu_state.color_schemes[1].grid_color[0] = 0.2f;
    menu_state.color_schemes[1].grid_color[1] = 0.2f;
    menu_state.color_schemes[1].grid_color[2] = 0.2f;
    menu_state.color_schemes[1].grid_color[3] = 1.0f;
    menu_state.color_schemes[1].axis_color[0] = 0.5f;
    menu_state.color_schemes[1].axis_color[1] = 0.5f;
    menu_state.color_schemes[1].axis_color[2] = 0.5f;
    menu_state.color_schemes[1].axis_color[3] = 1.0f;
    menu_state.color_schemes[1].point_color[0] = 0.3f;
    menu_state.color_schemes[1].point_color[1] = 0.7f;
    menu_state.color_schemes[1].point_color[2] = 1.0f;
    menu_state.color_schemes[1].point_color[3] = 1.0f;
    menu_state.color_schemes[1].line_color[0] = 0.8f;
    menu_state.color_schemes[1].line_color[1] = 0.8f;
    menu_state.color_schemes[1].line_color[2] = 0.8f;
    menu_state.color_schemes[1].line_color[3] = 1.0f;
    menu_state.color_schemes[1].text_color[0] = 0.9f;
    menu_state.color_schemes[1].text_color[1] = 0.9f;
    menu_state.color_schemes[1].text_color[2] = 0.9f;
    menu_state.color_schemes[1].text_color[3] = 1.0f;
    menu_state.color_schemes[1].highlight_color[0] = 1.0f;
    menu_state.color_schemes[1].highlight_color[1] = 0.6f;
    menu_state.color_schemes[1].highlight_color[2] = 0.2f;
    menu_state.color_schemes[1].highlight_color[3] = 1.0f;

    // Light color scheme
    strcpy(menu_state.color_schemes[2].name, "Light");
    menu_state.color_schemes[2].background_color[0] = 0.9f;
    menu_state.color_schemes[2].background_color[1] = 0.9f;
    menu_state.color_schemes[2].background_color[2] = 0.9f;
    menu_state.color_schemes[2].background_color[3] = 1.0f;
    menu_state.color_schemes[2].grid_color[0] = 0.7f;
    menu_state.color_schemes[2].grid_color[1] = 0.7f;
    menu_state.color_schemes[2].grid_color[2] = 0.7f;
    menu_state.color_schemes[2].grid_color[3] = 1.0f;
    menu_state.color_schemes[2].axis_color[0] = 0.3f;
    menu_state.color_schemes[2].axis_color[1] = 0.3f;
    menu_state.color_schemes[2].axis_color[2] = 0.3f;
    menu_state.color_schemes[2].axis_color[3] = 1.0f;
    menu_state.color_schemes[2].point_color[0] = 0.1f;
    menu_state.color_schemes[2].point_color[1] = 0.4f;
    menu_state.color_schemes[2].point_color[2] = 0.8f;
    menu_state.color_schemes[2].point_color[3] = 1.0f;
    menu_state.color_schemes[2].line_color[0] = 0.2f;
    menu_state.color_schemes[2].line_color[1] = 0.2f;
    menu_state.color_schemes[2].line_color[2] = 0.2f;
    menu_state.color_schemes[2].line_color[3] = 1.0f;
    menu_state.color_schemes[2].text_color[0] = 0.1f;
    menu_state.color_schemes[2].text_color[1] = 0.1f;
    menu_state.color_schemes[2].text_color[2] = 0.1f;
    menu_state.color_schemes[2].text_color[3] = 1.0f;
    menu_state.color_schemes[2].highlight_color[0] = 0.8f;
    menu_state.color_schemes[2].highlight_color[1] = 0.3f;
    menu_state.color_schemes[2].highlight_color[2] = 0.1f;
    menu_state.color_schemes[2].highlight_color[3] = 1.0f;

    // Initialize animation controls
    menu_state.animation_controls.enabled = false;
    menu_state.animation_controls.duration = 2.0f;
    menu_state.animation_controls.speed = 1.0f;
    menu_state.animation_controls.loop = false;
    menu_state.animation_controls.easing_type = 0; // Linear
    menu_state.animation_controls.auto_rotate = false;
    menu_state.animation_controls.auto_rotate_speed = 0.5f;

    // Initialize general settings
    menu_state.general_settings.log_level = 2; // INFO
    menu_state.general_settings.log_to_file = false;
    strcpy(menu_state.general_settings.log_file_path, "cqanalyzer.log");
    menu_state.general_settings.log_timestamps = true;
    menu_state.general_settings.max_log_file_size = 10; // MB
    menu_state.general_settings.max_threads = 4;
    menu_state.general_settings.enable_multithreading = true;
    menu_state.general_settings.cache_size_mb = 100;
    menu_state.general_settings.enable_gpu_acceleration = true;
    menu_state.general_settings.theme = 0; // Dark
    menu_state.general_settings.ui_scale = 1.0f;
    menu_state.general_settings.show_tooltips = true;
    menu_state.general_settings.auto_save_settings = true;
    menu_state.general_settings.auto_save_interval = 5; // minutes

    // Initialize analysis settings
    menu_state.analysis_settings.enable_incremental_parsing = true;
    menu_state.analysis_settings.max_file_size_mb = 50;
    menu_state.analysis_settings.follow_symbolic_links = false;
    menu_state.analysis_settings.parsing_timeout_seconds = 30;
    menu_state.analysis_settings.enable_c_support = true;
    menu_state.analysis_settings.enable_cpp_support = true;
    menu_state.analysis_settings.enable_java_support = true;
    menu_state.analysis_settings.enable_python_support = true;
    menu_state.analysis_settings.enable_javascript_support = true;
    menu_state.analysis_settings.enable_typescript_support = true;
    menu_state.analysis_settings.enable_custom_languages = false;
    menu_state.analysis_settings.enable_parallel_analysis = true;
    menu_state.analysis_settings.analysis_batch_size = 10;
    menu_state.analysis_settings.enable_caching = true;

    // Initialize export settings
    menu_state.export_settings.enable_csv_export = true;
    menu_state.export_settings.enable_json_export = true;
    menu_state.export_settings.enable_xml_export = true;
    menu_state.export_settings.enable_html_export = true;
    menu_state.export_settings.enable_pdf_export = false;
    strcpy(menu_state.export_settings.default_export_path, "./exports");
    menu_state.export_settings.auto_open_after_export = true;
    menu_state.export_settings.include_timestamps = true;
    strcpy(menu_state.export_settings.csv_template, "default.csv");
    strcpy(menu_state.export_settings.json_template, "default.json");
    strcpy(menu_state.export_settings.html_template, "default.html");

    // Initialize metric configuration with defaults
    menu_state.metric_config.enable_cyclomatic_complexity = true;
    menu_state.metric_config.enable_lines_of_code = true;
    menu_state.metric_config.enable_halstead_metrics = true;
    menu_state.metric_config.enable_maintainability_index = true;
    menu_state.metric_config.enable_comment_density = true;
    menu_state.metric_config.enable_class_cohesion = false;
    menu_state.metric_config.enable_class_coupling = false;
    menu_state.metric_config.enable_dead_code_detection = true;
    menu_state.metric_config.enable_duplication_detection = true;

    // Default thresholds
    menu_state.metric_config.cyclomatic_complexity_threshold = 10.0f;
    menu_state.metric_config.halstead_volume_threshold = 1000.0f;
    menu_state.metric_config.halstead_difficulty_threshold = 20.0f;
    menu_state.metric_config.halstead_effort_threshold = 20000.0f;
    menu_state.metric_config.maintainability_index_threshold = 50.0f;
    menu_state.metric_config.comment_density_threshold = 15.0f;
    menu_state.metric_config.class_cohesion_threshold = 0.5f;
    menu_state.metric_config.class_coupling_threshold = 0.7f;
    menu_state.metric_config.dead_code_percentage_threshold = 20.0f;
    menu_state.metric_config.duplication_percentage_threshold = 30.0f;

    // Default weights (equal distribution)
    menu_state.metric_config.cyclomatic_complexity_weight = 0.2f;
    menu_state.metric_config.halstead_metrics_weight = 0.2f;
    menu_state.metric_config.maintainability_index_weight = 0.2f;
    menu_state.metric_config.comment_density_weight = 0.1f;
    menu_state.metric_config.class_cohesion_weight = 0.1f;
    menu_state.metric_config.class_coupling_weight = 0.1f;
    menu_state.metric_config.dead_code_weight = 0.05f;
    menu_state.metric_config.duplication_weight = 0.05f;

    // Normalization settings
    menu_state.metric_config.normalization_method = 0; // Min-Max
    menu_state.metric_config.auto_normalize = true;

    // Preset
    strcpy(menu_state.metric_config.current_preset_name, "Default");
    menu_state.metric_config.show_metric_config_panel = false;

    // Try to load settings from file
    imgui_load_settings("cqanalyzer_settings.ini");

    // Try to load dock layout and panel states
    if (access("cqanalyzer_dock_layout.ini", F_OK) == 0)
    {
        imgui_load_dock_layout("current");
        LOG_INFO("Loaded saved dock layout on startup");
    }
    else
    {
        LOG_INFO("No saved dock layout found, using defaults");
    }
}

void menu_state_reset(void)
{
    menu_state_init();
}

// Main menu bar
void imgui_show_main_menu_bar(void)
{
    if (ImGui::BeginMainMenuBar())
    {
        imgui_show_file_menu();
        imgui_show_view_menu();
        imgui_show_tools_menu();
        imgui_show_help_menu();

        ImGui::EndMainMenuBar();
    }

    // Show export dialog if requested
    imgui_show_export_dialog(&menu_state.show_export_dialog);

    // Show theme panel if requested
    imgui_show_theme_panel(&menu_state.show_theme_panel);
}

// File menu
void imgui_show_file_menu(void)
{
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
        {
            menu_state.show_project_selector = true;
        }

        if (ImGui::MenuItem("Save Results", "Ctrl+S"))
        {
            // TODO: Implement save results functionality
            printf("Save Results selected\n");
        }

        if (ImGui::MenuItem("Export...", "Ctrl+E"))
        {
            menu_state.show_export_dialog = true;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Exit", "Alt+F4"))
        {
            // TODO: Signal application to exit
            printf("Exit selected\n");
        }

        ImGui::EndMenu();
    }
}

// View menu
void imgui_show_view_menu(void)
{
    if (ImGui::BeginMenu("View"))
    {
        if (ImGui::BeginMenu("Visualization Mode"))
        {
            if (ImGui::MenuItem("Scatter Plot", NULL, menu_state.visualization_mode == 0))
            {
                menu_state.visualization_mode = 0;
                // TODO: Update visualization mode
                printf("Switched to Scatter Plot mode\n");
            }

            if (ImGui::MenuItem("Tree", NULL, menu_state.visualization_mode == 1))
            {
                menu_state.visualization_mode = 1;
                // TODO: Update visualization mode
                printf("Switched to Tree mode\n");
            }

            if (ImGui::MenuItem("Network", NULL, menu_state.visualization_mode == 2))
            {
                menu_state.visualization_mode = 2;
                // TODO: Update visualization mode
                printf("Switched to Network mode\n");
            }

            if (ImGui::MenuItem("Heatmap", NULL, menu_state.visualization_mode == 3))
            {
                menu_state.visualization_mode = 3;
                // TODO: Update visualization mode
                printf("Switched to Heatmap mode\n");
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Show Axes", NULL, &menu_state.show_axes))
        {
            // TODO: Toggle axes display
            printf("Axes display: %s\n", menu_state.show_axes ? "ON" : "OFF");
        }

        if (ImGui::MenuItem("Show Grid", NULL, &menu_state.show_grid))
        {
            // TODO: Toggle grid display
            printf("Grid display: %s\n", menu_state.show_grid ? "ON" : "OFF");
        }

        if (ImGui::MenuItem("Show Labels", NULL, &menu_state.show_labels))
        {
            // TODO: Toggle labels display
            printf("Labels display: %s\n", menu_state.show_labels ? "ON" : "OFF");
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Control Panel", NULL, &menu_state.show_main_control_panel))
        {
            // Control panel visibility is handled by the checkbox state
        }

        if (ImGui::MenuItem("Visualization Settings", NULL, &menu_state.show_visualization_settings))
        {
            // Settings window visibility is handled by the checkbox state
        }

        if (ImGui::MenuItem("Analysis Results", NULL, &menu_state.show_analysis_results))
        {
            // Results window visibility is handled by the checkbox state
        }

        if (ImGui::MenuItem("Metric Configuration", NULL, &menu_state.show_metric_config_panel))
        {
            // Metric configuration panel visibility is handled by the checkbox state
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Camera Controls", NULL, &menu_state.show_camera_controls))
        {
            // Camera controls panel visibility is handled by the checkbox state
        }

        if (ImGui::MenuItem("Display Options", NULL, &menu_state.show_display_options))
        {
            // Display options panel visibility is handled by the checkbox state
        }

        if (ImGui::MenuItem("Color Scheme", NULL, &menu_state.show_color_scheme))
        {
            // Color scheme panel visibility is handled by the checkbox state
        }

        if (ImGui::MenuItem("Animation Controls", NULL, &menu_state.show_animation_controls))
        {
            // Animation controls panel visibility is handled by the checkbox state
        }

        if (ImGui::MenuItem("Theme Manager", NULL, &menu_state.show_theme_panel))
        {
            // Theme manager panel visibility is handled by the checkbox state
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Dock Layout"))
        {
            if (ImGui::MenuItem("Save Layout", "Ctrl+Shift+S"))
            {
                imgui_save_dock_layout("current");
            }

            if (ImGui::MenuItem("Load Layout", "Ctrl+Shift+L"))
            {
                imgui_load_dock_layout("current");
            }

            if (ImGui::MenuItem("Reset Layout", "Ctrl+Shift+R"))
            {
                imgui_reset_dock_layout();
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Presets"))
            {
                if (ImGui::MenuItem("Default Layout"))
                {
                    imgui_apply_dock_preset(0);
                }

                if (ImGui::MenuItem("Analysis Layout"))
                {
                    imgui_apply_dock_preset(1);
                }

                if (ImGui::MenuItem("Visualization Layout"))
                {
                    imgui_apply_dock_preset(2);
                }

                if (ImGui::MenuItem("Development Layout"))
                {
                    imgui_apply_dock_preset(3);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }
}

// Tools menu
void imgui_show_tools_menu(void)
{
    if (ImGui::BeginMenu("Tools"))
    {
        if (ImGui::MenuItem("Run Analysis", "F5"))
        {
            // TODO: Trigger analysis
            printf("Analysis triggered\n");
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Complexity Analysis", NULL, &menu_state.enable_complexity_analysis))
        {
            // TODO: Update analysis settings
            printf("Complexity analysis: %s\n", menu_state.enable_complexity_analysis ? "ENABLED" : "DISABLED");
        }

        if (ImGui::MenuItem("Dead Code Detection", NULL, &menu_state.enable_dead_code_detection))
        {
            // TODO: Update analysis settings
            printf("Dead code detection: %s\n", menu_state.enable_dead_code_detection ? "ENABLED" : "DISABLED");
        }

        if (ImGui::MenuItem("Duplication Detection", NULL, &menu_state.enable_duplication_detection))
        {
            // TODO: Update analysis settings
            printf("Duplication detection: %s\n", menu_state.enable_duplication_detection ? "ENABLED" : "DISABLED");
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Settings...", "Ctrl+,"))
        {
            menu_state.show_settings_dialog = true;
        }

        ImGui::EndMenu();
    }
}

// Help menu
void imgui_show_help_menu(void)
{
    if (ImGui::BeginMenu("Help"))
    {
        if (ImGui::MenuItem("Documentation"))
        {
            menu_state.show_help_documentation = true;
        }

        if (ImGui::MenuItem("Keyboard Shortcuts"))
        {
            menu_state.show_help_keyboard_shortcuts = true;
        }

        if (ImGui::MenuItem("FAQ"))
        {
            menu_state.show_help_faq = true;
        }

        if (ImGui::MenuItem("System Information"))
        {
            menu_state.show_help_system_info = true;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("About CQAnalyzer"))
        {
            menu_state.show_about_dialog = true;
        }

        ImGui::EndMenu();
    }
}

// About dialog
void imgui_show_about_dialog(void)
{
    if (!menu_state.show_about_dialog)
        return;

    ImGui::OpenPopup("About CQAnalyzer");

    if (ImGui::BeginPopupModal("About CQAnalyzer", &menu_state.show_about_dialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("CQAnalyzer v1.0.0");
        ImGui::Separator();
        ImGui::Text("Code Quality Analyzer with 3D Visualization");
        ImGui::Text("");
        ImGui::Text("Built with:");
        ImGui::BulletText("Dear ImGui for GUI");
        ImGui::BulletText("OpenGL for 3D rendering");
        ImGui::BulletText("GLFW for window management");
        ImGui::Text("");
        ImGui::Text("Copyright (c) 2024 CQAnalyzer Team");
        ImGui::Text("");
        ImGui::Text("Credits:");
        ImGui::BulletText("Developed by: CQAnalyzer Development Team");
        ImGui::BulletText("Icons: Custom designed");
        ImGui::BulletText("Documentation: Community contributed");
        ImGui::Text("");
        ImGui::Text("License: MIT License");
        ImGui::Text("Website: https://github.com/cqanalyzer/cqanalyzer");

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            menu_state.show_about_dialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

// Help system functions
void imgui_show_help_keyboard_shortcuts(bool* show)
{
    if (!*show) return;

    ImGui::Begin("Keyboard Shortcuts", show, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::BulletText("F1: Show ImGui Demo Window");
        ImGui::BulletText("F2: Toggle Main Control Panel");
        ImGui::BulletText("F3: Toggle Metrics Window");
        ImGui::BulletText("F5: Run Analysis");
        ImGui::BulletText("F11: Toggle Fullscreen");
        ImGui::BulletText("ESC: Exit Application");
    }

    if (ImGui::CollapsingHeader("File Operations"))
    {
        ImGui::BulletText("Ctrl+O: Open Project");
        ImGui::BulletText("Ctrl+S: Save Results");
        ImGui::BulletText("Ctrl+B: Open File Browser");
    }

    if (ImGui::CollapsingHeader("Visualization"))
    {
        ImGui::BulletText("1-4: Switch Visualization Modes");
        ImGui::BulletText("R: Reset Camera");
        ImGui::BulletText("+/=: Zoom In");
        ImGui::BulletText("-: Zoom Out");
        ImGui::BulletText("W: Toggle Wireframe");
        ImGui::BulletText("L: Toggle Lighting");
    }

    if (ImGui::CollapsingHeader("Settings"))
    {
        ImGui::BulletText("Ctrl+M: Metric Configuration");
        ImGui::BulletText("Ctrl+C: Camera Controls");
        ImGui::BulletText("Ctrl+D: Display Options");
        ImGui::BulletText("Ctrl+L: Color Scheme");
        ImGui::BulletText("Ctrl+A: Animation Controls");
        ImGui::BulletText("Ctrl+T: Toggle Theme Manager");
        ImGui::BulletText("Ctrl+,: Settings Dialog");
    }

    if (ImGui::CollapsingHeader("Dock Layout"))
    {
        ImGui::BulletText("Ctrl+Shift+S: Save Dock Layout");
        ImGui::BulletText("Ctrl+Shift+L: Load Dock Layout");
        ImGui::BulletText("Ctrl+Shift+R: Reset Dock Layout");
    }

    if (ImGui::CollapsingHeader("Themes"))
    {
        ImGui::BulletText("Ctrl+[: Previous Theme");
        ImGui::BulletText("Ctrl+]: Next Theme");
        ImGui::BulletText("Ctrl+1-6: Switch to Theme 1-6");
    }

    if (ImGui::CollapsingHeader("Help"))
    {
        ImGui::BulletText("F1: Toggle ImGui Demo Window");
        ImGui::BulletText("Shift+F1: Show Keyboard Shortcuts Dialog");
        ImGui::BulletText("H: Show Keyboard Shortcuts in Console");
    }

    if (ImGui::CollapsingHeader("Media"))
    {
        ImGui::BulletText("S: Take Screenshot");
        ImGui::BulletText("V: Toggle Video Recording");
        ImGui::BulletText("P: Toggle Performance Overlay");
    }

    ImGui::Separator();
    ImGui::TextWrapped("Tip: Most shortcuts work when the application window is focused. Some shortcuts may be overridden by the operating system.");

    ImGui::End();
}

void imgui_show_help_documentation(bool* show)
{
    if (!*show) return;

    ImGui::Begin("Documentation", show, ImGuiWindowFlags_AlwaysAutoResize);

    static int selected_section = 0;
    const char* sections[] = { "Getting Started", "Analysis Features", "Visualization", "Configuration", "Troubleshooting" };

    ImGui::BeginChild("DocumentationContent", ImVec2(600, 400), true);

    // Section selector
    ImGui::Text("Select a section:");
    for (int i = 0; i < IM_ARRAYSIZE(sections); i++)
    {
        if (ImGui::Selectable(sections[i], selected_section == i))
        {
            selected_section = i;
        }
    }

    ImGui::Separator();

    // Content based on selected section
    switch (selected_section)
    {
        case 0: // Getting Started
            ImGui::TextWrapped("Welcome to CQAnalyzer!\n\n"
                "CQAnalyzer is a comprehensive code quality analysis tool that provides 3D visualization of code metrics.\n\n"
                "To get started:\n"
                "1. Open a project using File -> Open Project\n"
                "2. Configure your analysis settings in Tools -> Settings\n"
                "3. Run analysis using Tools -> Run Analysis or F5\n"
                "4. Explore the results in the 3D visualization\n\n"
                "Use the mouse to navigate the 3D space:\n"
                "- Left click and drag to rotate\n"
                "- Right click and drag to pan\n"
                "- Scroll wheel to zoom");
            break;

        case 1: // Analysis Features
            ImGui::TextWrapped("Analysis Features:\n\n"
                "CQAnalyzer supports analysis of multiple programming languages including C, C++, Java, Python, JavaScript, and TypeScript.\n\n"
                "Available metrics:\n"
                "- Cyclomatic Complexity\n"
                "- Lines of Code (Physical and Logical)\n"
                "- Halstead Metrics (Volume, Difficulty, Effort)\n"
                "- Maintainability Index\n"
                "- Comment Density\n"
                "- Class Cohesion and Coupling\n"
                "- Dead Code Detection\n"
                "- Code Duplication Detection\n\n"
                "Results are displayed both numerically and visually in 3D space.");
            break;

        case 2: // Visualization
            ImGui::TextWrapped("Visualization Modes:\n\n"
                "CQAnalyzer offers multiple visualization modes to help you understand your code:\n\n"
                "1. Scatter Plot: Shows data points in 3D space\n"
                "2. Tree Map: Hierarchical representation\n"
                "3. Network Graph: Shows relationships between components\n"
                "4. Heat Map: Color-coded metric density\n"
                "5. Bubble Chart: Size represents additional metrics\n"
                "6. Bar Chart: 3D comparative analysis\n\n"
                "Use the View menu to switch between modes and customize display options.");
            break;

        case 3: // Configuration
            ImGui::TextWrapped("Configuration:\n\n"
                "Customize CQAnalyzer behavior through the Settings dialog (Ctrl+,):\n\n"
                "- General: Logging, performance, UI preferences\n"
                "- Analysis: Language support, parsing options\n"
                "- Visualization: Display options, color schemes\n"
                "- Export: Output formats and destinations\n\n"
                "Metric thresholds can be adjusted in the Metric Configuration panel (Ctrl+M).\n\n"
                "Settings are automatically saved and restored on startup.");
            break;

        case 4: // Troubleshooting
            ImGui::TextWrapped("Troubleshooting:\n\n"
                "Common issues and solutions:\n\n"
                "1. Application won't start:\n"
                "   - Ensure OpenGL 3.3+ support\n"
                "   - Check graphics drivers\n\n"
                "2. Analysis fails:\n"
                "   - Verify project path accessibility\n"
                "   - Check file permissions\n"
                "   - Ensure supported file types\n\n"
                "3. Poor performance:\n"
                "   - Reduce visualization complexity\n"
                "   - Enable multithreading in settings\n"
                "   - Adjust render quality\n\n"
                "4. Visualization issues:\n"
                "   - Reset camera with 'R' key\n"
                "   - Check display options\n"
                "   - Try different visualization modes\n\n"
                "For additional help, check the FAQ or contact support.");
            break;
    }

    ImGui::EndChild();

    ImGui::End();
}

void imgui_show_help_faq(bool* show)
{
    if (!*show) return;

    ImGui::Begin("Frequently Asked Questions", show, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::BeginChild("FAQContent", ImVec2(600, 400), true);

    if (ImGui::CollapsingHeader("General Questions", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::BulletText("Q: What is CQAnalyzer?");
        ImGui::TextWrapped("A: CQAnalyzer is a code quality analysis tool that provides 3D visualization of various code metrics to help developers understand and improve their codebase.");

        ImGui::BulletText("Q: Which programming languages are supported?");
        ImGui::TextWrapped("A: CQAnalyzer supports C, C++, Java, Python, JavaScript, TypeScript, and has extensible support for custom languages.");

        ImGui::BulletText("Q: Is CQAnalyzer free to use?");
        ImGui::TextWrapped("A: Yes, CQAnalyzer is open source and free to use under the MIT License.");
    }

    if (ImGui::CollapsingHeader("Analysis Questions"))
    {
        ImGui::BulletText("Q: How do I analyze my project?");
        ImGui::TextWrapped("A: Use File -> Open Project to select your project directory, then run analysis using Tools -> Run Analysis or the F5 key.");

        ImGui::BulletText("Q: What metrics does CQAnalyzer measure?");
        ImGui::TextWrapped("A: CQAnalyzer measures cyclomatic complexity, lines of code, Halstead metrics, maintainability index, comment density, and detects dead code and duplication.");

        ImGui::BulletText("Q: Can I customize metric thresholds?");
        ImGui::TextWrapped("A: Yes, you can adjust all metric thresholds in the Metric Configuration panel (Ctrl+M) to match your project's standards.");
    }

    if (ImGui::CollapsingHeader("Visualization Questions"))
    {
        ImGui::BulletText("Q: How do I navigate the 3D visualization?");
        ImGui::TextWrapped("A: Left-click and drag to rotate, right-click and drag to pan, and use the scroll wheel to zoom. Press 'R' to reset the camera.");

        ImGui::BulletText("Q: What do the different visualization modes show?");
        ImGui::TextWrapped("A: Each mode presents the same data differently - scatter plots show individual points, tree maps show hierarchy, network graphs show relationships, etc.");

        ImGui::BulletText("Q: Why is the visualization slow?");
        ImGui::TextWrapped("A: Try reducing the number of data points, enabling multithreading in settings, or lowering the render quality in display options.");
    }

    if (ImGui::CollapsingHeader("Technical Questions"))
    {
        ImGui::BulletText("Q: What are the system requirements?");
        ImGui::TextWrapped("A: CQAnalyzer requires OpenGL 3.3+ support, at least 4GB RAM, and works on Windows, macOS, and Linux.");

        ImGui::BulletText("Q: Can I export analysis results?");
        ImGui::TextWrapped("A: Yes, CQAnalyzer supports export to CSV, JSON, XML, and HTML formats. Configure export options in the Settings dialog.");

        ImGui::BulletText("Q: How do I report bugs or request features?");
        ImGui::TextWrapped("A: Please use the GitHub issue tracker at https://github.com/cqanalyzer/cqanalyzer/issues");
    }

    ImGui::EndChild();

    ImGui::End();
}

void imgui_show_help_system_info(bool* show)
{
    if (!*show) return;

    ImGui::Begin("System Information", show, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("CQAnalyzer System Information");
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Application", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::BulletText("Version: 1.0.0");
        ImGui::BulletText("Build Date: %s", __DATE__ " " __TIME__);
        ImGui::BulletText("Architecture: %s", sizeof(void*) == 8 ? "64-bit" : "32-bit");
        ImGui::BulletText("Platform: Linux");
    }

    if (ImGui::CollapsingHeader("System Resources"))
    {
        // Get system information (simplified for demo)
        ImGui::BulletText("Operating System: Linux");
        ImGui::BulletText("Kernel Version: %s", "5.14.0"); // Would get from uname in real implementation
        ImGui::BulletText("CPU Cores: %d", 4); // Would get from sysconf in real implementation
        ImGui::BulletText("Total Memory: %d MB", 8192); // Would get from sysinfo in real implementation
        ImGui::BulletText("Available Memory: %d MB", 4096); // Would get from sysinfo in real implementation
    }

    if (ImGui::CollapsingHeader("Graphics"))
    {
        ImGui::BulletText("OpenGL Version: 3.3");
        ImGui::BulletText("GLFW Version: 3.3");
        ImGui::BulletText("GLEW Version: 2.1");
        ImGui::BulletText("Renderer: %s", glGetString(GL_RENDERER));
        ImGui::BulletText("Vendor: %s", glGetString(GL_VENDOR));
    }

    if (ImGui::CollapsingHeader("Libraries"))
    {
        ImGui::BulletText("Dear ImGui: 1.89");
        ImGui::BulletText("GLM: 0.9.9");
        ImGui::BulletText("C Standard Library: C99");
    }

    if (ImGui::CollapsingHeader("Configuration"))
    {
        ImGui::BulletText("Config File: cqanalyzer.conf");
        ImGui::BulletText("Settings File: cqanalyzer_settings.ini");
        ImGui::BulletText("Log File: cqanalyzer.log");
        ImGui::BulletText("Cache Directory: ./cache");
    }

    ImGui::Separator();
    ImGui::TextWrapped("This information is useful for troubleshooting and support requests. You can copy this information when reporting issues.");

    if (ImGui::Button("Copy to Clipboard"))
    {
        // TODO: Implement clipboard copy functionality
        printf("System info copied to clipboard\n");
    }

    ImGui::SameLine();
    if (ImGui::Button("Save to File"))
    {
        // TODO: Implement save to file functionality
        printf("System info saved to file\n");
    }

    ImGui::End();
}

// Export dialog implementation
void imgui_show_export_dialog(bool* show)
{
    if (!*show) return;

    ImGui::OpenPopup("Export Data");

    if (ImGui::BeginPopupModal("Export Data", show, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static int selected_format = 0;
        const char* formats[] = { "CSV", "JSON", "XML", "HTML", "PDF" };

        ImGui::Text("Select export format:");
        ImGui::Separator();

        ImGui::Combo("Format", &selected_format, formats, IM_ARRAYSIZE(formats));

        ImGui::Separator();

        // Format-specific options
        switch (selected_format)
        {
            case 0: // CSV
                ImGui::Text("CSV Export Options:");
                ImGui::Checkbox("Include headers", &menu_state.export_settings.include_timestamps);
                ImGui::Checkbox("Custom columns", &menu_state.export_settings.enable_csv_export);
                break;
            case 1: // JSON
                ImGui::Text("JSON Export Options:");
                ImGui::Checkbox("Pretty print", &menu_state.export_settings.enable_json_export);
                ImGui::Checkbox("Include metadata", &menu_state.export_settings.include_timestamps);
                break;
            case 2: // XML
                ImGui::Text("XML Export Options:");
                ImGui::Checkbox("Include schema", &menu_state.export_settings.enable_xml_export);
                ImGui::Checkbox("Validate output", &menu_state.export_settings.include_timestamps);
                break;
            case 3: // HTML
                ImGui::Text("HTML Export Options:");
                ImGui::Checkbox("Include charts", &menu_state.export_settings.enable_html_export);
                ImGui::Checkbox("Responsive design", &menu_state.export_settings.include_timestamps);
                break;
            case 4: // PDF
                ImGui::Text("PDF Export Options:");
                ImGui::Checkbox("High quality", &menu_state.export_settings.enable_pdf_export);
                ImGui::Checkbox("Include images", &menu_state.export_settings.include_timestamps);
                break;
        }

        ImGui::Separator();

        // Export destination
        static char export_path[4096] = "";
        if (strlen(export_path) == 0) {
            strcpy(export_path, menu_state.export_settings.default_export_path);
        }

        ImGui::InputText("Export Path", export_path, sizeof(export_path));
        ImGui::SameLine();
        if (ImGui::Button("Browse...")) {
            menu_state.show_file_browser = true;
        }

        ImGui::Separator();

        // Dialog buttons
        if (ImGui::Button("Export", ImVec2(80, 0)))
        {
            // TODO: Implement actual export based on selected format
            printf("Exporting to %s format: %s\n", formats[selected_format], export_path);
            *show = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0)))
        {
            *show = false;
        }

        ImGui::EndPopup();
    }
}

void imgui_show_visualization_settings(bool* show)
{
    if (!*show) return;

    ImGui::Begin("Visualization Settings", show, ImGuiWindowFlags_None);

    static float point_size = 5.0f;
    static float line_width = 2.0f;
    static bool enable_lighting = true;
    static bool enable_wireframe = false;

    ImGui::SliderFloat("Point Size", &point_size, 1.0f, 20.0f);
    ImGui::SliderFloat("Line Width", &line_width, 1.0f, 10.0f);
    ImGui::Checkbox("Enable Lighting", &enable_lighting);
    ImGui::Checkbox("Wireframe Mode", &enable_wireframe);

    if (ImGui::Button("Reset to Defaults")) {
        point_size = 5.0f;
        line_width = 2.0f;
        enable_lighting = true;
        enable_wireframe = false;
    }

    ImGui::End();
}

// Settings dialog implementation
void imgui_show_settings_dialog(bool* show)
{
    if (!*show) return;

    ImGui::OpenPopup("Settings");

    if (ImGui::BeginPopupModal("Settings", show, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static int current_tab = 0;
        const char* tabs[] = { "General", "Analysis", "Visualization", "Export" };

        // Tab bar
        if (ImGui::BeginTabBar("SettingsTabs"))
        {
            for (int i = 0; i < IM_ARRAYSIZE(tabs); i++)
            {
                if (ImGui::BeginTabItem(tabs[i]))
                {
                    current_tab = i;
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::Separator();

        // Tab content
        switch (current_tab)
        {
            case 0: // General tab
                imgui_show_general_settings_tab();
                break;
            case 1: // Analysis tab
                imgui_show_analysis_settings_tab();
                break;
            case 2: // Visualization tab
                imgui_show_visualization_settings_tab();
                break;
            case 3: // Export tab
                imgui_show_export_settings_tab();
                break;
        }

        ImGui::Separator();

        // Dialog buttons
        if (ImGui::Button("Apply", ImVec2(80, 0)))
        {
            // Apply settings to the running system
            // TODO: Implement actual application of settings
            printf("Settings applied\n");
        }

        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(80, 0)))
        {
            if (imgui_save_settings("cqanalyzer_settings.ini")) {
                printf("Settings saved successfully\n");
            } else {
                printf("Failed to save settings\n");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Load", ImVec2(80, 0)))
        {
            if (imgui_load_settings("cqanalyzer_settings.ini")) {
                printf("Settings loaded successfully\n");
            } else {
                printf("Failed to load settings\n");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset to Defaults", ImVec2(120, 0)))
        {
            // Reset to defaults
            menu_state_init();
            printf("Settings reset to defaults\n");
        }

        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(80, 0)))
        {
            *show = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

// Settings save/load functionality
bool imgui_save_settings(const char* filename)
{
    FILE* file = fopen(filename, "w");
    if (!file) {
        LOG_ERROR("Failed to open settings file for writing: %s", filename);
        return false;
    }

    // Save general settings
    fprintf(file, "[General]\n");
    fprintf(file, "log_level=%d\n", menu_state.general_settings.log_level);
    fprintf(file, "log_to_file=%d\n", menu_state.general_settings.log_to_file);
    fprintf(file, "log_file_path=%s\n", menu_state.general_settings.log_file_path);
    fprintf(file, "log_timestamps=%d\n", menu_state.general_settings.log_timestamps);
    fprintf(file, "max_log_file_size=%d\n", menu_state.general_settings.max_log_file_size);
    fprintf(file, "max_threads=%d\n", menu_state.general_settings.max_threads);
    fprintf(file, "enable_multithreading=%d\n", menu_state.general_settings.enable_multithreading);
    fprintf(file, "cache_size_mb=%d\n", menu_state.general_settings.cache_size_mb);
    fprintf(file, "enable_gpu_acceleration=%d\n", menu_state.general_settings.enable_gpu_acceleration);
    fprintf(file, "theme=%d\n", menu_state.general_settings.theme);
    fprintf(file, "ui_scale=%.2f\n", menu_state.general_settings.ui_scale);
    fprintf(file, "show_tooltips=%d\n", menu_state.general_settings.show_tooltips);
    fprintf(file, "auto_save_settings=%d\n", menu_state.general_settings.auto_save_settings);
    fprintf(file, "auto_save_interval=%d\n", menu_state.general_settings.auto_save_interval);

    // Save analysis settings
    fprintf(file, "\n[Analysis]\n");
    fprintf(file, "enable_incremental_parsing=%d\n", menu_state.analysis_settings.enable_incremental_parsing);
    fprintf(file, "max_file_size_mb=%d\n", menu_state.analysis_settings.max_file_size_mb);
    fprintf(file, "follow_symbolic_links=%d\n", menu_state.analysis_settings.follow_symbolic_links);
    fprintf(file, "parsing_timeout_seconds=%d\n", menu_state.analysis_settings.parsing_timeout_seconds);
    fprintf(file, "enable_c_support=%d\n", menu_state.analysis_settings.enable_c_support);
    fprintf(file, "enable_cpp_support=%d\n", menu_state.analysis_settings.enable_cpp_support);
    fprintf(file, "enable_java_support=%d\n", menu_state.analysis_settings.enable_java_support);
    fprintf(file, "enable_python_support=%d\n", menu_state.analysis_settings.enable_python_support);
    fprintf(file, "enable_javascript_support=%d\n", menu_state.analysis_settings.enable_javascript_support);
    fprintf(file, "enable_typescript_support=%d\n", menu_state.analysis_settings.enable_typescript_support);
    fprintf(file, "enable_custom_languages=%d\n", menu_state.analysis_settings.enable_custom_languages);
    fprintf(file, "enable_parallel_analysis=%d\n", menu_state.analysis_settings.enable_parallel_analysis);
    fprintf(file, "analysis_batch_size=%d\n", menu_state.analysis_settings.analysis_batch_size);
    fprintf(file, "enable_caching=%d\n", menu_state.analysis_settings.enable_caching);

    // Save export settings
    fprintf(file, "\n[Export]\n");
    fprintf(file, "enable_csv_export=%d\n", menu_state.export_settings.enable_csv_export);
    fprintf(file, "enable_json_export=%d\n", menu_state.export_settings.enable_json_export);
    fprintf(file, "enable_xml_export=%d\n", menu_state.export_settings.enable_xml_export);
    fprintf(file, "enable_html_export=%d\n", menu_state.export_settings.enable_html_export);
    fprintf(file, "enable_pdf_export=%d\n", menu_state.export_settings.enable_pdf_export);
    fprintf(file, "default_export_path=%s\n", menu_state.export_settings.default_export_path);
    fprintf(file, "auto_open_after_export=%d\n", menu_state.export_settings.auto_open_after_export);
    fprintf(file, "include_timestamps=%d\n", menu_state.export_settings.include_timestamps);
    fprintf(file, "csv_template=%s\n", menu_state.export_settings.csv_template);
    fprintf(file, "json_template=%s\n", menu_state.export_settings.json_template);
    fprintf(file, "html_template=%s\n", menu_state.export_settings.html_template);

    fclose(file);
    LOG_INFO("Settings saved to: %s", filename);
    return true;
}

bool imgui_load_settings(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        LOG_ERROR("Failed to open settings file for reading: %s", filename);
        return false;
    }

    char line[512];
    char section[64] = "";

    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;

        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#') continue;

        // Check for section headers
        if (line[0] == '[' && line[strlen(line) - 1] == ']') {
            strncpy(section, line + 1, sizeof(section) - 1);
            section[strlen(section) - 1] = '\0'; // Remove closing bracket
            continue;
        }

        // Parse key-value pairs
        char* equals = strchr(line, '=');
        if (!equals) continue;

        *equals = '\0';
        char* key = line;
        char* value = equals + 1;

        // Parse based on section
        if (strcmp(section, "General") == 0) {
            if (strcmp(key, "log_level") == 0) menu_state.general_settings.log_level = atoi(value);
            else if (strcmp(key, "log_to_file") == 0) menu_state.general_settings.log_to_file = atoi(value);
            else if (strcmp(key, "log_file_path") == 0) strncpy(menu_state.general_settings.log_file_path, value, sizeof(menu_state.general_settings.log_file_path));
            else if (strcmp(key, "log_timestamps") == 0) menu_state.general_settings.log_timestamps = atoi(value);
            else if (strcmp(key, "max_log_file_size") == 0) menu_state.general_settings.max_log_file_size = atoi(value);
            else if (strcmp(key, "max_threads") == 0) menu_state.general_settings.max_threads = atoi(value);
            else if (strcmp(key, "enable_multithreading") == 0) menu_state.general_settings.enable_multithreading = atoi(value);
            else if (strcmp(key, "cache_size_mb") == 0) menu_state.general_settings.cache_size_mb = atoi(value);
            else if (strcmp(key, "enable_gpu_acceleration") == 0) menu_state.general_settings.enable_gpu_acceleration = atoi(value);
            else if (strcmp(key, "theme") == 0) menu_state.general_settings.theme = atoi(value);
            else if (strcmp(key, "ui_scale") == 0) menu_state.general_settings.ui_scale = atof(value);
            else if (strcmp(key, "show_tooltips") == 0) menu_state.general_settings.show_tooltips = atoi(value);
            else if (strcmp(key, "auto_save_settings") == 0) menu_state.general_settings.auto_save_settings = atoi(value);
            else if (strcmp(key, "auto_save_interval") == 0) menu_state.general_settings.auto_save_interval = atoi(value);
        }
        else if (strcmp(section, "Analysis") == 0) {
            if (strcmp(key, "enable_incremental_parsing") == 0) menu_state.analysis_settings.enable_incremental_parsing = atoi(value);
            else if (strcmp(key, "max_file_size_mb") == 0) menu_state.analysis_settings.max_file_size_mb = atoi(value);
            else if (strcmp(key, "follow_symbolic_links") == 0) menu_state.analysis_settings.follow_symbolic_links = atoi(value);
            else if (strcmp(key, "parsing_timeout_seconds") == 0) menu_state.analysis_settings.parsing_timeout_seconds = atoi(value);
            else if (strcmp(key, "enable_c_support") == 0) menu_state.analysis_settings.enable_c_support = atoi(value);
            else if (strcmp(key, "enable_cpp_support") == 0) menu_state.analysis_settings.enable_cpp_support = atoi(value);
            else if (strcmp(key, "enable_java_support") == 0) menu_state.analysis_settings.enable_java_support = atoi(value);
            else if (strcmp(key, "enable_python_support") == 0) menu_state.analysis_settings.enable_python_support = atoi(value);
            else if (strcmp(key, "enable_javascript_support") == 0) menu_state.analysis_settings.enable_javascript_support = atoi(value);
            else if (strcmp(key, "enable_typescript_support") == 0) menu_state.analysis_settings.enable_typescript_support = atoi(value);
            else if (strcmp(key, "enable_custom_languages") == 0) menu_state.analysis_settings.enable_custom_languages = atoi(value);
            else if (strcmp(key, "enable_parallel_analysis") == 0) menu_state.analysis_settings.enable_parallel_analysis = atoi(value);
            else if (strcmp(key, "analysis_batch_size") == 0) menu_state.analysis_settings.analysis_batch_size = atoi(value);
            else if (strcmp(key, "enable_caching") == 0) menu_state.analysis_settings.enable_caching = atoi(value);
        }
        else if (strcmp(section, "Export") == 0) {
            if (strcmp(key, "enable_csv_export") == 0) menu_state.export_settings.enable_csv_export = atoi(value);
            else if (strcmp(key, "enable_json_export") == 0) menu_state.export_settings.enable_json_export = atoi(value);
            else if (strcmp(key, "enable_xml_export") == 0) menu_state.export_settings.enable_xml_export = atoi(value);
            else if (strcmp(key, "enable_html_export") == 0) menu_state.export_settings.enable_html_export = atoi(value);
            else if (strcmp(key, "enable_pdf_export") == 0) menu_state.export_settings.enable_pdf_export = atoi(value);
            else if (strcmp(key, "default_export_path") == 0) strncpy(menu_state.export_settings.default_export_path, value, sizeof(menu_state.export_settings.default_export_path));
            else if (strcmp(key, "auto_open_after_export") == 0) menu_state.export_settings.auto_open_after_export = atoi(value);
            else if (strcmp(key, "include_timestamps") == 0) menu_state.export_settings.include_timestamps = atoi(value);
            else if (strcmp(key, "csv_template") == 0) strncpy(menu_state.export_settings.csv_template, value, sizeof(menu_state.export_settings.csv_template));
            else if (strcmp(key, "json_template") == 0) strncpy(menu_state.export_settings.json_template, value, sizeof(menu_state.export_settings.json_template));
            else if (strcmp(key, "html_template") == 0) strncpy(menu_state.export_settings.html_template, value, sizeof(menu_state.export_settings.html_template));
        }
    }

    fclose(file);
    LOG_INFO("Settings loaded from: %s", filename);
    return true;
}

// General settings tab
void imgui_show_general_settings_tab(void)
{
    GeneralSettings* settings = &menu_state.general_settings;

    if (ImGui::CollapsingHeader("Logging", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const char* log_levels[] = { "ERROR", "WARN", "INFO", "DEBUG" };
        ImGui::Combo("Log Level", &settings->log_level, log_levels, IM_ARRAYSIZE(log_levels));

        ImGui::Checkbox("Log to File", &settings->log_to_file);
        if (settings->log_to_file)
        {
            ImGui::InputText("Log File Path", settings->log_file_path, sizeof(settings->log_file_path));
        }

        ImGui::Checkbox("Include Timestamps", &settings->log_timestamps);
        ImGui::SliderInt("Max Log File Size (MB)", &settings->max_log_file_size, 1, 100);
    }

    if (ImGui::CollapsingHeader("Performance"))
    {
        ImGui::SliderInt("Max Threads", &settings->max_threads, 1, 16);
        ImGui::Checkbox("Enable Multithreading", &settings->enable_multithreading);
        ImGui::SliderInt("Cache Size (MB)", &settings->cache_size_mb, 10, 1000);
        ImGui::Checkbox("Enable GPU Acceleration", &settings->enable_gpu_acceleration);
    }

    if (ImGui::CollapsingHeader("UI Preferences"))
    {
        const char* themes[] = { "Dark", "Light", "System" };
        ImGui::Combo("Theme", &settings->theme, themes, IM_ARRAYSIZE(themes));

        ImGui::SliderFloat("UI Scale", &settings->ui_scale, 0.5f, 2.0f, "%.2f");
        ImGui::Checkbox("Show Tooltips", &settings->show_tooltips);
        ImGui::Checkbox("Auto-save Settings", &settings->auto_save_settings);

        if (settings->auto_save_settings)
        {
            ImGui::SliderInt("Auto-save Interval (minutes)", &settings->auto_save_interval, 1, 60);
        }
    }

    if (ImGui::CollapsingHeader("Theme System"))
    {
        ImGui::Text("Current Theme: %s", imgui_get_current_theme_name());

        if (ImGui::Button("Open Theme Manager")) {
            menu_state.show_theme_panel = true;
        }

        ImGui::Separator();
        ImGui::Text("Quick Theme Switch:");

        int theme_count = imgui_get_theme_count();
        for (int i = 0; i < theme_count && i < 6; i++) {
            if (ImGui::Button(imgui_get_theme_name(i))) {
                imgui_apply_theme(i);
            }
            if ((i + 1) % 3 != 0) ImGui::SameLine();
        }

        ImGui::Separator();
        ImGui::Text("Theme Shortcuts:");
        ImGui::BulletText("Ctrl+T: Toggle Theme Manager");
        ImGui::BulletText("Ctrl+[/]: Previous/Next Theme");
        ImGui::BulletText("Ctrl+1-6: Switch to Theme 1-6");
    }
}

// Analysis settings tab
void imgui_show_analysis_settings_tab(void)
{
    AnalysisSettings* settings = &menu_state.analysis_settings;

    if (ImGui::CollapsingHeader("Parsing Options", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Enable Incremental Parsing", &settings->enable_incremental_parsing);
        ImGui::SliderInt("Max File Size (MB)", &settings->max_file_size_mb, 1, 500);
        ImGui::Checkbox("Follow Symbolic Links", &settings->follow_symbolic_links);
        ImGui::SliderInt("Parsing Timeout (seconds)", &settings->parsing_timeout_seconds, 5, 300);
    }

    if (ImGui::CollapsingHeader("Language Support"))
    {
        ImGui::Checkbox("C Support", &settings->enable_c_support);
        ImGui::Checkbox("C++ Support", &settings->enable_cpp_support);
        ImGui::Checkbox("Java Support", &settings->enable_java_support);
        ImGui::Checkbox("Python Support", &settings->enable_python_support);
        ImGui::Checkbox("JavaScript Support", &settings->enable_javascript_support);
        ImGui::Checkbox("TypeScript Support", &settings->enable_typescript_support);
        ImGui::Checkbox("Custom Languages", &settings->enable_custom_languages);
    }

    if (ImGui::CollapsingHeader("Analysis Options"))
    {
        ImGui::Checkbox("Enable Parallel Analysis", &settings->enable_parallel_analysis);
        if (settings->enable_parallel_analysis)
        {
            ImGui::SliderInt("Analysis Batch Size", &settings->analysis_batch_size, 1, 100);
        }
        ImGui::Checkbox("Enable Caching", &settings->enable_caching);
    }
}

// Visualization settings tab
void imgui_show_visualization_settings_tab(void)
{
    if (ImGui::CollapsingHeader("Display Options", ImGuiTreeNodeFlags_DefaultOpen))
    {
        DisplayOptions* opts = &menu_state.display_options;
        ImGui::Checkbox("Show Axes", &opts->show_axes);
        ImGui::Checkbox("Show Grid", &opts->show_grid);
        ImGui::Checkbox("Show Labels", &opts->show_labels);
        ImGui::Checkbox("Show Bounding Box", &opts->show_bounding_box);
        ImGui::Checkbox("Wireframe Mode", &opts->show_wireframe);
        ImGui::Checkbox("Enable Lighting", &opts->enable_lighting);
        ImGui::Checkbox("Enable Shadows", &opts->enable_shadows);
        ImGui::Checkbox("Enable Fog", &opts->enable_fog);

        const char* quality_options[] = { "Low", "Medium", "High" };
        ImGui::Combo("Render Quality", &opts->render_quality, quality_options, IM_ARRAYSIZE(quality_options));

        ImGui::SliderFloat("Point Size", &opts->point_size, 1.0f, 20.0f, "%.1f");
        ImGui::SliderFloat("Line Width", &opts->line_width, 1.0f, 10.0f, "%.1f");
        ImGui::SliderFloat("Label Scale", &opts->label_scale, 0.1f, 5.0f, "%.2f");
    }

    if (ImGui::CollapsingHeader("Color Scheme"))
    {
        const char* scheme_names[10];
        for (int i = 0; i < menu_state.num_color_schemes; i++) {
            scheme_names[i] = menu_state.color_schemes[i].name;
        }

        ImGui::Combo("Current Scheme", &menu_state.current_color_scheme,
                      scheme_names, menu_state.num_color_schemes);

        ColorScheme* scheme = &menu_state.color_schemes[menu_state.current_color_scheme];
        ImGui::ColorEdit4("Background", scheme->background_color);
        ImGui::ColorEdit4("Grid", scheme->grid_color);
        ImGui::ColorEdit4("Axes", scheme->axis_color);
        ImGui::ColorEdit4("Points", scheme->point_color);
        ImGui::ColorEdit4("Lines", scheme->line_color);
        ImGui::ColorEdit4("Text", scheme->text_color);
        ImGui::ColorEdit4("Highlight", scheme->highlight_color);
    }

    if (ImGui::CollapsingHeader("Animation"))
    {
        AnimationControls* anim = &menu_state.animation_controls;
        ImGui::Checkbox("Enable Animation", &anim->enabled);
        ImGui::SliderFloat("Duration", &anim->duration, 0.5f, 10.0f, "%.1f s");
        ImGui::SliderFloat("Speed", &anim->speed, 0.1f, 5.0f, "%.1f x");
        ImGui::Checkbox("Loop", &anim->loop);

        const char* easing_options[] = { "Linear", "Ease In", "Ease Out", "Ease In-Out" };
        ImGui::Combo("Easing", &anim->easing_type, easing_options, IM_ARRAYSIZE(easing_options));

        ImGui::Checkbox("Auto Rotate", &anim->auto_rotate);
        if (anim->auto_rotate) {
            ImGui::SliderFloat("Rotation Speed", &anim->auto_rotate_speed, 0.1f, 2.0f, "%.1f");
        }
    }
}

// Export settings tab
void imgui_show_export_settings_tab(void)
{
    ExportSettings* settings = &menu_state.export_settings;

    if (ImGui::CollapsingHeader("Export Formats", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("CSV Export", &settings->enable_csv_export);
        ImGui::Checkbox("JSON Export", &settings->enable_json_export);
        ImGui::Checkbox("XML Export", &settings->enable_xml_export);
        ImGui::Checkbox("HTML Export", &settings->enable_html_export);
        ImGui::Checkbox("PDF Export", &settings->enable_pdf_export);
    }

    if (ImGui::CollapsingHeader("Export Destinations"))
    {
        ImGui::InputText("Default Export Path", settings->default_export_path, sizeof(settings->default_export_path));
        ImGui::Checkbox("Auto-open After Export", &settings->auto_open_after_export);
        ImGui::Checkbox("Include Timestamps", &settings->include_timestamps);
    }

    if (ImGui::CollapsingHeader("Templates"))
    {
        if (settings->enable_csv_export) {
            ImGui::InputText("CSV Template", settings->csv_template, sizeof(settings->csv_template));
        }
        if (settings->enable_json_export) {
            ImGui::InputText("JSON Template", settings->json_template, sizeof(settings->json_template));
        }
        if (settings->enable_html_export) {
            ImGui::InputText("HTML Template", settings->html_template, sizeof(settings->html_template));
        }
    }
}

// Integration functions for applying control panel settings

void imgui_apply_camera_settings(void)
{
    // This function would be called by the renderer to apply camera settings
    // For now, it's a placeholder for the integration framework
    CameraControls* cam = &menu_state.camera_controls;

    // TODO: Apply to actual camera system
    // camera_set_position(&renderer_camera, cam->position[0], cam->position[1], cam->position[2]);
    // camera_set_target(&renderer_camera, cam->target[0], cam->target[1], cam->target[2]);
    // camera_set_fov(&renderer_camera, cam->fov);
    // camera_set_near_far(&renderer_camera, cam->near_plane, cam->far_plane);

    // Apply yaw/pitch/distance transformations
    // This would require spherical to cartesian conversion
}

void imgui_apply_display_settings(void)
{
    // Apply display options to rendering system
    DisplayOptions* opts = &menu_state.display_options;

    // TODO: Apply to actual rendering system
    // renderer_set_wireframe(opts->show_wireframe);
    // renderer_set_lighting(opts->enable_lighting);
    // renderer_set_shadows(opts->enable_shadows);
    // renderer_set_fog(opts->enable_fog);
    // renderer_set_point_size(opts->point_size);
    // renderer_set_line_width(opts->line_width);
    // renderer_set_label_scale(opts->label_scale);
    // renderer_set_render_quality(opts->render_quality);
}

void imgui_apply_color_scheme(void)
{
    // Apply current color scheme to rendering system
    ColorScheme* scheme = &menu_state.color_schemes[menu_state.current_color_scheme];

    // TODO: Apply to actual rendering system
    // renderer_set_background_color(scheme->background_color);
    // renderer_set_grid_color(scheme->grid_color);
    // renderer_set_axis_color(scheme->axis_color);
    // renderer_set_point_color(scheme->point_color);
    // renderer_set_line_color(scheme->line_color);
    // renderer_set_text_color(scheme->text_color);
    // renderer_set_highlight_color(scheme->highlight_color);
}

int imgui_get_visualization_mode(void)
{
    return menu_state.visualization_mode;
}

bool imgui_get_display_option(const char* option_name)
{
    DisplayOptions* opts = &menu_state.display_options;

    if (strcmp(option_name, "show_axes") == 0) return opts->show_axes;
    if (strcmp(option_name, "show_grid") == 0) return opts->show_grid;
    if (strcmp(option_name, "show_labels") == 0) return opts->show_labels;
    if (strcmp(option_name, "show_bounding_box") == 0) return opts->show_bounding_box;
    if (strcmp(option_name, "show_wireframe") == 0) return opts->show_wireframe;
    if (strcmp(option_name, "enable_lighting") == 0) return opts->enable_lighting;
    if (strcmp(option_name, "enable_shadows") == 0) return opts->enable_shadows;
    if (strcmp(option_name, "enable_fog") == 0) return opts->enable_fog;

    return false;
}

// Camera control panel implementation
void imgui_show_camera_control_panel(bool* show)
{
    if (!*show) return;

    ImGui::Begin("Camera Controls", show, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_None);

    CameraControls* cam = &menu_state.camera_controls;
    static CameraControls prev_cam; // Track previous values for change detection
    static bool first_frame = true;

    if (first_frame) {
        memcpy(&prev_cam, cam, sizeof(CameraControls));
        first_frame = false;
    }

    bool camera_changed = false;

    if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Camera Position");
        camera_changed |= ImGui::SliderFloat("X", &cam->position[0], -20.0f, 20.0f, "%.2f");
        camera_changed |= ImGui::SliderFloat("Y", &cam->position[1], -20.0f, 20.0f, "%.2f");
        camera_changed |= ImGui::SliderFloat("Z", &cam->position[2], -20.0f, 20.0f, "%.2f");

        ImGui::Separator();
        ImGui::Text("Target Position");
        camera_changed |= ImGui::SliderFloat("Target X", &cam->target[0], -10.0f, 10.0f, "%.2f");
        camera_changed |= ImGui::SliderFloat("Target Y", &cam->target[1], -10.0f, 10.0f, "%.2f");
        camera_changed |= ImGui::SliderFloat("Target Z", &cam->target[2], -10.0f, 10.0f, "%.2f");
    }

    if (ImGui::CollapsingHeader("Rotation")) {
        camera_changed |= ImGui::SliderFloat("Yaw", &cam->yaw, -180.0f, 180.0f, "%.1f°");
        camera_changed |= ImGui::SliderFloat("Pitch", &cam->pitch, -90.0f, 90.0f, "%.1f°");
        camera_changed |= ImGui::SliderFloat("Distance", &cam->distance, 1.0f, 50.0f, "%.2f");
    }

    if (ImGui::CollapsingHeader("Projection")) {
        camera_changed |= ImGui::SliderFloat("Field of View", &cam->fov, 10.0f, 120.0f, "%.1f°");
        camera_changed |= ImGui::SliderFloat("Near Plane", &cam->near_plane, 0.01f, 1.0f, "%.3f");
        camera_changed |= ImGui::SliderFloat("Far Plane", &cam->far_plane, 10.0f, 1000.0f, "%.1f");
    }

    ImGui::Separator();

    if (ImGui::Button("Reset to Default")) {
        cam->position[0] = 0.0f;
        cam->position[1] = 0.0f;
        cam->position[2] = 5.0f;
        cam->target[0] = 0.0f;
        cam->target[1] = 0.0f;
        cam->target[2] = 0.0f;
        cam->up[0] = 0.0f;
        cam->up[1] = 1.0f;
        cam->up[2] = 0.0f;
        cam->yaw = 0.0f;
        cam->pitch = 0.0f;
        cam->distance = 5.0f;
        cam->fov = 45.0f;
        cam->near_plane = 0.1f;
        cam->far_plane = 100.0f;
        camera_changed = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Top View")) {
        cam->position[0] = 0.0f;
        cam->position[1] = 10.0f;
        cam->position[2] = 0.0f;
        cam->target[0] = 0.0f;
        cam->target[1] = 0.0f;
        cam->target[2] = 0.0f;
        cam->up[0] = 0.0f;
        cam->up[1] = 0.0f;
        cam->up[2] = -1.0f;
        cam->yaw = 0.0f;
        cam->pitch = -90.0f;
        cam->distance = 10.0f;
        camera_changed = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Side View")) {
        cam->position[0] = 10.0f;
        cam->position[1] = 0.0f;
        cam->position[2] = 0.0f;
        cam->target[0] = 0.0f;
        cam->target[1] = 0.0f;
        cam->target[2] = 0.0f;
        cam->up[0] = 0.0f;
        cam->up[1] = 1.0f;
        cam->up[2] = 0.0f;
        cam->yaw = -90.0f;
        cam->pitch = 0.0f;
        cam->distance = 10.0f;
        camera_changed = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Front View")) {
        cam->position[0] = 0.0f;
        cam->position[1] = 0.0f;
        cam->position[2] = 10.0f;
        cam->target[0] = 0.0f;
        cam->target[1] = 0.0f;
        cam->target[2] = 0.0f;
        cam->up[0] = 0.0f;
        cam->up[1] = 1.0f;
        cam->up[2] = 0.0f;
        cam->yaw = 0.0f;
        cam->pitch = 0.0f;
        cam->distance = 10.0f;
        camera_changed = true;
    }

    // Apply camera changes if any occurred
    if (camera_changed) {
        // Apply camera changes to the actual camera system
        // Note: This requires access to the camera instance from renderer.c
        // For now, we'll prepare the integration framework
        printf("Camera updated: Pos(%.2f, %.2f, %.2f) Target(%.2f, %.2f, %.2f) Yaw:%.1f Pitch:%.1f Distance:%.2f\n",
               cam->position[0], cam->position[1], cam->position[2],
               cam->target[0], cam->target[1], cam->target[2],
               cam->yaw, cam->pitch, cam->distance);

        // TODO: Integrate with actual camera system
        // camera_set_position(&renderer_camera, cam->position[0], cam->position[1], cam->position[2]);
        // camera_set_target(&renderer_camera, cam->target[0], cam->target[1], cam->target[2]);
        // Apply yaw/pitch/distance transformations
    }

    ImGui::End();
}

// Display options panel implementation
void imgui_show_display_options_panel(bool* show)
{
    if (!*show) return;

    ImGui::Begin("Display Options", show, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_None);

    DisplayOptions* opts = &menu_state.display_options;

    if (ImGui::CollapsingHeader("Visibility", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Show Axes", &opts->show_axes);
        ImGui::Checkbox("Show Grid", &opts->show_grid);
        ImGui::Checkbox("Show Labels", &opts->show_labels);
        ImGui::Checkbox("Show Bounding Box", &opts->show_bounding_box);
        ImGui::Checkbox("Wireframe Mode", &opts->show_wireframe);
    }

    if (ImGui::CollapsingHeader("Rendering")) {
        ImGui::Checkbox("Enable Lighting", &opts->enable_lighting);
        ImGui::Checkbox("Enable Shadows", &opts->enable_shadows);
        ImGui::Checkbox("Enable Fog", &opts->enable_fog);

        const char* quality_options[] = { "Low", "Medium", "High" };
        ImGui::Combo("Render Quality", &opts->render_quality, quality_options, IM_ARRAYSIZE(quality_options));
    }

    if (ImGui::CollapsingHeader("Size & Scale")) {
        ImGui::SliderFloat("Point Size", &opts->point_size, 1.0f, 20.0f, "%.1f");
        ImGui::SliderFloat("Line Width", &opts->line_width, 1.0f, 10.0f, "%.1f");
        ImGui::SliderFloat("Label Scale", &opts->label_scale, 0.1f, 5.0f, "%.2f");
    }

    ImGui::Separator();

    if (ImGui::Button("Reset to Defaults")) {
        opts->show_axes = true;
        opts->show_grid = true;
        opts->show_labels = true;
        opts->show_bounding_box = false;
        opts->show_wireframe = false;
        opts->enable_lighting = true;
        opts->enable_shadows = false;
        opts->enable_fog = false;
        opts->point_size = 5.0f;
        opts->line_width = 2.0f;
        opts->label_scale = 1.0f;
        opts->render_quality = 1;
    }

    ImGui::End();
}

// Color scheme panel implementation
void imgui_show_color_scheme_panel(bool* show)
{
    if (!*show) return;

    ImGui::Begin("Color Scheme", show, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_None);

    // Color scheme selection
    if (ImGui::CollapsingHeader("Scheme Selection", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* scheme_names[10];
        for (int i = 0; i < menu_state.num_color_schemes; i++) {
            scheme_names[i] = menu_state.color_schemes[i].name;
        }

        ImGui::Combo("Current Scheme", &menu_state.current_color_scheme,
                     scheme_names, menu_state.num_color_schemes);

        ImGui::Separator();

        if (ImGui::Button("Create New Scheme")) {
            if (menu_state.num_color_schemes < 10) {
                int idx = menu_state.num_color_schemes;
                sprintf(menu_state.color_schemes[idx].name, "Custom %d", idx + 1);
                // Copy current scheme as base
                memcpy(&menu_state.color_schemes[idx],
                       &menu_state.color_schemes[menu_state.current_color_scheme],
                       sizeof(ColorScheme));
                menu_state.num_color_schemes++;
                menu_state.current_color_scheme = idx;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Delete Scheme") && menu_state.num_color_schemes > 1) {
            // Remove current scheme and shift others
            for (int i = menu_state.current_color_scheme; i < menu_state.num_color_schemes - 1; i++) {
                memcpy(&menu_state.color_schemes[i], &menu_state.color_schemes[i + 1], sizeof(ColorScheme));
            }
            menu_state.num_color_schemes--;
            if (menu_state.current_color_scheme >= menu_state.num_color_schemes) {
                menu_state.current_color_scheme = menu_state.num_color_schemes - 1;
            }
        }
    }

    ColorScheme* scheme = &menu_state.color_schemes[menu_state.current_color_scheme];

    if (ImGui::CollapsingHeader("Colors")) {
        ImGui::ColorEdit4("Background", scheme->background_color);
        ImGui::ColorEdit4("Grid", scheme->grid_color);
        ImGui::ColorEdit4("Axes", scheme->axis_color);
        ImGui::ColorEdit4("Points", scheme->point_color);
        ImGui::ColorEdit4("Lines", scheme->line_color);
        ImGui::ColorEdit4("Text", scheme->text_color);
        ImGui::ColorEdit4("Highlight", scheme->highlight_color);
    }

    if (ImGui::CollapsingHeader("Scheme Name")) {
        ImGui::InputText("Name", scheme->name, sizeof(scheme->name));
    }

    ImGui::Separator();

    if (ImGui::Button("Apply Scheme")) {
        // TODO: Apply the color scheme to the renderer
        printf("Applied color scheme: %s\n", scheme->name);
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset Scheme")) {
        // Reset to default values based on scheme name
        if (strcmp(scheme->name, "Default") == 0) {
            scheme->background_color[0] = 0.1f; scheme->background_color[1] = 0.1f; scheme->background_color[2] = 0.1f; scheme->background_color[3] = 1.0f;
            scheme->grid_color[0] = 0.3f; scheme->grid_color[1] = 0.3f; scheme->grid_color[2] = 0.3f; scheme->grid_color[3] = 1.0f;
            scheme->axis_color[0] = 0.7f; scheme->axis_color[1] = 0.7f; scheme->axis_color[2] = 0.7f; scheme->axis_color[3] = 1.0f;
            scheme->point_color[0] = 0.2f; scheme->point_color[1] = 0.6f; scheme->point_color[2] = 1.0f; scheme->point_color[3] = 1.0f;
            scheme->line_color[0] = 1.0f; scheme->line_color[1] = 1.0f; scheme->line_color[2] = 1.0f; scheme->line_color[3] = 1.0f;
            scheme->text_color[0] = 1.0f; scheme->text_color[1] = 1.0f; scheme->text_color[2] = 1.0f; scheme->text_color[3] = 1.0f;
            scheme->highlight_color[0] = 1.0f; scheme->highlight_color[1] = 0.5f; scheme->highlight_color[2] = 0.0f; scheme->highlight_color[3] = 1.0f;
        }
    }

    ImGui::End();
}

// Animation control panel implementation
void imgui_show_animation_control_panel(bool* show)
{
    if (!*show) return;

    ImGui::Begin("Animation Controls", show, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_None);

    AnimationControls* anim = &menu_state.animation_controls;

    if (ImGui::CollapsingHeader("Animation Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable Animation", &anim->enabled);
        ImGui::SliderFloat("Duration", &anim->duration, 0.5f, 10.0f, "%.1f s");
        ImGui::SliderFloat("Speed", &anim->speed, 0.1f, 5.0f, "%.1f x");
        ImGui::Checkbox("Loop", &anim->loop);

        const char* easing_options[] = { "Linear", "Ease In", "Ease Out", "Ease In-Out" };
        ImGui::Combo("Easing", &anim->easing_type, easing_options, IM_ARRAYSIZE(easing_options));
    }

    if (ImGui::CollapsingHeader("Camera Animation")) {
        ImGui::Checkbox("Auto Rotate", &anim->auto_rotate);
        if (anim->auto_rotate) {
            ImGui::SliderFloat("Rotation Speed", &anim->auto_rotate_speed, 0.1f, 2.0f, "%.1f");
        }
    }

    ImGui::Separator();

    static bool is_animating = false;
    if (anim->enabled) {
        if (!is_animating) {
            if (ImGui::Button("Start Animation")) {
                is_animating = true;
                // TODO: Start animation
                printf("Animation started\n");
            }
        } else {
            if (ImGui::Button("Stop Animation")) {
                is_animating = false;
                // TODO: Stop animation
                printf("Animation stopped\n");
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset Animation")) {
        is_animating = false;
        anim->enabled = false;
        anim->duration = 2.0f;
        anim->speed = 1.0f;
        anim->loop = false;
        anim->easing_type = 0;
        anim->auto_rotate = false;
        anim->auto_rotate_speed = 0.5f;
        // TODO: Reset animation state
        printf("Animation reset\n");
    }

    ImGui::End();
}

// Visualization mode panel implementation
void imgui_show_visualization_mode_panel(bool* show)
{
    if (!*show) return;

    ImGui::Begin("Visualization Mode", show, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_None);

    const char* modes[] = {
        "Scatter Plot",
        "Bubble Chart",
        "Bar Chart",
        "Tree Map",
        "Network Graph",
        "Heat Map",
        "Surface Plot",
        "3D Histogram"
    };

    static int current_mode = 0;
    static int previous_mode = 0;

    ImGui::Text("Current Mode: %s", modes[menu_state.visualization_mode]);

    if (ImGui::CollapsingHeader("Available Modes", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (int i = 0; i < IM_ARRAYSIZE(modes); i++) {
            bool is_selected = (menu_state.visualization_mode == i);
            if (ImGui::Selectable(modes[i], is_selected)) {
                previous_mode = menu_state.visualization_mode;
                menu_state.visualization_mode = i;
                // TODO: Switch visualization mode
                printf("Switched to %s mode\n", modes[i]);
            }
        }
    }

    if (ImGui::CollapsingHeader("Mode Settings")) {
        switch (menu_state.visualization_mode) {
            case 0: // Scatter Plot
                ImGui::Text("Scatter Plot Settings:");
                ImGui::BulletText("Shows data points in 3D space");
                ImGui::BulletText("X, Y, Z axes represent different metrics");
                break;
            case 1: // Bubble Chart
                ImGui::Text("Bubble Chart Settings:");
                ImGui::BulletText("Bubble size represents additional metric");
                ImGui::BulletText("Color can represent another dimension");
                break;
            case 2: // Bar Chart
                ImGui::Text("Bar Chart Settings:");
                ImGui::BulletText("3D bars for comparative analysis");
                ImGui::BulletText("Height represents metric values");
                break;
            case 3: // Tree Map
                ImGui::Text("Tree Map Settings:");
                ImGui::BulletText("Hierarchical data representation");
                ImGui::BulletText("Size represents importance");
                break;
            case 4: // Network Graph
                ImGui::Text("Network Graph Settings:");
                ImGui::BulletText("Shows relationships between entities");
                ImGui::BulletText("Node size represents metrics");
                break;
            case 5: // Heat Map
                ImGui::Text("Heat Map Settings:");
                ImGui::BulletText("Color-coded surface representation");
                ImGui::BulletText("Shows metric density");
                break;
            case 6: // Surface Plot
                ImGui::Text("Surface Plot Settings:");
                ImGui::BulletText("3D surface from metric data");
                ImGui::BulletText("Shows continuous relationships");
                break;
            case 7: // 3D Histogram
                ImGui::Text("3D Histogram Settings:");
                ImGui::BulletText("Frequency distribution in 3D");
                ImGui::BulletText("Shows data distribution patterns");
                break;
        }
    }

    ImGui::Separator();

    if (ImGui::Button("Apply Mode")) {
        // TODO: Apply the selected visualization mode
        printf("Applied visualization mode: %s\n", modes[menu_state.visualization_mode]);
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset to Default")) {
        menu_state.visualization_mode = 0;
        printf("Reset to default visualization mode\n");
    }

    ImGui::End();
}

void imgui_show_analysis_results(bool* show)
{
    if (!*show) return;

    ImGui::Begin("Analysis Results", show, ImGuiWindowFlags_None);

    MetricConfig* config = &menu_state.metric_config;

    // Sample data for demonstration (in real implementation, this would come from actual analysis)
    static MetricResults sample_results = {0};
    static bool results_calculated = false;

    if (!results_calculated) {
        // Generate sample results for demonstration
        HalsteadMetrics sample_halstead = {
            .n1 = 15, .n2 = 25, .N1 = 150, .N2 = 200,
            .volume = 1200.0, .difficulty = 18.0, .effort = 21600.0,
            .time = 1200.0, .bugs = 0.8
        };

        apply_metric_configuration(config,
                                 12,  // complexity
                                 500, // physical LOC
                                 400, // logical LOC
                                 50,  // comment LOC
                                 &sample_halstead,
                                 65.0, // maintainability
                                 10.0, // comment density
                                 0.7,  // cohesion
                                 0.4,  // coupling
                                 15.0, // dead code %
                                 25.0, // duplication %
                                 &sample_results);
        results_calculated = true;
    }

    if (ImGui::CollapsingHeader("Summary", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Combined Score: %.1f/100", sample_results.combined_score);

        bool has_violations = check_threshold_violations(config, &sample_results);
        if (has_violations) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Issues Detected");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "All Metrics Within Thresholds");
        }

        ImGui::ProgressBar(sample_results.combined_score / 100.0f, ImVec2(-1, 0), "");
    }

    if (ImGui::CollapsingHeader("Detailed Metrics")) {
        if (config->enable_cyclomatic_complexity) {
            ImGui::Text("Cyclomatic Complexity: %d", sample_results.cyclomatic_complexity);
            if (sample_results.complexity_violation) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(> %.1f)",
                                 config->cyclomatic_complexity_threshold);
            }
        }

        if (config->enable_lines_of_code) {
            ImGui::Text("Lines of Code: %d physical, %d logical, %d comments",
                       sample_results.physical_loc, sample_results.logical_loc, sample_results.comment_loc);
        }

        if (config->enable_halstead_metrics) {
            ImGui::Text("Halstead Volume: %.0f", sample_results.halstead.volume);
            if (sample_results.halstead_violation) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(> %.0f)",
                                 config->halstead_volume_threshold);
            }
            ImGui::Text("Halstead Difficulty: %.1f", sample_results.halstead.difficulty);
            ImGui::Text("Halstead Effort: %.0f", sample_results.halstead.effort);
        }

        if (config->enable_maintainability_index) {
            ImGui::Text("Maintainability Index: %.1f", sample_results.maintainability_index);
            if (sample_results.maintainability_violation) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(< %.1f)",
                                 config->maintainability_index_threshold);
            }
        }

        if (config->enable_comment_density) {
            ImGui::Text("Comment Density: %.1f%%", sample_results.comment_density);
            if (sample_results.comment_density_violation) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(< %.1f%%)",
                                 config->comment_density_threshold);
            }
        }

        if (config->enable_class_cohesion) {
            ImGui::Text("Class Cohesion: %.2f", sample_results.class_cohesion);
            if (sample_results.cohesion_violation) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(< %.2f)",
                                 config->class_cohesion_threshold);
            }
        }

        if (config->enable_class_coupling) {
            ImGui::Text("Class Coupling: %.2f", sample_results.class_coupling);
            if (sample_results.coupling_violation) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(> %.2f)",
                                 config->class_coupling_threshold);
            }
        }

        if (config->enable_dead_code_detection) {
            ImGui::Text("Dead Code: %.1f%%", sample_results.dead_code_percentage);
            if (sample_results.dead_code_violation) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(> %.1f%%)",
                                 config->dead_code_percentage_threshold);
            }
        }

        if (config->enable_duplication_detection) {
            ImGui::Text("Code Duplication: %.1f%%", sample_results.duplication_percentage);
            if (sample_results.duplication_violation) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "(> %.1f%%)",
                                 config->duplication_percentage_threshold);
            }
        }
    }

    if (ImGui::CollapsingHeader("Recommendations")) {
        static char recommendations[1024] = "";
        if (strlen(recommendations) == 0) {
            get_recommendations(config, &sample_results, recommendations, sizeof(recommendations));
        }

        if (strlen(recommendations) > 0) {
            ImGui::TextWrapped("%s", recommendations);
        } else {
            ImGui::Text("No specific recommendations at this time.");
        }
    }

    ImGui::Separator();

    if (ImGui::Button("Recalculate")) {
        results_calculated = false;
        memset(recommendations, 0, sizeof(recommendations));
    }

    ImGui::SameLine();
    if (ImGui::Button("Export Results")) {
        // TODO: Export functionality
        printf("Export results triggered\n");
    }

    ImGui::End();
}

// Keyboard shortcuts handling
void imgui_handle_keyboard_shortcuts(void)
{
    ImGuiIO& io = ImGui::GetIO();

    // Theme switching shortcuts
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_T)) {
        // Ctrl+T: Toggle theme panel
        menu_state.show_theme_panel = !menu_state.show_theme_panel;
    }

    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_RightBracket)) {
        // Ctrl+]: Next theme
        imgui_next_theme();
    }

    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_LeftBracket)) {
        // Ctrl+[: Previous theme
        imgui_previous_theme();
    }

    // Number keys for direct theme selection (Ctrl+1-6 for first 6 themes)
    for (int i = 0; i < 6; i++) {
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_1 + i)) {
            if (i < imgui_get_theme_count()) {
                imgui_apply_theme(i);
            }
        }
    }
}

// Theme management functions
void imgui_init_theme_manager(void)
{
    if (!theme_manager_init(&global_theme_manager)) {
        fprintf(stderr, "Failed to initialize theme manager\n");
        return;
    }

    menu_state.theme_manager = &global_theme_manager;
    printf("Theme manager initialized with %d themes\n", theme_manager_get_theme_count(&global_theme_manager));
}

void imgui_shutdown_theme_manager(void)
{
    theme_manager_shutdown(&global_theme_manager);
    menu_state.theme_manager = NULL;
    printf("Theme manager shutdown complete\n");
}

void imgui_show_theme_panel(bool* show)
{
    if (!*show || !menu_state.theme_manager) return;

    ImGui::Begin("Theme Manager", show, ImGuiWindowFlags_AlwaysAutoResize);

    ThemeManager* tm = menu_state.theme_manager;
    int theme_count = theme_manager_get_theme_count(tm);

    if (ImGui::CollapsingHeader("Current Theme", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Active Theme: %s", theme_manager_get_current_theme_name(tm));

        if (ImGui::Button("Next Theme")) {
            imgui_next_theme();
        }
        ImGui::SameLine();
        if (ImGui::Button("Previous Theme")) {
            imgui_previous_theme();
        }
    }

    if (ImGui::CollapsingHeader("Available Themes")) {
        for (int i = 0; i < theme_count; i++) {
            const char* theme_name = theme_manager_get_theme_name(tm, i);
            bool is_current = (i == tm->current_theme);

            if (ImGui::Selectable(theme_name, is_current)) {
                imgui_apply_theme(i);
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Click to apply this theme");
            }
        }
    }

    if (ImGui::CollapsingHeader("Theme Management")) {
        static char new_theme_name[64] = "";

        ImGui::InputText("New Theme Name", new_theme_name, sizeof(new_theme_name));

        if (ImGui::Button("Create Custom Theme")) {
            if (strlen(new_theme_name) > 0) {
                if (imgui_create_custom_theme(new_theme_name)) {
                    memset(new_theme_name, 0, sizeof(new_theme_name));
                }
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Save Themes")) {
            theme_manager_save_themes(tm, tm->theme_file_path);
            printf("Themes saved to %s\n", tm->theme_file_path);
        }

        ImGui::SameLine();
        if (ImGui::Button("Load Themes")) {
            theme_manager_load_themes(tm, tm->theme_file_path);
            printf("Themes loaded from %s\n", tm->theme_file_path);
        }
    }

    ImGui::End();
}

bool imgui_apply_theme(int theme_index)
{
    if (!menu_state.theme_manager) return false;

    if (theme_manager_apply_theme(menu_state.theme_manager, theme_index)) {
        printf("Applied theme: %s\n", theme_manager_get_current_theme_name(menu_state.theme_manager));
        return true;
    }
    return false;
}

bool imgui_create_custom_theme(const char* name)
{
    if (!menu_state.theme_manager || !name) return false;

    // Create theme based on current ImGui style
    return theme_manager_create_theme(menu_state.theme_manager, name, &ImGui::GetStyle());
}

bool imgui_delete_theme(int theme_index)
{
    if (!menu_state.theme_manager) return false;
    return theme_manager_delete_theme(menu_state.theme_manager, theme_index);
}

const char* imgui_get_current_theme_name(void)
{
    if (!menu_state.theme_manager) return "Unknown";
    return theme_manager_get_current_theme_name(menu_state.theme_manager);
}

int imgui_get_theme_count(void)
{
    if (!menu_state.theme_manager) return 0;
    return theme_manager_get_theme_count(menu_state.theme_manager);
}

const char* imgui_get_theme_name(int index)
{
    if (!menu_state.theme_manager) return "Unknown";
    return theme_manager_get_theme_name(menu_state.theme_manager, index);
}

void imgui_next_theme(void)
{
    if (!menu_state.theme_manager) return;

    int next_index = menu_state.theme_manager->current_theme + 1;
    if (next_index >= menu_state.theme_manager->num_themes) {
        next_index = 0;
    }
    imgui_apply_theme(next_index);
}

void imgui_previous_theme(void)
{
    if (!menu_state.theme_manager) return;

    int prev_index = menu_state.theme_manager->current_theme - 1;
    if (prev_index < 0) {
        prev_index = menu_state.theme_manager->num_themes - 1;
    }
    imgui_apply_theme(prev_index);
}

void imgui_show_metric_config_panel(bool* show)
{
    if (!*show) return;

    ImGui::Begin("Metric Configuration", show, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_None);

// Dock layout management functions
void imgui_save_dock_layout(const char* layout_name)
{
    // Save current dock layout to ini file
    ImGui::SaveIniSettingsToDisk("cqanalyzer_dock_layout.ini");

    // Save panel states to a separate file
    FILE* file = fopen("cqanalyzer_panel_states.ini", "w");
    if (file)
    {
        fprintf(file, "[PanelStates]\n");
        fprintf(file, "show_main_control_panel=%d\n", menu_state.show_main_control_panel);
        fprintf(file, "show_camera_controls=%d\n", menu_state.show_camera_controls);
        fprintf(file, "show_display_options=%d\n", menu_state.show_display_options);
        fprintf(file, "show_color_scheme=%d\n", menu_state.show_color_scheme);
        fprintf(file, "show_animation_controls=%d\n", menu_state.show_animation_controls);
        fprintf(file, "show_analysis_results=%d\n", menu_state.show_analysis_results);
        fprintf(file, "show_metric_config_panel=%d\n", menu_state.show_metric_config_panel);
        fprintf(file, "show_visualization_settings=%d\n", menu_state.show_visualization_settings);
        fprintf(file, "show_theme_panel=%d\n", menu_state.show_theme_panel);
        fprintf(file, "show_settings_dialog=%d\n", menu_state.show_settings_dialog);
        fclose(file);
    }

    LOG_INFO("Dock layout and panel states saved");
}

void imgui_load_dock_layout(const char* layout_name)
{
    // Load dock layout from ini file
    ImGui::LoadIniSettingsFromDisk("cqanalyzer_dock_layout.ini");

    // Load panel states from separate file
    FILE* file = fopen("cqanalyzer_panel_states.ini", "r");
    if (file)
    {
        char line[256];
        while (fgets(line, sizeof(line), file))
        {
            // Remove newline
            line[strcspn(line, "\n")] = 0;

            if (strlen(line) == 0 || line[0] == '#') continue;

            char* equals = strchr(line, '=');
            if (!equals) continue;

            *equals = '\0';
            char* key = line;
            char* value = equals + 1;

            if (strcmp(key, "show_main_control_panel") == 0) menu_state.show_main_control_panel = atoi(value);
            else if (strcmp(key, "show_camera_controls") == 0) menu_state.show_camera_controls = atoi(value);
            else if (strcmp(key, "show_display_options") == 0) menu_state.show_display_options = atoi(value);
            else if (strcmp(key, "show_color_scheme") == 0) menu_state.show_color_scheme = atoi(value);
            else if (strcmp(key, "show_animation_controls") == 0) menu_state.show_animation_controls = atoi(value);
            else if (strcmp(key, "show_analysis_results") == 0) menu_state.show_analysis_results = atoi(value);
            else if (strcmp(key, "show_metric_config_panel") == 0) menu_state.show_metric_config_panel = atoi(value);
            else if (strcmp(key, "show_visualization_settings") == 0) menu_state.show_visualization_settings = atoi(value);
            else if (strcmp(key, "show_theme_panel") == 0) menu_state.show_theme_panel = atoi(value);
            else if (strcmp(key, "show_settings_dialog") == 0) menu_state.show_settings_dialog = atoi(value);
        }
        fclose(file);
    }

    LOG_INFO("Dock layout and panel states loaded");
}

void imgui_reset_dock_layout(void)
{
    // Reset to default dock layout
    ImGui::LoadIniSettingsFromMemory(""); // Load default layout

    // Reset panel states to defaults
    menu_state.show_main_control_panel = true;
    menu_state.show_camera_controls = false;
    menu_state.show_display_options = false;
    menu_state.show_color_scheme = false;
    menu_state.show_animation_controls = false;
    menu_state.show_analysis_results = false;
    menu_state.show_metric_config_panel = false;
    menu_state.show_visualization_settings = false;
    menu_state.show_theme_panel = false;
    menu_state.show_settings_dialog = false;

    LOG_INFO("Dock layout and panel states reset to default");
}

void imgui_apply_dock_preset(int preset_index)
{
    // Reset to default first
    ImGui::LoadIniSettingsFromMemory("");

    switch (preset_index)
    {
        case 0: // Default Layout
            imgui_reset_dock_layout();
            break;
        case 1: // Analysis Layout
            // Configure layout optimized for analysis
            menu_state.show_main_control_panel = true;
            menu_state.show_analysis_results = true;
            menu_state.show_metric_config_panel = true;
            menu_state.show_camera_controls = false;
            menu_state.show_display_options = false;
            menu_state.show_color_scheme = false;
            menu_state.show_animation_controls = false;
            menu_state.show_visualization_settings = false;
            menu_state.show_theme_panel = false;
            menu_state.show_settings_dialog = false;
            break;
        case 2: // Visualization Layout
            // Configure layout optimized for visualization
            menu_state.show_main_control_panel = true;
            menu_state.show_camera_controls = true;
            menu_state.show_display_options = true;
            menu_state.show_color_scheme = true;
            menu_state.show_analysis_results = false;
            menu_state.show_metric_config_panel = false;
            menu_state.show_animation_controls = false;
            menu_state.show_visualization_settings = true;
            menu_state.show_theme_panel = false;
            menu_state.show_settings_dialog = false;
            break;
        case 3: // Development Layout
            // Configure layout for development/debugging
            menu_state.show_main_control_panel = true;
            menu_state.show_camera_controls = true;
            menu_state.show_display_options = true;
            menu_state.show_color_scheme = true;
            menu_state.show_analysis_results = true;
            menu_state.show_metric_config_panel = true;
            menu_state.show_animation_controls = true;
            menu_state.show_visualization_settings = true;
            menu_state.show_theme_panel = true;
            menu_state.show_settings_dialog = false;
            break;
    }
    LOG_INFO("Applied dock preset: %d", preset_index);
}

    MetricConfig* config = &menu_state.metric_config;

    // Preset management
    if (ImGui::CollapsingHeader("Presets", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Current Preset: %s", config->current_preset_name);

        if (ImGui::Button("Load Preset")) {
            ImGui::OpenPopup("Load Preset");
        }

        ImGui::SameLine();
        if (ImGui::Button("Save Preset")) {
            ImGui::OpenPopup("Save Preset");
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset to Defaults")) {
            // Reset to default values
            menu_state_init();
        }

        // Load Preset popup
        if (ImGui::BeginPopupModal("Load Preset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            static int selected_preset = 0;
            static char available_presets[10][64];
            static int preset_count = 0;

            // Load available presets
            if (preset_count == 0) {
                metric_config_list_presets(available_presets, &preset_count, 10);
            }

            // Add built-in presets
            const char* builtin_presets[] = { "Code Quality Focus", "Performance Focus", "Maintainability Focus" };
            const int builtin_count = 3;

            ImGui::Text("Select a preset to load:");
            if (ImGui::BeginCombo("Preset", selected_preset < preset_count ?
                                  available_presets[selected_preset] :
                                  builtin_presets[selected_preset - preset_count])) {

                // User presets
                for (int i = 0; i < preset_count; i++) {
                    bool is_selected = (selected_preset == i);
                    if (ImGui::Selectable(available_presets[i], is_selected)) {
                        selected_preset = i;
                    }
                }

                // Built-in presets
                for (int i = 0; i < builtin_count; i++) {
                    bool is_selected = (selected_preset == i + preset_count);
                    if (ImGui::Selectable(builtin_presets[i], is_selected)) {
                        selected_preset = i + preset_count;
                    }
                }

                ImGui::EndCombo();
            }

            if (ImGui::Button("Load", ImVec2(80, 0))) {
                if (selected_preset < preset_count) {
                    // Load user preset
                    if (metric_config_load_preset(available_presets[selected_preset], config)) {
                        strcpy(config->current_preset_name, available_presets[selected_preset]);
                    }
                } else {
                    // Load built-in preset
                    int builtin_index = selected_preset - preset_count;
                    switch (builtin_index) {
                        case 0: metric_config_load_code_quality_preset(config); break;
                        case 1: metric_config_load_performance_preset(config); break;
                        case 2: metric_config_load_maintainability_preset(config); break;
                    }
                }
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(80, 0))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        // Save Preset popup
        if (ImGui::BeginPopupModal("Save Preset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            static char preset_name[64] = "";

            ImGui::Text("Enter preset name:");
            ImGui::InputText("Name", preset_name, sizeof(preset_name));

            if (ImGui::Button("Save", ImVec2(80, 0))) {
                if (strlen(preset_name) > 0) {
                    if (metric_config_save_preset(preset_name, config)) {
                        strcpy(config->current_preset_name, preset_name);
                        memset(preset_name, 0, sizeof(preset_name)); // Clear for next use
                    }
                }
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(80, 0))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    // Metric Enable/Disable
    if (ImGui::CollapsingHeader("Enabled Metrics", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Cyclomatic Complexity", &config->enable_cyclomatic_complexity);
        ImGui::Checkbox("Lines of Code", &config->enable_lines_of_code);
        ImGui::Checkbox("Halstead Metrics", &config->enable_halstead_metrics);
        ImGui::Checkbox("Maintainability Index", &config->enable_maintainability_index);
        ImGui::Checkbox("Comment Density", &config->enable_comment_density);
        ImGui::Checkbox("Class Cohesion", &config->enable_class_cohesion);
        ImGui::Checkbox("Class Coupling", &config->enable_class_coupling);
        ImGui::Checkbox("Dead Code Detection", &config->enable_dead_code_detection);
        ImGui::Checkbox("Duplication Detection", &config->enable_duplication_detection);
    }

    // Thresholds
    if (ImGui::CollapsingHeader("Metric Thresholds")) {
        if (config->enable_cyclomatic_complexity) {
            ImGui::SliderFloat("Cyclomatic Complexity", &config->cyclomatic_complexity_threshold, 1.0f, 50.0f, "%.1f");
        }

        if (config->enable_halstead_metrics) {
            ImGui::SliderFloat("Halstead Volume", &config->halstead_volume_threshold, 100.0f, 10000.0f, "%.0f");
            ImGui::SliderFloat("Halstead Difficulty", &config->halstead_difficulty_threshold, 1.0f, 100.0f, "%.1f");
            ImGui::SliderFloat("Halstead Effort", &config->halstead_effort_threshold, 1000.0f, 100000.0f, "%.0f");
        }

        if (config->enable_maintainability_index) {
            ImGui::SliderFloat("Maintainability Index", &config->maintainability_index_threshold, 0.0f, 100.0f, "%.1f");
        }

        if (config->enable_comment_density) {
            ImGui::SliderFloat("Comment Density (%)", &config->comment_density_threshold, 0.0f, 50.0f, "%.1f");
        }

        if (config->enable_class_cohesion) {
            ImGui::SliderFloat("Class Cohesion", &config->class_cohesion_threshold, 0.0f, 1.0f, "%.2f");
        }

        if (config->enable_class_coupling) {
            ImGui::SliderFloat("Class Coupling", &config->class_coupling_threshold, 0.0f, 1.0f, "%.2f");
        }

        if (config->enable_dead_code_detection) {
            ImGui::SliderFloat("Dead Code (%)", &config->dead_code_percentage_threshold, 0.0f, 100.0f, "%.1f");
        }

        if (config->enable_duplication_detection) {
            ImGui::SliderFloat("Duplication (%)", &config->duplication_percentage_threshold, 0.0f, 100.0f, "%.1f");
        }
    }

    // Weights
    if (ImGui::CollapsingHeader("Metric Weights")) {
        ImGui::Text("Adjust weights for combined scoring:");

        if (config->enable_cyclomatic_complexity) {
            ImGui::SliderFloat("Complexity Weight", &config->cyclomatic_complexity_weight, 0.0f, 1.0f, "%.2f");
        }

        if (config->enable_halstead_metrics) {
            ImGui::SliderFloat("Halstead Weight", &config->halstead_metrics_weight, 0.0f, 1.0f, "%.2f");
        }

        if (config->enable_maintainability_index) {
            ImGui::SliderFloat("Maintainability Weight", &config->maintainability_index_weight, 0.0f, 1.0f, "%.2f");
        }

        if (config->enable_comment_density) {
            ImGui::SliderFloat("Comment Density Weight", &config->comment_density_weight, 0.0f, 1.0f, "%.2f");
        }

        if (config->enable_class_cohesion) {
            ImGui::SliderFloat("Cohesion Weight", &config->class_cohesion_weight, 0.0f, 1.0f, "%.2f");
        }

        if (config->enable_class_coupling) {
            ImGui::SliderFloat("Coupling Weight", &config->class_coupling_weight, 0.0f, 1.0f, "%.2f");
        }

        if (config->enable_dead_code_detection) {
            ImGui::SliderFloat("Dead Code Weight", &config->dead_code_weight, 0.0f, 1.0f, "%.2f");
        }

        if (config->enable_duplication_detection) {
            ImGui::SliderFloat("Duplication Weight", &config->duplication_weight, 0.0f, 1.0f, "%.2f");
        }

        // Normalize weights button
        if (ImGui::Button("Normalize Weights")) {
            float total_weight = 0.0f;
            int enabled_count = 0;

            if (config->enable_cyclomatic_complexity) { total_weight += config->cyclomatic_complexity_weight; enabled_count++; }
            if (config->enable_halstead_metrics) { total_weight += config->halstead_metrics_weight; enabled_count++; }
            if (config->enable_maintainability_index) { total_weight += config->maintainability_index_weight; enabled_count++; }
            if (config->enable_comment_density) { total_weight += config->comment_density_weight; enabled_count++; }
            if (config->enable_class_cohesion) { total_weight += config->class_cohesion_weight; enabled_count++; }
            if (config->enable_class_coupling) { total_weight += config->class_coupling_weight; enabled_count++; }
            if (config->enable_dead_code_detection) { total_weight += config->dead_code_weight; enabled_count++; }
            if (config->enable_duplication_detection) { total_weight += config->duplication_weight; enabled_count++; }

            if (enabled_count > 0 && total_weight > 0.0f) {
                float normalized_weight = 1.0f / enabled_count;

                if (config->enable_cyclomatic_complexity) config->cyclomatic_complexity_weight = normalized_weight;
                if (config->enable_halstead_metrics) config->halstead_metrics_weight = normalized_weight;
                if (config->enable_maintainability_index) config->maintainability_index_weight = normalized_weight;
                if (config->enable_comment_density) config->comment_density_weight = normalized_weight;
                if (config->enable_class_cohesion) config->class_cohesion_weight = normalized_weight;
                if (config->enable_class_coupling) config->class_coupling_weight = normalized_weight;
                if (config->enable_dead_code_detection) config->dead_code_weight = normalized_weight;
                if (config->enable_duplication_detection) config->duplication_weight = normalized_weight;
            }
        }
    }

    // Normalization settings
    if (ImGui::CollapsingHeader("Normalization")) {
        const char* normalization_methods[] = { "Min-Max", "Z-Score", "Robust" };
        ImGui::Combo("Method", &config->normalization_method, normalization_methods, IM_ARRAYSIZE(normalization_methods));
        ImGui::Checkbox("Auto Normalize", &config->auto_normalize);

        ImGui::TextWrapped("Normalization adjusts metric values to a common scale for fair comparison and visualization.");
    }

    // Real-time preview
    if (ImGui::CollapsingHeader("Preview")) {
        ImGui::Text("Current Configuration Summary:");
        ImGui::Separator();

        int enabled_metrics = 0;
        if (config->enable_cyclomatic_complexity) enabled_metrics++;
        if (config->enable_lines_of_code) enabled_metrics++;
        if (config->enable_halstead_metrics) enabled_metrics++;
        if (config->enable_maintainability_index) enabled_metrics++;
        if (config->enable_comment_density) enabled_metrics++;
        if (config->enable_class_cohesion) enabled_metrics++;
        if (config->enable_class_coupling) enabled_metrics++;
        if (config->enable_dead_code_detection) enabled_metrics++;
        if (config->enable_duplication_detection) enabled_metrics++;

        ImGui::Text("Enabled Metrics: %d", enabled_metrics);

        // Show weight distribution
        float total_weight = 0.0f;
        if (config->enable_cyclomatic_complexity) total_weight += config->cyclomatic_complexity_weight;
        if (config->enable_halstead_metrics) total_weight += config->halstead_metrics_weight;
        if (config->enable_maintainability_index) total_weight += config->maintainability_index_weight;
        if (config->enable_comment_density) total_weight += config->comment_density_weight;
        if (config->enable_class_cohesion) total_weight += config->class_cohesion_weight;
        if (config->enable_class_coupling) total_weight += config->class_coupling_weight;
        if (config->enable_dead_code_detection) total_weight += config->dead_code_weight;
        if (config->enable_duplication_detection) total_weight += config->duplication_weight;

        ImGui::Text("Total Weight: %.2f", total_weight);

        if (total_weight > 0.0f) {
            ImGui::ProgressBar(total_weight / 1.0f, ImVec2(-1, 0), "Weight Balance");
        }

        // Show key thresholds
        ImGui::Text("Key Thresholds:");
        if (config->enable_cyclomatic_complexity) {
            ImGui::BulletText("Complexity: %.1f", config->cyclomatic_complexity_threshold);
        }
        if (config->enable_maintainability_index) {
            ImGui::BulletText("Maintainability: %.1f", config->maintainability_index_threshold);
        }
        if (config->enable_comment_density) {
            ImGui::BulletText("Comment Density: %.1f%%", config->comment_density_threshold);
        }
    }

    ImGui::End();
}