set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/if_without_locals.c
  ${CMAKE_CURRENT_LIST_DIR}/if_with_locals.c
  ${CMAKE_CURRENT_LIST_DIR}/if_with_more_locals.c
  )

foreach(source ${SOURCES})
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION ll
    FLAGS -c -emit-llvm -S -fno-discard-value-names
  )
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION opt.ll
    FLAGS -c -emit-llvm -S -fno-discard-value-names -O1
  )
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION bc
    FLAGS -c -emit-llvm -fno-discard-value-names
  )
endforeach()

