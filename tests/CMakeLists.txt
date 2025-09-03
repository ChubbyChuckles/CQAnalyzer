# Tests directory CMakeLists.txt
# Handles the test build system

# Find CUnit testing framework
find_package(PkgConfig REQUIRED)
pkg_check_modules(CUNIT REQUIRED cunit)

# Include test utilities
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Test executable
add_executable(cqanalyzer_tests
    test_main.c
    test_utils.c
    test_parser.c
    test_analyzer.c
    test_data.c
    test_ui.c
)

# Link with main libraries and CUnit
target_link_libraries(cqanalyzer_tests
    cqanalyzer_parser
    cqanalyzer_analyzer
    cqanalyzer_data
    cqanalyzer_visualizer
    cqanalyzer_ui
    cqanalyzer_utils
    ${CUNIT_LIBRARIES}
    m  # Math library
)

# Add test
add_test(NAME cqanalyzer_unit_tests
         COMMAND cqanalyzer_tests)
