# DEV: Debugging file to be deleted

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ftp-dev VERSION 0.0.0 DESCRIPTION "Debugging FTP server")
add_executable(ftp-dev FtpServer.cpp test.cpp)
target_compile_options(ftp-dev PRIVATE -fexceptions -Wall -g -Og -DDEVELOP)
target_link_libraries(ftp-dev -lcrypto -lcrypt -lssl -pthread)
