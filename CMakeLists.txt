cmake_minimum_required(VERSION 3.14)
project(DisjointIntervalSet VERSION 1.1.2 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Interface library for header-only installation
add_library(libdis INTERFACE)
target_include_directories(libdis INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(libdis INTERFACE cxx_std_17)

# Testing (optional)
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    enable_testing()

    # Find Google Test
    find_package(GTest REQUIRED)
endif()

# Coverage options
option(ENABLE_COVERAGE "Enable coverage reporting" ON)
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-O0 -g --coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

if(BUILD_TESTING)
# Test executables
add_executable(test_interval tests/test_interval.cpp)
target_link_libraries(test_interval GTest::gtest GTest::gtest_main pthread)
add_test(NAME test_interval COMMAND test_interval)

add_executable(test_disjoint_interval_set tests/test_disjoint_interval_set.cpp)
target_link_libraries(test_disjoint_interval_set GTest::gtest GTest::gtest_main pthread)
add_test(NAME test_disjoint_interval_set COMMAND test_disjoint_interval_set)

add_executable(test_algorithms tests/test_algorithms.cpp)
target_link_libraries(test_algorithms GTest::gtest GTest::gtest_main pthread)
add_test(NAME test_algorithms COMMAND test_algorithms)

add_executable(test_integration tests/test_integration.cpp)
target_link_libraries(test_integration GTest::gtest GTest::gtest_main pthread)
add_test(NAME test_integration COMMAND test_integration)

# Comprehensive test suites
add_executable(test_elegant_api tests/test_elegant_api.cpp)
target_link_libraries(test_elegant_api GTest::gtest GTest::gtest_main pthread)
add_test(NAME test_elegant_api COMMAND test_elegant_api)

add_executable(test_interval_comprehensive tests/test_interval_comprehensive.cpp)
target_link_libraries(test_interval_comprehensive GTest::gtest GTest::gtest_main pthread)
add_test(NAME test_interval_comprehensive COMMAND test_interval_comprehensive)

add_executable(test_dis_comprehensive tests/test_dis_comprehensive.cpp)
target_link_libraries(test_dis_comprehensive GTest::gtest GTest::gtest_main pthread)
add_test(NAME test_dis_comprehensive COMMAND test_dis_comprehensive)

add_executable(test_parser_formatter_comprehensive tests/test_parser_formatter_comprehensive.cpp)
target_link_libraries(test_parser_formatter_comprehensive GTest::gtest GTest::gtest_main pthread)
add_test(NAME test_parser_formatter_comprehensive COMMAND test_parser_formatter_comprehensive)

# Custom target to run all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_interval test_disjoint_interval_set test_algorithms test_integration
            test_elegant_api test_interval_comprehensive test_dis_comprehensive test_parser_formatter_comprehensive
)

# Custom target for coverage report
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
        COMMAND gcovr -r ${CMAKE_SOURCE_DIR} --html --html-details -o ${CMAKE_BINARY_DIR}/coverage/index.html
        COMMAND echo "Coverage report generated at: ${CMAKE_BINARY_DIR}/coverage/index.html"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS run_tests
    )
endif()

endif() # BUILD_TESTING

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install headers
install(DIRECTORY include/dis
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp")

install(DIRECTORY include/disjoint_interval_set
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp")

# Install library target
install(TARGETS libdis
        EXPORT libdis-targets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Create and install CMake config files
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/libdis-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libdis-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/libdis-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libdis
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/libdis-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/libdis-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libdis)

install(EXPORT libdis-targets
        FILE libdis-targets.cmake
        NAMESPACE libdis::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libdis)