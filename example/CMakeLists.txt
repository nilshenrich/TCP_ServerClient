# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=DEBUG .."

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(example VERSION 3.2.1 DESCRIPTION "Example of using the TCP client and server classes")

include_directories(
    ${CMAKE_SOURCE_DIR}/src/basic
)

add_executable(server server.cpp)
add_executable(client client.cpp)

target_compile_options(server PRIVATE -fexceptions -Wall $<$<NOT:$<CONFIG:DEBUG>>: -O3> $<$<CONFIG:DEBUG>: -g -Og -DDEVELOP>)
target_compile_options(client PRIVATE -fexceptions -Wall $<$<NOT:$<CONFIG:DEBUG>>: -O3> $<$<CONFIG:DEBUG>: -g -Og -DDEVELOP>)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(server OpenSSL::SSL ${OPENSSL_LIBRARIES} Threads::Threads)
target_link_libraries(client OpenSSL::SSL ${OPENSSL_LIBRARIES} Threads::Threads)

# Message build type
if ("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    message(STATUS "Build type: DEBUG")
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
