set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/select_inst.ll
  ${CMAKE_CURRENT_LIST_DIR}/gep_flat_struct.ll
  ${CMAKE_CURRENT_LIST_DIR}/gep_array.ll
  ${CMAKE_CURRENT_LIST_DIR}/gep_mixed.ll
  ${CMAKE_CURRENT_LIST_DIR}/fneg_inst.ll
  ${CMAKE_CURRENT_LIST_DIR}/phi.ll
  ${CMAKE_CURRENT_LIST_DIR}/const_null.ll
  ${CMAKE_CURRENT_LIST_DIR}/store_const_expr.ll
  ${CMAKE_CURRENT_LIST_DIR}/branches.ll
  ${CMAKE_CURRENT_LIST_DIR}/switches.ll
  ${CMAKE_CURRENT_LIST_DIR}/atomics.ll
  ${CMAKE_CURRENT_LIST_DIR}/fence.ll
  ${CMAKE_CURRENT_LIST_DIR}/vectors.ll
  ${CMAKE_CURRENT_LIST_DIR}/unreachable.ll
  ${CMAKE_CURRENT_LIST_DIR}/nested_const_expr.ll
  ${CMAKE_CURRENT_LIST_DIR}/aggregate_access.ll
  ${CMAKE_CURRENT_LIST_DIR}/alias.ll
  ${CMAKE_CURRENT_LIST_DIR}/inline_asm.ll
  ${CMAKE_CURRENT_LIST_DIR}/constants.ll
  ${CMAKE_CURRENT_LIST_DIR}/malformed_utf8.ll
  ${CMAKE_CURRENT_LIST_DIR}/llvm_structs.ll
  ${CMAKE_CURRENT_LIST_DIR}/struct_member_access.ll

  ${CMAKE_CURRENT_LIST_DIR}/struct_merging_1.ll
  ${CMAKE_CURRENT_LIST_DIR}/struct_merging_2.ll
  ${CMAKE_CURRENT_LIST_DIR}/self_referencing_global.ll
  )

foreach(source ${SOURCES})
  compile_fixture(
    COMPILER ${LLVM_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION bc
    FLAGS -c -emit-llvm -fno-discard-value-names
  )
endforeach()
