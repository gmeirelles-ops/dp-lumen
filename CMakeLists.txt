cmake_minimum_required(VERSION 3.16)
project(dp-lumen C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(DP_LUMEN_BUILD_TESTS "Build host unit tests" ON)

if(DEFINED ENV{RA08H_SDK_PATH} AND EXISTS "$ENV{RA08H_SDK_PATH}/CMakeLists.txt")
    set(DP_LUMEN_PLATFORM_RA08H ON)
    add_subdirectory("$ENV{RA08H_SDK_PATH}" ra08h_sdk EXCLUDE_FROM_ALL)
else()
    set(DP_LUMEN_PLATFORM_STUB ON)
    add_compile_definitions(DP_LUMEN_PLATFORM_STUB=1)
    message(STATUS "RA08H_SDK_PATH not set — building with platform stub (host gcc)")
endif()

add_subdirectory(components)
add_subdirectory(main)

if(DP_LUMEN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/unit)
endif()
