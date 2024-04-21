# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Build entire gtest project
# All source files from src folder should be compiled

# Minimum cmake version required is 3.13
cmake_minimum_required(VERSION 3.13)

# Project name: test
project(test)

# Include directories
include_directories(include include/api ../../Server/include ../../Client/include)

# Add source files (All files in src folder)
file(GLOB sourcefiles
    "src/*.cpp"
    "src/*/*.cpp"
    "../../Server/src/*.cpp"
    "../../Client/src/*.cpp")
add_executable(test ${sourcefiles})

# Set compiler flags
target_compile_options(test PRIVATE -fexceptions -Wall -g -Og
    $<$<CONFIG:DEBUG>: -DDEVELOP>)

# Add libraries
target_link_libraries(test
    -pthread
    -lgtest
    -lcrypto
    -lssl)

# Use C++17 standard
set_target_properties(test PROPERTIES
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
