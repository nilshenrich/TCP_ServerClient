# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Create library for this package

# Minimum requried version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(networkListenerTcp VERSION 2.2 DESCRIPTION "Listener for TCP")
project(networkListenerTls VERSION 2.2 DESCRIPTION "Listener for TLS")

# Include standard directories
include(GNUInstallDirs)

# The deault build type is Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Source files to be compiled
add_library(networkListenerTcp SHARED "src/TcpServer.cpp")
add_library(networkListenerTls SHARED "src/TlsServer.cpp")

# Set include path to search for headers
include_directories(include)

# Set compiler flags
target_compile_options(networkListenerTcp PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(networkListenerTcp PUBLIC $<$<CONFIG:RELEASE>: -s>)
target_compile_options(networkListenerTls PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(networkListenerTls PUBLIC $<$<CONFIG:RELEASE>: -s>)

# Use C++17 standard
set_target_properties(networkListenerTcp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 2
    PUBLIC_HEADER "include/TcpServer.h;include/NetworkListener.h;include/NetworkingDefines.h;include/NetworkListener.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
set_target_properties(networkListenerTls PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 2
    PUBLIC_HEADER "include/TlsServer.h;include/NetworkListener.h;include/NetworkingDefines.h;include/NetworkListener.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Install rule (copy to install directory)
install(TARGETS networkListenerTcp networkListenerTls
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NetworkListener)

# Print build type
message("Build type: ${CMAKE_BUILD_TYPE}")
