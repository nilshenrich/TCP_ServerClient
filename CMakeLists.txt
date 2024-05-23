# For local installation please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Minimum required cmake version
cmake_minimum_required(VERSION 3.10)

# Projects to be installed
project(tcp VERSION 2.0)

# Include standard GNU install directories
include(GNUInstallDirs)

# The deault build type is Release
# For Release install in default location, for Debug install in build folder locally
# If build type is other that Release or Debug, throw an error
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
if(CMAKE_BUILD_TYPE MATCHES "^(Debug|Local)$")
  set(PATH_INSTALL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/build/include/tcp)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(PATH_INSTALL_INCLUDE ${CMAKE_INSTALL_INCLUDEDIR}/tcp)
else()
  message(FATAL_ERROR "CMAKE_BUILD_TYPE must be 'Release' or 'Debug'")
endif()

# Install library
# TODO: Remove .h when copying
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src DESTINATION ${PATH_INSTALL_INCLUDE} FILES_MATCHING PATTERN "*.h")
message(STATUS "Installation path: ${PATH_INSTALL_INCLUDE}")
