# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(example VERSION 3.0.0 DESCRIPTION "Example of using the TCP client and server classes")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../build/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../build/lib)

file(GLOB sourcefiles_server server.cpp)
file(GLOB sourcefiles_client client.cpp)
add_executable(server ${sourcefiles_server})
add_executable(client ${sourcefiles_client})

target_compile_options(server PRIVATE -fexceptions -Wall $<$<NOT:$<CONFIG:Debug>>: -O3> $<$<CONFIG:DEBUG>: -g -Og -DDEVELOP>)
target_compile_options(client PRIVATE -fexceptions -Wall $<$<NOT:$<CONFIG:Debug>>: -O3> $<$<CONFIG:DEBUG>: -g -Og -DDEVELOP>)

target_link_libraries(server -lcrypto -lcrypt -lssl -pthread)
target_link_libraries(client -lcrypto -lcrypt -lssl -pthread)

# Message build type
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
if ("${CMAKE_BUILD_TYPE_UPPER}" STREQUAL "DEBUG")
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE_UPPER}")
endif()

# ---------- Commands to be executed when creating makefile ----------

# Create TLS certificates using EC
execute_process(COMMAND bash -c "echo 'Create TLS certificates ...'")
execute_process(COMMAND bash -c "${CMAKE_CURRENT_LIST_DIR}/CreateCerts.sh")
execute_process(COMMAND bash -c "echo 'Done.'")

# Install the entire project
if ("$ENV{USER}" STREQUAL "root")
    execute_process(COMMAND bash -c "echo 'Installing the project ...'")
    execute_process(COMMAND make WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
else()
    execute_process(COMMAND bash -c "echo 'Installing the project as root ...'")
    execute_process(COMMAND sudo make WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()
