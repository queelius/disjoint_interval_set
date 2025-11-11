cmake_minimum_required(VERSION 3.14)
project(DisjointIntervalSet VERSION 1.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest REQUIRED)

# Coverage options
option(ENABLE_COVERAGE "Enable coverage reporting" ON)
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-O0 -g --coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

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