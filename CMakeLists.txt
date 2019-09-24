cmake_minimum_required(VERSION 3.14)
project(dimeta)

set(CMAKE_CXX_STANDARD 17)

set(macro_files
        "include/dimeta/macro/primitives.hpp"
        "include/dimeta/macro/delay.hpp"
        "include/dimeta/macro/compiler.hpp"
        "include/dimeta/macro/simulator.hpp"
        "include/dimeta/macro/macro.hpp")

set(block_files
        "include/dimeta/function/and_function.hpp"
        "include/dimeta/function/nand_function.hpp"
        "include/dimeta/function/or_function.hpp"
        "include/dimeta/function/nor_function.hpp"
        "include/dimeta/function/xor_function.hpp"
        "include/dimeta/function/buffer_function.hpp"
        "include/dimeta/function/not_function.hpp")

set(simulator_files
        "include/dimeta/delay.hpp"
        "include/dimeta/primitives.hpp"
        "include/dimeta/future.hpp"
        "include/dimeta/scheduling.hpp"
        "include/dimeta/simulator.hpp"
        "include/dimeta/dimeta.hpp"
        "include/dimeta/detail/mpl/map.hpp"
        "include/dimeta/detail/mpl/sequence.hpp"
        "include/dimeta/detail/mpl/set.hpp"
        "include/dimeta/detail/consistency.hpp")

set(include_files ${simulator_files} ${block_files} ${macro_files})

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