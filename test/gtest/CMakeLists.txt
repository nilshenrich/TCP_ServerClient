# Build entire gtest project
# All source files from src folder should be compiled
# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Minimum cmake version required is 3.13
cmake_minimum_required(VERSION 3.13)

# Project name: test
project(test)

# Include directories
include_directories(include include/api ../../src)

# Add source files (All files in src folder)
file(GLOB sourcefiles "src/*.cpp" "src/*/*.cpp")
add_executable(test ${sourcefiles})

# Set compiler flags
target_compile_options(test PRIVATE -fexceptions -Wall -O0 $<$<CONFIG:DEBUG>: -g -DDEVELOP>)

# Add libraries
target_link_libraries(test -lgtest -lssl -lcrypto)

# Use C++17 standard
set_target_properties(test PROPERTIES
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Message build type
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
if ("${CMAKE_BUILD_TYPE_UPPER}" STREQUAL "DEBUG")
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE_UPPER}")
endif()
