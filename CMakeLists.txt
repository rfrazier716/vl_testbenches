cmake_minimum_required (VERSION 3.10)
project ("vl-testbenches")
include(CTest)

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

# Path Definitions used in submodules
set(VL_TEST_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(VL_TEST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/inc")
set(VL_TEST_BIN_DIR "${CMAKE_SOURCE_DIR}/bin")
set(VL_TEST_LIB_DIR "${CMAKE_SOURCE_DIR}/lib")


# Add the Include path to the include search directory
set(VL_INC_DIR "/usr/share/verilator/include")
include_directories(
        ${VL_TEST_INCLUDE_DIR}  # include directory for testbench
        ${VL_INC_DIR}       # include directory for verilator headers
)

# add the library as a target
add_subdirectory("${VL_TEST_SOURCE_DIR}")

# Add unittests as a target
add_subdirectory("${CMAKE_SOURCE_DIR}/test")
