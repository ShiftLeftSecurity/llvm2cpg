add_library(LLVMExt TypeEquality.cpp)
target_compile_options(LLVMExt PRIVATE ${LLVM2CPG_CXX_FLAGS})
target_include_directories(LLVMExt PUBLIC ${LLVM2CPG_INCLUDE_DIRS})
target_include_directories(LLVMExt SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})

if (LLVM IN_LIST LLVM_AVAILABLE_LIBS)
  target_link_libraries(LLVMExt CPGLogger LLVM)
else()
  target_link_libraries(LLVMExt CPGLogger
    LLVMSupport
    LLVMCore
  )
endif()