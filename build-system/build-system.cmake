set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_LIST_DIR}/vendor/vendor.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/tests/tests.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cpack.cmake)

set (LLVM2CPG_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/include
  )


set (LLVM2CPG_CXX_FLAGS
  -std=c++14
  -Wall
  -Werror
  -fno-exceptions
  -fvisibility=hidden
  -fvisibility-inlines-hidden
  )

include(${CMAKE_CURRENT_LIST_DIR}/version.cmake)
