set (SOURCES
  llvm2cpgTool.cpp
  FileType.cpp
  FileType.h
  )

add_executable(llvm2cpg ${SOURCES})

if (LLVM IN_LIST LLVM_AVAILABLE_LIBS)
  target_link_libraries(llvm2cpg CPG CPGProtoWriter CPGLogger LLVM z)
else()
  target_link_libraries(llvm2cpg CPG CPGProtoWriter CPGLogger LLVMSupport LLVMLinker LLVMPasses LLVMAnalysis LLVMTransformUtils LLVMCore z)
endif()


target_link_directories(llvm2cpg PRIVATE /usr/local/lib)
target_compile_options(llvm2cpg PRIVATE ${LLVM2CPG_CXX_FLAGS} -fno-rtti)

install(TARGETS llvm2cpg
  RUNTIME DESTINATION .
)
