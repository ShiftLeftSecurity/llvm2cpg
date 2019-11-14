set (SOURCES
  CPGEmitter.cpp
  CPGProtoWriter.cpp
  CPGProtoAdapter.cpp
  CPGProtoBuilder.cpp
  CPGProtoNode.cpp
  CPGTypeEmitter.cpp
  )

add_library(CPGProtoWriter ${SOURCES})
target_compile_options(CPGProtoWriter PRIVATE ${LLVM2CPG_CXX_FLAGS})
target_link_libraries(CPGProtoWriter CPG CPGProto CPGLogger CPGDemangler protobuf zip)
target_include_directories(CPGProtoWriter SYSTEM PRIVATE /usr/local/include)