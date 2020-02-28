cmake_minimum_required(VERSION 3.13)

project(llvm2cpg
        VERSION 0.4.1
        LANGUAGES C CXX
        )

include(${CMAKE_CURRENT_LIST_DIR}/build-system/build-system.cmake)

add_subdirectory(lib)
add_subdirectory(tests)
add_subdirectory(tools)