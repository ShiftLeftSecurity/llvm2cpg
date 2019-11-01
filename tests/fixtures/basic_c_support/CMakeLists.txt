set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/return_constant.c
  ${CMAKE_CURRENT_LIST_DIR}/return_parameter.c
  ${CMAKE_CURRENT_LIST_DIR}/return_multiplied_parameter.c
  ${CMAKE_CURRENT_LIST_DIR}/return_conditional.c
  ${CMAKE_CURRENT_LIST_DIR}/return_global.c
  ${CMAKE_CURRENT_LIST_DIR}/return_cast.c
  ${CMAKE_CURRENT_LIST_DIR}/switch.c
  ${CMAKE_CURRENT_LIST_DIR}/gep_flat_struct.c
  ${CMAKE_CURRENT_LIST_DIR}/gep_array.c
  ${CMAKE_CURRENT_LIST_DIR}/call_int.c
  ${CMAKE_CURRENT_LIST_DIR}/call_string.c
  ${CMAKE_CURRENT_LIST_DIR}/call_unknown_function.c
  ${CMAKE_CURRENT_LIST_DIR}/call_function_pointer.c
  ${CMAKE_CURRENT_LIST_DIR}/phi.c
  ${CMAKE_CURRENT_LIST_DIR}/dataflow_caller.c
  ${CMAKE_CURRENT_LIST_DIR}/dataflow_caller_arguments.c
  ${CMAKE_CURRENT_LIST_DIR}/inline_asm.c
  )

set (SOURCES_DBG
  ${CMAKE_CURRENT_LIST_DIR}/vectorfind.c
)

set (SOURCES_DBG_OPT
  ${CMAKE_CURRENT_LIST_DIR}/vectorfind.c
)


foreach(source ${SOURCES})
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION bc
    FLAGS -c -emit-llvm -fno-discard-value-names
  )
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION ll
    FLAGS -c -emit-llvm -S -fno-discard-value-names
  )
endforeach()


foreach(source ${SOURCES_DBG})
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION dbg.bc
    FLAGS -c -emit-llvm -g
  )
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION dbg.ll
    FLAGS -c -emit-llvm -S -g
  )
endforeach()


foreach(source ${SOURCES_DBG_OPT})
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION o3.dbg.bc
    FLAGS -c -emit-llvm -gfull -gdwarf-5 -O3 -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer
  )
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION o3.dbg.ll
    FLAGS -c -emit-llvm -S -gfull -gdwarf-5 -O3 -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer
  )
endforeach()