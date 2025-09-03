cmake_minimum_required(VERSION 3.16)
project(CQAnalyzer VERSION 1.0.0 LANGUAGES C CXX)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Set C++ standard for any C++ components
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find OpenGL (optional for graceful degradation)
find_package(OpenGL)
if(OpenGL_FOUND)
    set(HAVE_OPENGL ON)
    include_directories(${OPENGL_INCLUDE_DIR})
else()
    set(HAVE_OPENGL OFF)
    message(WARNING "OpenGL not found - GUI features will be disabled")
endif()

# Find GLFW3 (optional)
pkg_check_modules(GLFW3 glfw3)
if(GLFW3_FOUND)
    set(HAVE_GLFW3 ON)
    include_directories(${GLFW3_INCLUDE_DIRS})
    link_directories(${GLFW3_LIBRARY_DIRS})
else()
    set(HAVE_GLFW3 OFF)
    message(WARNING "GLFW3 not found - GUI features will be disabled")
endif()

# Find GLEW (optional)
pkg_check_modules(GLEW glew)
if(GLEW_FOUND)
    set(HAVE_GLEW ON)
    include_directories(${GLEW_INCLUDE_DIRS})
    link_directories(${GLEW_LIBRARY_DIRS})
else()
    set(HAVE_GLEW OFF)
    message(WARNING "GLEW not found - GUI features will be disabled")
endif()

# Find GLM (optional)
pkg_check_modules(GLM glm)
if(GLM_FOUND)
    set(HAVE_GLM ON)
    include_directories(${GLM_INCLUDE_DIRS})
else()
    set(HAVE_GLM OFF)
    message(WARNING "GLM not found - 3D visualization features will be limited")
endif()

# Find FreeType (optional)
pkg_check_modules(FREETYPE freetype2)
if(FREETYPE_FOUND)
    set(HAVE_FREETYPE ON)
    include_directories(${FREETYPE_INCLUDE_DIRS})
    link_directories(${FREETYPE_LIBRARY_DIRS})
else()
    set(HAVE_FREETYPE OFF)
    message(WARNING "FreeType not found - text rendering features will be disabled")
endif()

# Add ImGui (only if graphics libraries are available)
if(HAVE_OPENGL AND HAVE_GLFW3 AND HAVE_GLEW)
    set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
    include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)
    set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
    set(HAVE_IMGUI ON)
else()
    set(HAVE_IMGUI OFF)
    message(WARNING "ImGui not available - GUI features will be disabled")
endif()

# Find libclang (required for basic functionality)
find_library(LIBCLANG_LIBRARY NAMES clang libclang clang-18 libclang-18)
pkg_check_modules(LIBCLANG libclang)
if(LIBCLANG_FOUND OR LIBCLANG_LIBRARY)
    set(HAVE_LIBCLANG ON)
    if(LIBCLANG_FOUND)
        include_directories(${LIBCLANG_INCLUDE_DIRS})
        link_directories(${LIBCLANG_LIBRARY_DIRS})
    else()
        # Fallback: try to find libclang manually
        find_path(LIBCLANG_INCLUDE_DIR clang-c/Index.h
            PATHS /usr/include /usr/local/include /usr/lib/llvm-18/include
        )
        if(LIBCLANG_INCLUDE_DIR)
            include_directories(${LIBCLANG_INCLUDE_DIR})
        endif()
    endif()
else()
    set(HAVE_LIBCLANG OFF)
    message(WARNING "libclang not found - code parsing features will be disabled")
endif()

# Optional packages
pkg_check_modules(CJSON cjson)
if(CJSON_FOUND)
    set(HAVE_CJSON ON)
else()
    set(HAVE_CJSON OFF)
endif()

pkg_check_modules(PCRE2 libpcre2-8)
if(PCRE2_FOUND)
    set(HAVE_PCRE2 ON)
else()
    set(HAVE_PCRE2 OFF)
endif()

pkg_check_modules(SQLITE3 sqlite3)
if(SQLITE3_FOUND)
    set(HAVE_SQLITE3 ON)
else()
    set(HAVE_SQLITE3 OFF)
endif()

# Threading support
find_package(Threads REQUIRED)

