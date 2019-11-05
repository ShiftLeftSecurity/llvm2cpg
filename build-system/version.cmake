execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND date "+%d %b %Y"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE BUILD_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file (
  ${CMAKE_SOURCE_DIR}/lib/CPG/Version.cpp
  ${CMAKE_BINARY_DIR}/lib/CPG/Version.cpp
  @ONLY
)

add_library(CPGVersion ${CMAKE_BINARY_DIR}/lib/CPG/Version.cpp)
target_compile_options(CPGVersion PRIVATE ${LLVM2CPG_CXX_FLAGS} -fno-rtti)
target_include_directories(CPGVersion PUBLIC ${LLVM2CPG_INCLUDE_DIRS})
target_include_directories(CPGVersion SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})

if (LLVM IN_LIST LLVM_AVAILABLE_LIBS)
  target_link_libraries(CPGVersion LLVM)
else()
  target_link_libraries(CPGVersion LLVMSupport)
endif()
