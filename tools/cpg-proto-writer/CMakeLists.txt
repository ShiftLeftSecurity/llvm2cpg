set (SOURCES
  CPGProtoWriterTool.cpp
  )

add_executable(cpg-proto-writer ${SOURCES})
target_link_libraries(cpg-proto-writer CPG CPGProtoWriter LLVMSupport)
target_link_directories(cpg-proto-writer PRIVATE /usr/local/lib)
target_compile_options(cpg-proto-writer PRIVATE ${LLVM2CPG_CXX_FLAGS} -fno-rtti)
