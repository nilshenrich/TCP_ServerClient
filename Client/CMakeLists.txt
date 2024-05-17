# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Create library for this package

# Minimum requried version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(tcpclient VERSION 1.0 DESCRIPTION "TCP client")
project(tlsclient VERSION 1.0 DESCRIPTION "TLS client")

# Include standard directories
include(GNUInstallDirs)

# The deault build type is Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Source files to be compiled
add_library(tcpclient SHARED "src/TcpClient.cpp")
add_library(tlsclient SHARED "src/TlsClient.cpp")

# Set include path to search for headers
include_directories(include)

# Set compiler flags
target_compile_options(tcpclient PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(tcpclient PUBLIC $<$<CONFIG:RELEASE>: -s>)
target_compile_options(tlsclient PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(tlsclient PUBLIC $<$<CONFIG:RELEASE>: -s>)

# Use C++17 standard
set_target_properties(tcpclient PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "include/TcpClient.h;include/Client.h;include/ClientDefines.h;include/Client.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
set_target_properties(tlsclient PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "include/TlsClient.h;include/Client.h;include/ClientDefines.h;include/Client.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Install rule (copy to install directory)
install(TARGETS tcpclient tlsclient
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tcp )

# Print build type
message("Client build type: ${CMAKE_BUILD_TYPE}")
