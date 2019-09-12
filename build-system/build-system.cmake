include(${CMAKE_CURRENT_LIST_DIR}/vendor/vendor.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/fixtures.cmake)

set (LLVM2CPG_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/include
  )

set (LLVM2CPG_CXX_FLAGS
  -std=c++14
  -Wall
  -Werror
  -fno-exceptions
  )