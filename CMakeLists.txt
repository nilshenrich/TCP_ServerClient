# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."
# For local installation please use the command "cmake -DCMAKE_BUILD_TYPE=Local .."

# Minimum required cmake version
cmake_minimum_required(VERSION 3.10)

# Projects to be built
project(tcpserver VERSION 2.0 DESCRIPTION "TCP server")
project(tcpclient VERSION 2.0 DESCRIPTION "TCP client")
project(tlsserver VERSION 2.0 DESCRIPTION "TLS server")
project(tlsclient VERSION 2.0 DESCRIPTION "TLS client")

# Include standard GNU install directories
include(GNUInstallDirs)

# The deault build type is Release
# For Release install in default location, for Debug install in build folder locally
# If build type is other that Release or Debug, throw an error
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
if(CMAKE_BUILD_TYPE MATCHES "^(Debug|Local)$")
  set(PATH_INSTALL_LIB ${CMAKE_CURRENT_SOURCE_DIR}/build/lib)
  set(PATH_INSTALL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/build/include/tcp_serverclient)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(PATH_INSTALL_LIB ${CMAKE_INSTALL_LIBDIR})
  set(PATH_INSTALL_INCLUDE ${CMAKE_INSTALL_INCLUDEDIR}/tcp_serverclient)
else()
  message(FATAL_ERROR "CMAKE_BUILD_TYPE must be 'Release' or 'Debug'")
endif()

# Source files to be compiled
add_library(tcpserver SHARED src/tcp/server/TcpServer.cpp)
add_library(tcpclient SHARED src/tcp/client/TcpClient.cpp)
add_library(tlsserver SHARED src/tls/server/TlsServer.cpp)
add_library(tlsclient SHARED src/tls/client/TlsClient.cpp)

# Set include directories
include_directories(src)

# Set compiler flags
target_compile_options(tcpserver PUBLIC -fexceptions $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og> $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:LOCAL>>: -O3>)
target_compile_options(tcpclient PUBLIC -fexceptions $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og> $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:LOCAL>>: -O3>)
target_compile_options(tlsserver PUBLIC -fexceptions $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og> $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:LOCAL>>: -O3>)
target_compile_options(tlsclient PUBLIC -fexceptions $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og> $<$<OR:$<CONFIG:RELEASE>,$<CONFIG:LOCAL>>: -O3>)
target_link_options(tlsserver PUBLIC $<$<CONFIG:RELEASE>: -s>)
target_link_options(tlsclient PUBLIC $<$<CONFIG:RELEASE>: -s>)
target_link_options(tcpserver PUBLIC $<$<CONFIG:RELEASE>: -s>)
target_link_options(tcpclient PUBLIC $<$<CONFIG:RELEASE>: -s>)

# Compiler options
set_target_properties(tcpserver PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR} CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
set_target_properties(tcpclient PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR} CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
set_target_properties(tlsserver PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR} CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
set_target_properties(tlsclient PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR} CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

# Component to be compiled can be specified via make target
# No make target will build all components
# e.g. make tcp -> will build tcp server and tcp client
# e.g. make tls -> will build tls server and tls client
# e.g. make server -> will build tcp server and tls server
# e.g. make client -> will build tcp client and tls client
# e.g. make tcpserver -> will build tcp server
# e.g. make tcpclient -> will build tcp client
# e.g. make tlsserver -> will build tls server
# e.g. make tlsclient -> will build tls client
# e.g. make all -> will build all components
# e.g. make -> will build all components
add_custom_target(tcp DEPENDS tcpserver tcpclient COMMENT "Just build TCP server and client")
add_custom_target(tls DEPENDS tlsserver tlsclient COMMENT "Just build TLS server and client")
add_custom_target(server DEPENDS tcpserver tlsserver COMMENT "Just build TCP server and TLS server")
add_custom_target(client DEPENDS tcpclient tlsclient COMMENT "Just build TCP client and TLS client")

# Install library
install(TARGETS tcpserver tcpclient tlsserver tlsclient LIBRARY DESTINATION ${PATH_INSTALL_LIB})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/general ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp ${CMAKE_CURRENT_SOURCE_DIR}/src/tls DESTINATION ${PATH_INSTALL_INCLUDE} FILES_MATCHING PATTERN "*.h" PATTERN "*.tpp")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