# Add subdirectories
add_subdirectory(src)

# Enable testing
enable_testing()
add_subdirectory(tests)

# Add test parser executable
add_executable(test_parser test_parser.c src/cqanalyzer.c)
target_link_libraries(test_parser
    cqanalyzer_parser
    cqanalyzer_utils
)


# Add main executable
add_executable(cqanalyzer
    src/main.c
    src/main_gui.c
    src/cqanalyzer.c
)

# Set up libraries list
set(CQANALYZER_LIBRARIES
    cqanalyzer_parser
    cqanalyzer_analyzer
    cqanalyzer_data
    cqanalyzer_ui
    cqanalyzer_utils
    Threads::Threads
    m  # Math library
)

# Add optional libraries
if(HAVE_LIBCLANG)
    list(APPEND CQANALYZER_LIBRARIES ${LIBCLANG_LIBRARY})
endif()

if(HAVE_GLFW3)
    list(APPEND CQANALYZER_LIBRARIES ${GLFW3_LIBRARIES})
endif()

if(HAVE_GLEW)
    list(APPEND CQANALYZER_LIBRARIES ${GLEW_LIBRARIES})
endif()

if(HAVE_OPENGL)
    list(APPEND CQANALYZER_LIBRARIES ${OPENGL_LIBRARIES})
endif()

if(HAVE_FREETYPE)
    list(APPEND CQANALYZER_LIBRARIES ${FREETYPE_LIBRARIES})
endif()

if(HAVE_IMGUI)
    target_sources(cqanalyzer PRIVATE ${IMGUI_SOURCES})
endif()

target_link_libraries(cqanalyzer ${CQANALYZER_LIBRARIES})

# Add compile definitions for optional features
if(HAVE_OPENGL)
    target_compile_definitions(cqanalyzer PRIVATE HAVE_OPENGL)
endif()

if(HAVE_GLFW3)
    target_compile_definitions(cqanalyzer PRIVATE HAVE_GLFW3)
endif()

if(HAVE_GLEW)
    target_compile_definitions(cqanalyzer PRIVATE HAVE_GLEW)
endif()

if(HAVE_GLM)
    target_compile_definitions(cqanalyzer PRIVATE HAVE_GLM)
endif()

if(HAVE_FREETYPE)
    target_compile_definitions(cqanalyzer PRIVATE HAVE_FREETYPE)
endif()

if(HAVE_LIBCLANG)
    target_compile_definitions(cqanalyzer PRIVATE HAVE_LIBCLANG)
endif()

if(HAVE_CJSON)
    target_compile_definitions(cqanalyzer PRIVATE CJSON_FOUND)
endif()

if(HAVE_PCRE2)
    target_compile_definitions(cqanalyzer PRIVATE PCRE2_FOUND)
endif()

if(HAVE_SQLITE3)
    target_compile_definitions(cqanalyzer PRIVATE SQLITE3_FOUND)
endif()

# Install targets
install(TARGETS cqanalyzer
    RUNTIME DESTINATION bin
)

# Generate documentation target (if Doxygen is available)
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    add_custom_target(doc_doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Print configuration summary
message(STATUS "=== CQAnalyzer Build Configuration ===")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "OpenGL: ${OPENGL_LIBRARIES}")
message(STATUS "GLFW3: ${GLFW3_LIBRARIES}")
message(STATUS "GLEW: ${GLEW_LIBRARIES}")
message(STATUS "GLM: ${GLM_INCLUDE_DIRS}")
message(STATUS "FreeType: ${FREETYPE_LIBRARIES}")
message(STATUS "LibClang: ${LIBCLANG_LIBRARY}")
if(CJSON_FOUND)
    message(STATUS "cJSON: Found")
else()
    message(STATUS "cJSON: Not found (optional)")
endif()
if(PCRE2_FOUND)
    message(STATUS "PCRE2: Found")
else()
    message(STATUS "PCRE2: Not found (optional)")
endif()
if(SQLITE3_FOUND)
    message(STATUS "SQLite3: Found")
else()
    message(STATUS "SQLite3: Not found (optional)")
endif()
message(STATUS "=======================================")
