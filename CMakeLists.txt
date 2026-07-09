# FIXME: Translate to english

cmake_minimum_required(VERSION 3.13)
project(TCP_ServerClient VERSION 1.0.0 DESCRIPTION "Unified TCP Server/Client project with FTP and tests")

# C++17 Standard für das gesamte Projekt
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Standard-Compiler-Flags für alle Targets
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Og -DDEVELOP")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# OpenSSL und Threads (wird von allen Subprojekten benötigt)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Unity Build Option (optional, kann per -DUSE_UNITY_BUILD=ON aktiviert werden)
option(USE_UNITY_BUILD "Use Unity Build for faster compilation" OFF)
if(USE_UNITY_BUILD)
    set(CMAKE_UNITY_BUILD ON)
    set(CMAKE_UNITY_BUILD_BATCH_SIZE 8)
endif()

message(STATUS "TCP_ServerClient - Unified Build Configuration")
message(STATUS "C++ Standard: C++17")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenSSL found: ${OPENSSL_FOUND}")
message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
message(STATUS "Threads found: ${Threads_FOUND}")

# ============================================================================
# Googletest Submodule (wird für die Tests benötigt)
# ============================================================================
set(INSTALL_GTEST OFF CACHE BOOL "Disable gtest installation")
set(BUILD_GMOCK ON CACHE BOOL "Build googlemock (includes gtest)")
add_subdirectory(test/gtest/googletest)

# ============================================================================
# Projekt-Subverzeichnisse
# ============================================================================

# Beispiel-Programme (server, client)
add_subdirectory(example)

# FTP Server (Debug)
add_subdirectory(src/ftp)

# GTest-basierte Tests (nach googletest, da diese davon abhängen)
add_subdirectory(test/gtest)

# ============================================================================
# Zusammenfassung der verfügbaren Targets
# ============================================================================
message(STATUS "")
message(STATUS "Available build targets:")
message(STATUS "  - server (example)")
message(STATUS "  - client (example)")
message(STATUS "  - ftp-dev (src/ftp)")
message(STATUS "  - test (test/gtest)")
message(STATUS "  - gtest, gtest_main, gmock, gmock_main (test/gtest/googletest)")
message(STATUS "")
message(STATUS "To build all: cmake --build .")
message(STATUS "To build specific target: cmake --build . --target <target>")
message(STATUS "")
