cmake_minimum_required (VERSION 3.10)
project ("tests")

# Find the Boost Library Installation
find_package(Boost REQUIRED COMPONENTS system)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Load the Catch2 Package, which is used for all unittests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)

# Load the Verilator Package so that we can verilate a simple RTL and confirm our testbench
find_package(verilator HINTS $ENV{VERILATOR_ROOT})


# compile catchMain as a library for quicker compilation times
add_library(catch_main "${CMAKE_CURRENT_LIST_DIR}/catchMain.cpp")

# Create the test target
add_executable(tests
        "${CMAKE_CURRENT_LIST_DIR}/test_synchronous_testbench.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/interfaces/test_IOInterface.cpp"
        )

# verilate and link the d-flipflop library to the test
verilate(tests SOURCES "${CMAKE_CURRENT_LIST_DIR}/rtl/d_flip_flop.v" TRACE )

target_link_libraries(tests PUBLIC
        interfaces
        catch_main)


# Register Ctests
set(TEST_TAGS "[testbench]")

foreach(tag ${TEST_TAGS})
    catch_discover_tests(tests
            TEST_SPEC "${tag}"
            TEST_PREFIX "${tag}-")
endforeach()



