cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(example VERSION 1.0 DESCRIPTION "Example of using the TCP client and server classes")

include_directories(../Server/include ../Client/include)

file(GLOB sourcefiles_server "../Server/src/*.cpp" server.cpp)
file(GLOB sourcefiles_client "../Client/src/*.cpp" client.cpp)
add_executable(server ${sourcefiles_server})
add_executable(client ${sourcefiles_client})

target_compile_options(server PRIVATE -fexceptions -Wall -g -Og
    $<$<CONFIG:DEBUG>: -DDEVELOP>)
target_compile_options(client PRIVATE -fexceptions -Wall -g -Og
    $<$<CONFIG:DEBUG>: -DDEVELOP>)

target_link_libraries(server -lcrypto -lcrypt -lssl -pthread)
target_link_libraries(client -lcrypto -lcrypt -lssl -pthread)

# Create TLS certificates using EC
execute_process(COMMAND bash -c "echo 'Create TLS certificates ...'")
execute_process(COMMAND bash -c "${CMAKE_CURRENT_LIST_DIR}/CreateCerts.sh")
execute_process(COMMAND bash -c "echo 'Done.'")
