set (SOURCES
  Transforms.cpp
  )

add_library(Transforms ${SOURCES})
target_compile_options(Transforms PRIVATE ${LLVM2CPG_CXX_FLAGS})
# TODO: Split into public/system
target_include_directories(Transforms PUBLIC ${LLVM2CPG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})

if (LLVM IN_LIST LLVM_AVAILABLE_LIBS)
  target_link_libraries(Transforms LLVM)
else()
  target_link_libraries(Transforms LLVMCore LLVMTransformUtils)
endif()
target_link_libraries(Transforms Traversals)