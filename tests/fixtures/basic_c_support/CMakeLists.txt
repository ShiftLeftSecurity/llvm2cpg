set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/return_constant.c
  ${CMAKE_CURRENT_LIST_DIR}/return_parameter.c
  ${CMAKE_CURRENT_LIST_DIR}/return_multiplied_parameter.c
  ${CMAKE_CURRENT_LIST_DIR}/return_conditional.c
  ${CMAKE_CURRENT_LIST_DIR}/return_global.c
  ${CMAKE_CURRENT_LIST_DIR}/return_cast.c
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
