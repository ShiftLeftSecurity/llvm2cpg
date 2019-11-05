set (SOURCES
  CPGProtoWriterTool.cpp
        FileType.cpp FileType.h)

add_executable(cpg-proto-writer ${SOURCES})

if (LLVM IN_LIST LLVM_AVAILABLE_LIBS)
  target_link_libraries(cpg-proto-writer CPG CPGProtoWriter LLVM)
else()
  target_link_libraries(cpg-proto-writer CPG CPGProtoWriter LLVMSupport)
endif()


target_link_directories(cpg-proto-writer PRIVATE /usr/local/lib)
target_compile_options(cpg-proto-writer PRIVATE ${LLVM2CPG_CXX_FLAGS} -fno-rtti)

install(TARGETS cpg-proto-writer
  RUNTIME DESTINATION .
)