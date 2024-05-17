# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

cmake_minimum_required(VERSION 3.10)

project(tcp) # Never used
add_subdirectory(Server) # Compile server library
add_subdirectory(Client) # Compile client library
