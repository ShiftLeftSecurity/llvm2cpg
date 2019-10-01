set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/select_inst.ll
  ${CMAKE_CURRENT_LIST_DIR}/gep_flat_struct.ll
  ${CMAKE_CURRENT_LIST_DIR}/gep_array.ll
  ${CMAKE_CURRENT_LIST_DIR}/gep_mixed.ll
  ${CMAKE_CURRENT_LIST_DIR}/fneg_inst.ll
  )

foreach(source ${SOURCES})
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION bc
    FLAGS -c -emit-llvm -fno-discard-value-names
  )
endforeach()