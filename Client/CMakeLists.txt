# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Create library for this package

# Minimum requried version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(tcpClient VERSION 1.0 DESCRIPTION "TCP client")
project(tlsClient VERSION 1.0 DESCRIPTION "TLS client")

# Include standard directories
include(GNUInstallDirs)

# The deault build type is Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Source files to be compiled
add_library(tcpClient SHARED "src/TcpClient.cpp")
add_library(tlsClient SHARED "src/TlsClient.cpp")

# Set include path to search for headers
include_directories(include)

# Set compiler flags
target_compile_options(tcpClient PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(tcpClient PUBLIC $<$<CONFIG:RELEASE>: -s>)
target_compile_options(tlsClient PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(tlsClient PUBLIC $<$<CONFIG:RELEASE>: -s>)

# Use C++17 standard
set_target_properties(tcpClient PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 2
    PUBLIC_HEADER "include/TcpClient.h;include/Client.h;include/Defines.h;include/Client.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
set_target_properties(tlsClient PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 2
    PUBLIC_HEADER "include/TlsClient.h;include/Client.h;include/Defines.h;include/Client.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Install rule (copy to install directory)
install(TARGETS tcpClient tlsClient
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Client)

# Print build type
message("Build type: ${CMAKE_BUILD_TYPE}")
