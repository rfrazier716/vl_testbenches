cmake_minimum_required (VERSION 3.10)
project ("vl-testbenches")

# Set the build directory to be ./build
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

# Uncomment to build Test files
SET(ENABLE_TESTS TRUE)

# set to cpp 14 standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the Boost Library Installation
find_package(Boost REQUIRED COMPONENTS system)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Find the Verilator Package and add necessary Definitions

# Add the Verilator Include path to the include search directory
include_directories(
        "${CMAKE_SOURCE_DIR}/inc")            # include directory for testbench

IF(${ENABLE_TESTS})
    enable_testing()
    INCLUDE("cmake/tests.cmake")
ENDIF()