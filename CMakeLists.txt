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

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLFW3
pkg_check_modules(GLFW3 REQUIRED glfw3)
include_directories(${GLFW3_INCLUDE_DIRS})
link_directories(${GLFW3_LIBRARY_DIRS})

# Find GLEW
pkg_check_modules(GLEW REQUIRED glew)
include_directories(${GLEW_INCLUDE_DIRS})
link_directories(${GLEW_LIBRARY_DIRS})

# Find GLM
pkg_check_modules(GLM REQUIRED glm)
include_directories(${GLM_INCLUDE_DIRS})

# Find FreeType
pkg_check_modules(FREETYPE REQUIRED freetype2)
include_directories(${FREETYPE_INCLUDE_DIRS})
link_directories(${FREETYPE_LIBRARY_DIRS})

# Find libclang
find_library(LIBCLANG_LIBRARY NAMES clang libclang clang-18 libclang-18 REQUIRED)
pkg_check_modules(LIBCLANG libclang)
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

# Optional packages
pkg_check_modules(CJSON cjson)
pkg_check_modules(PCRE2 libpcre2-8)
pkg_check_modules(SQLITE3 sqlite3)

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
    src/cqanalyzer.c
)
target_link_libraries(cqanalyzer
    cqanalyzer_parser
    cqanalyzer_analyzer
    cqanalyzer_data
    cqanalyzer_visualizer
    cqanalyzer_ui
    cqanalyzer_utils
    ${OPENGL_LIBRARIES}
    ${GLFW3_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${LIBCLANG_LIBRARY}
    Threads::Threads
    m  # Math library
)

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
