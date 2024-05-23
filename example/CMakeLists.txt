cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(example VERSION 2.0 DESCRIPTION "Example of using the TCP client and server classes")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../build/include)

file(GLOB sourcefiles_server server.cpp)
file(GLOB sourcefiles_client client.cpp)
add_executable(server ${sourcefiles_server})
add_executable(client ${sourcefiles_client})

target_compile_options(server PRIVATE -fexceptions -Wall -g -Og
    $<$<CONFIG:DEBUG>: -DDEVELOP>)
target_compile_options(client PRIVATE -fexceptions -Wall -g -Og
    $<$<CONFIG:DEBUG>: -DDEVELOP>)

target_link_libraries(server -lcrypto -lcrypt -lssl -pthread -ltcpserver -ltlsserver)
target_link_libraries(client -lcrypto -lcrypt -lssl -pthread -ttcpclient -ltlsclient)

# ---------- Commands to be executed when creating makefile ----------

# Create TLS certificates using EC
execute_process(COMMAND bash -c "echo 'Create TLS certificates ...'")
execute_process(COMMAND bash -c "${CMAKE_CURRENT_LIST_DIR}/CreateCerts.sh")
execute_process(COMMAND bash -c "echo 'Done.'")

# Install the entire project with build type 'Local'
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../build)
    execute_process(COMMAND mkdir build WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)    
endif()
execute_process(COMMAND cmake -DCMAKE_BUILD_TYPE=Local .. WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build)
execute_process(COMMAND make install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build)
