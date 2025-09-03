# Source directory CMakeLists.txt
# Handles the modular build system for CQAnalyzer

# Parser module
add_library(cqanalyzer_parser STATIC
    parser/file_scanner.c
    parser/ast_parser.c
    parser/language_support.c
    parser/generic_parser.c
    parser/preprocessor.c
)

target_include_directories(cqanalyzer_parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${LIBCLANG_INCLUDE_DIRS}
)

target_link_libraries(cqanalyzer_parser
    cqanalyzer_data
    ${LIBCLANG_LIBRARY}
)

# Analyzer module
add_library(cqanalyzer_analyzer STATIC
    analyzer/metric_calculator.c
    analyzer/complexity_analyzer.c
    analyzer/duplication_detector.c
    analyzer/dead_code_detector.c
)

target_include_directories(cqanalyzer_analyzer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(cqanalyzer_analyzer
    cqanalyzer_parser
)

# Data module
add_library(cqanalyzer_data STATIC
    data/ast_types.c
    data/data_store.c
    data/metric_aggregator.c
    data/serialization.c
)

target_include_directories(cqanalyzer_data PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(cqanalyzer_data
    cqanalyzer_analyzer
)

# Visualizer module (temporarily disabled due to build issues)
# add_library(cqanalyzer_visualizer STATIC
#     visualizer/renderer.c
#     visualizer/camera.c
#     visualizer/scene.c
#     visualizer/shader.c
#     visualizer/text_renderer.c
#     visualizer/transforms.c
#     visualizer/network_graph.c
#     visualizer/scatter_plot.c
#     visualizer/bubble_chart.c
#     visualizer/tree_visualization.c
#     visualizer/color.c
#     visualizer/gradient.c
#     visualizer/lighting.c
#     visualizer/bar_chart.c
#     visualizer/heatmap.c
#     visualizer/complexity_landscape.c
# )

# target_include_directories(cqanalyzer_visualizer PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/../include
# )

# target_link_libraries(cqanalyzer_visualizer
#     cqanalyzer_data
#     ${OPENGL_LIBRARIES}
#     ${GLFW3_LIBRARIES}
#     ${GLEW_LIBRARIES}
#     ${GLM_LIBRARIES}
#     ${FREETYPE_LIBRARIES}
# )

# UI module
add_library(cqanalyzer_ui STATIC
    ui/cli_interface.c
    ui/input_handler.c
    ui/progress_display.c
)

target_include_directories(cqanalyzer_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(cqanalyzer_ui
    cqanalyzer_data
)

# Utils module
add_library(cqanalyzer_utils STATIC
    utils/logger.c
    utils/config.c
    utils/memory.c
    utils/string_utils.c
)

target_include_directories(cqanalyzer_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Set common compile options for all modules
set(MODULE_LIBRARIES
    cqanalyzer_parser
    cqanalyzer_analyzer
    cqanalyzer_data
    cqanalyzer_ui
    cqanalyzer_utils
)

foreach(lib ${MODULE_LIBRARIES})
    if(MSVC)
        target_compile_options(${lib} PRIVATE /W4)
    else()
        target_compile_options(${lib} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
