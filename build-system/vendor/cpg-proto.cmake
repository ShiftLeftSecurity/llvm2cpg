set (PROTO_INPUT ${CMAKE_SOURCE_DIR}/cpg.proto)
set (PROTO_OUTPUT_DIR ${CMAKE_BINARY_DIR}/CPGProto)
set (PROTO_OUTPUT_SOURCES ${PROTO_OUTPUT_DIR}/cpg.pb.h ${PROTO_OUTPUT_DIR}/cpg.pb.cc)

file (MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

add_custom_command(OUTPUT ${PROTO_OUTPUT_SOURCES}
  COMMAND  protoc --cpp_out=${PROTO_OUTPUT_DIR} --proto_path=${CMAKE_SOURCE_DIR} ${PROTO_INPUT}
  DEPENDS ${PROTO_INPUT}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )

add_library(CPGProto ${PROTO_OUTPUT_SOURCES})
target_link_libraries(CPGProto protobuf)
target_include_directories(CPGProto PUBLIC ${PROTO_OUTPUT_DIR})
target_include_directories(CPGProto SYSTEM PRIVATE /usr/local/include)