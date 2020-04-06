set (SOURCES
)

foreach(source ${SOURCES})
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION ll
    FLAGS -c -emit-llvm -S -fno-discard-value-names
  )
endforeach()

set (IR_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/objc_call_00_definition.ll
  ${CMAKE_CURRENT_LIST_DIR}/objc_call_00_usage.ll
  )

foreach(source ${IR_SOURCES})
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION bc
    FLAGS -c -emit-llvm -fno-discard-value-names
  )
endforeach()