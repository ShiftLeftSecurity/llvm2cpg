set (SOURCES
  ObjCTraversal.cpp
)

add_library(Traversals ${SOURCES})
target_compile_options(Traversals PRIVATE ${LLVM2CPG_CXX_FLAGS})
target_include_directories(Traversals PUBLIC ${LLVM2CPG_INCLUDE_DIRS})
target_include_directories(Traversals SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
if (LLVM IN_LIST LLVM_AVAILABLE_LIBS)
  target_link_libraries(Traversals LLVM)
else()
  target_link_libraries(Traversals LLVMCore)
endif()
target_link_libraries(Traversals CPGLogger)