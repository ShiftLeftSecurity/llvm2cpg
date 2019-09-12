set (SOURCES
  CPGProtoWriter.cpp
  CPGProtoAdapter.cpp
  )

add_library(CPGProtoWriter ${SOURCES})
target_compile_options(CPGProtoWriter PRIVATE ${LLVM2CPG_CXX_FLAGS})
target_link_libraries(CPGProtoWriter CPG CPGProto protobuf zip)