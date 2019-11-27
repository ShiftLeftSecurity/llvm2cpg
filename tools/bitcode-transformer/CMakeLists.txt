set (SOURCES
  BitcodeTransformerTool.cpp
  )

add_executable(bitcode-transformer ${SOURCES})
target_compile_options(bitcode-transformer PRIVATE ${LLVM2CPG_CXX_FLAGS} -fno-rtti)


if (LLVM IN_LIST LLVM_AVAILABLE_LIBS)
  target_link_libraries(bitcode-transformer CPG Transforms CPGLogger LLVM)
else()
  target_link_libraries(bitcode-transformer CPG Transforms CPGLogger 
  LLVMBitWriter)
endif()