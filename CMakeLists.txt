cmake_minimum_required(VERSION 3.14)
project(dimeta)

set(CMAKE_CXX_STANDARD 17)

set(include_files
        "include/dimeta/dimeta.hpp"
        "include/dimeta/primitives.hpp"
        "include/dimeta/future.hpp"
        "include/dimeta/scheduling.hpp"
        "include/dimeta/detail/mpl/map.hpp"
        "include/dimeta/detail/mpl/sequence.hpp"
        "include/dimeta/detail/consistency.hpp")

add_library(dimeta INTERFACE)
target_sources(dimeta INTERFACE ${include_files})

target_include_directories(dimeta INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

add_subdirectory(extern EXCLUDE_FROM_ALL)

option(DIMETA_CONSISTENCY_CHECKS "whether or not to enable internal state validation" ON)
if (DIMETA_CONSISTENCY_CHECKS)
    target_compile_definitions(dimeta INTERFACE DIMETA_CONSISTENCY_CHECKS)
endif()

option(DIMETA_BUILD_TESTS "build tests" ON)
if (DIMETA_BUILD_TESTS)
    add_subdirectory(test)
endif()