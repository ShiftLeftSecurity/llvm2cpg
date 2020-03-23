set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/objc_classes.m
  ${CMAKE_CURRENT_LIST_DIR}/objc_methods.m
  ${CMAKE_CURRENT_LIST_DIR}/objc_external_class.m
  ${CMAKE_CURRENT_LIST_DIR}/objc_external_class_2.m
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
  ${CMAKE_CURRENT_LIST_DIR}/objc_classes.ll
  ${CMAKE_CURRENT_LIST_DIR}/objc_methods.ll
  ${CMAKE_CURRENT_LIST_DIR}/objc_external_class.ll
  ${CMAKE_CURRENT_LIST_DIR}/objc_external_class_2.ll
  ${CMAKE_CURRENT_LIST_DIR}/objc_external_defined_class.ll
  ${CMAKE_CURRENT_LIST_DIR}/objc_categories.ll
  ${CMAKE_CURRENT_LIST_DIR}/objc_categories_external.ll
)

foreach(source ${IR_SOURCES})
  compile_fixture(
    COMPILER ${CMAKE_C_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION bc
    FLAGS -c -emit-llvm -fno-discard-value-names
  )
endforeach()