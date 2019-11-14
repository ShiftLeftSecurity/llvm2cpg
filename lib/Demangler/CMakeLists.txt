set (SOURCES
  Demangler.cpp
)

add_library(Demangler ${SOURCES})
target_compile_options(Demangler PRIVATE ${LLVM2CPG_CXX_FLAGS})
target_include_directories(Demangler PRIVATE ${CMAKE_SOURCE_DIR}/include)