set (SOURCES
  Transforms.cpp
  )

add_library(Transforms ${SOURCES})
target_compile_options(Transforms PRIVATE ${LLVM2CPG_CXX_FLAGS})
# TODO: Split into public/system
target_include_directories(Transforms PUBLIC ${LLVM2CPG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
target_link_libraries(Transforms LLVMCore)