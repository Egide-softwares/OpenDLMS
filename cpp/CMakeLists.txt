cmake_minimum_required(VERSION 3.10)
project(OpenDLMS VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 1. Collect Sources
file(GLOB_RECURSE SRC_FILES 
    "src/*.cpp"
    "src/common/*.cpp"
    "src/dlms/*.cpp"
    "src/cosem/*.cpp"
    "src/transport/*.cpp"
)

# 2. Find Dependencies
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ASIO REQUIRED asio)

# 3. Build the Static Library
add_library(opendlms STATIC ${SRC_FILES})

# This tells CMake: anyone using opendlms needs these headers and thread libs
target_include_directories(opendlms PUBLIC include ${ASIO_INCLUDE_DIRS})
target_link_libraries(opendlms PUBLIC ${CMAKE_THREAD_LIBS_INIT})

# 4. Add Examples
# Add the TCP HDLC test executable
add_executable(tcp_hdlc_client examples/tcp_hdlc_client.cpp)
target_link_libraries(tcp_hdlc_client PRIVATE opendlms)

# Add the TCP Wrapper test executable
add_executable(tcp_wrapper_client examples/tcp_wrapper_client.cpp)
target_link_libraries(tcp_wrapper_client PRIVATE opendlms)

# (Optional) Add a custom command to create a symlink or print usage
add_custom_target(print_usage ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Build complete."
    COMMAND ${CMAKE_COMMAND} -E echo "Run HDLC: ./tcp_hdlc_client - Simulator without -w"
    COMMAND ${CMAKE_COMMAND} -E echo "Run Wrapper: ./tcp_wrapper_client - Simulator with -w"
    VERBATIM
)

enable_testing()