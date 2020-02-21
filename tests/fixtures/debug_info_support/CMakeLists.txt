set (C_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/debug_struct_members.c
  ${CMAKE_CURRENT_LIST_DIR}/debug_anonymous_struct_members.c
  ${CMAKE_CURRENT_LIST_DIR}/debug_nested_struct_members.c
)

foreach(source ${C_SOURCES})
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION ll
    FLAGS -c -S -emit-llvm -g
  )
endforeach()
