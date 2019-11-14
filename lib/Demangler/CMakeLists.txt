set (SOURCES
  Demangler.cpp
  CPGDemangler.cpp
)

add_library(CPGDemangler ${SOURCES})
target_compile_options(CPGDemangler PRIVATE ${LLVM2CPG_CXX_FLAGS})
target_include_directories(CPGDemangler PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(CPGDemangler SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})