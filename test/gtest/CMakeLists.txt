# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Build entire gtest project
# All source files from src folder should be compiled

# Minimum cmake version required is 3.10
cmake_minimum_required(VERSION 3.10)

# Project name: networktester
project(networktester)

# Include directories
include_directories(include include/api ../NetworkListener/include ../NetworkClient/include)

# Add source files (All files in src folder)
file(GLOB sourcefiles
    "src/*.cpp"
    "src/*/*.cpp"
    "../NetworkListener/src/*.cpp"
    "../NetworkClient/src/*.cpp")
add_executable(networktester ${sourcefiles})

# Set compiler flags
target_compile_options(networktester PRIVATE -fexceptions -Wall -g -Og
    $<$<CONFIG:DEBUG>: -DDEVELOP>)

# Add libraries
target_link_libraries(networktester
    -pthread
    -lgtest
    -lcrypto
    -lssl)

# Use C++17 standard
set_target_properties(networktester PROPERTIES
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
