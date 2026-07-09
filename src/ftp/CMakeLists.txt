# DEV: Debugging file to be deleted

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ftp-dev VERSION 0.0.0 DESCRIPTION "Debugging FTP server")

include_directories(
    ${CMAKE_SOURCE_DIR}/src/basic
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(ftp-dev FtpServer.cpp test.cpp)
target_compile_options(ftp-dev PRIVATE -fexceptions -Wall -g -Og -DDEVELOP)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(ftp-dev 
    OpenSSL::SSL 
    OpenSSL::Crypto 
    ${OPENSSL_LIBRARIES}
    Threads::Threads
    crypt
)
