# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Create library for this package

# Minimum requried version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(TcpServer VERSION 1.0 DESCRIPTION "TCP server")
project(TlsServer VERSION 1.0 DESCRIPTION "TLS server")

# Include standard directories
include(GNUInstallDirs)

# The deault build type is Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Source files to be compiled
add_library(TcpServer SHARED "src/TcpServer.cpp")
add_library(TlsServer SHARED "src/TlsServer.cpp")

# Set include path to search for headers
include_directories(include)

# Set compiler flags
target_compile_options(TcpServer PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(TcpServer PUBLIC $<$<CONFIG:RELEASE>: -s>)
target_compile_options(TlsServer PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(TlsServer PUBLIC $<$<CONFIG:RELEASE>: -s>)

# Use C++17 standard
set_target_properties(TcpServer PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/TcpServer.h;include/Server.h;include/Defines.h;include/Server.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
set_target_properties(TlsServer PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/TlsServer.h;include/Server.h;include/Defines.h;include/Server.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Install rule (copy to install directory)
install(TARGETS TcpServer TlsServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/TcpServer)

# Print build type
message("Build type: ${CMAKE_BUILD_TYPE}")
