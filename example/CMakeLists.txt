cmake_minimum_required(VERSION 3.10)

project(example VERSION 1.0 DESCRIPTION "Example of using the TCP client and server classes")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server server.cpp)
add_executable(client client.cpp)

target_link_libraries(server -lTcpServer -lTlsServer -lcrypto -lcrypt -lssl -pthread)
target_link_libraries(client -lTcpClient -lTlsClient -lcrypto -lcrypt -lssl -pthread)
