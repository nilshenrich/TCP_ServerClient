# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Create library for this package

# Minimum requried version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(TcpClient VERSION 1.0 DESCRIPTION "TCP client")
project(TlsClient VERSION 1.0 DESCRIPTION "TLS client")

# Include standard directories
include(GNUInstallDirs)

# The deault build type is Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Source files to be compiled
add_library(TcpClient SHARED "src/TcpClient.cpp")
add_library(TlsClient SHARED "src/TlsClient.cpp")

# Set include path to search for headers
include_directories(include)

# Set compiler flags
target_compile_options(TcpClient PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(TcpClient PUBLIC $<$<CONFIG:RELEASE>: -s>)
target_compile_options(TlsClient PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(TlsClient PUBLIC $<$<CONFIG:RELEASE>: -s>)

# Use C++17 standard
set_target_properties(TcpClient PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/TcpClient.h;include/Client.h;include/Defines.h;include/Client.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
set_target_properties(TlsClient PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/TlsClient.h;include/Client.h;include/Defines.h;include/Client.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Install rule (copy to install directory)
install(TARGETS TcpClient TlsClient
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/TCP_ServerClient )

# Print build type
message("Build type: ${CMAKE_BUILD_TYPE}")
