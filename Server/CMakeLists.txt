# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Create library for this package

# Minimum requried version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(tcpserver VERSION 1.0 DESCRIPTION "TCP server")
project(tlsserver VERSION 1.0 DESCRIPTION "TLS server")

# Include standard directories
include(GNUInstallDirs)

# The deault build type is Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Source files to be compiled
add_library(tcpserver SHARED "src/TcpServer.cpp")
add_library(tlsserver SHARED "src/TlsServer.cpp")

# Set include path to search for headers
include_directories(include)

# Set compiler flags
target_compile_options(tcpserver PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(tcpserver PUBLIC $<$<CONFIG:RELEASE>: -s>)
target_compile_options(tlsserver PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(tlsserver PUBLIC $<$<CONFIG:RELEASE>: -s>)

# Use C++17 standard
set_target_properties(tcpserver PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "include/TcpServer.h;include/Server.h;include/ServerDefines.h;include/Server.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
set_target_properties(tlsserver PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "include/TlsServer.h;include/Server.h;include/ServerDefines.h;include/Server.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Install rule (copy to install directory)
install(TARGETS tcpserver tlsserver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tcp )

# Print build type
message("Server build type: ${CMAKE_BUILD_TYPE}")
