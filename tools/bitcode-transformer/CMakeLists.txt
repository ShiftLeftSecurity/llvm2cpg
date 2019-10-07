set (SOURCES
  BitcodeTransformerTool.cpp
  )

add_executable(bitcode-transformer ${SOURCES})
target_compile_options(bitcode-transformer PRIVATE ${LLVM2CPG_CXX_FLAGS} -fno-rtti)
target_link_libraries(bitcode-transformer CPG Transforms LLVMBitWriter)