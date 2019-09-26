set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/select_inst.ll
  )

foreach(source ${SOURCES})
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION bc
    FLAGS -c -emit-llvm -fno-discard-value-names
  )
endforeach()