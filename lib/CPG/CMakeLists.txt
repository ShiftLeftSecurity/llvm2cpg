set (SOURCES
  BitcodeLoader.cpp
  CPGFile.cpp
  CPGMethod.cpp
  CPG.cpp
  CPGInstVisitor.cpp
  )

add_library(CPG ${SOURCES})
target_compile_options(CPG PRIVATE ${LLVM2CPG_CXX_FLAGS})
target_include_directories(CPG PUBLIC ${LLVM2CPG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
target_link_libraries(CPG LLVMCore LLVMBitReader)
