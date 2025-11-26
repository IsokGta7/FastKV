cmake_minimum_required(VERSION 3.25)
project(fastkv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compile commands (clang-tidy)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ================================
# Library
# ================================
add_library(fastkv src/fastkv.cpp)
target_include_directories(fastkv PUBLIC include)

# ================================
# Main executable
# ================================
add_executable(fastkv_main src/main.cpp)
target_link_libraries(fastkv_main PRIVATE fastkv)

# ================================
# Tests (GTest)
# ================================
enable_testing()                       # <---- ¡FALTA CRÍTICA!

find_package(GTest CONFIG REQUIRED)

add_executable(fastkv_tests tests/test_fastkv.cpp)
target_link_libraries(fastkv_tests PRIVATE fastkv GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(fastkv_tests)     # <---- ¡Necesario para CTest!

# ================================
# Benchmarks (Google Benchmark)
# ================================
find_package(benchmark CONFIG REQUIRED)

add_executable(fastkv_bench benchmarks/bench_fastkv.cpp)
target_link_libraries(fastkv_bench PRIVATE fastkv benchmark::benchmark)
