set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/return_constant.c
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