cmake_minimum_required (VERSION 3.10)
project ("vl_test")

# Find the Boost Library Installation
find_package(Boost REQUIRED COMPONENTS system)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# build the sublibraries
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/interfaces)

# Create a library from the source files
add_library(vl_test
        ${VL_TEST_SOURCE_DIR}/udp_server.cpp)

target_link_libraries(vl_test interfaces)