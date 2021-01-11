cmake_minimum_required (VERSION 3.10)
project ("vl_interfaces")

# Create a library from the source files
add_library(interfaces
        ${CMAKE_CURRENT_LIST_DIR}/VirtualUart.cpp)