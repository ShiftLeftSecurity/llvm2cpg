set (SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/class_method_names.cpp
)

foreach(source ${SOURCES})
  compile_fixture(
    COMPILER ${LLVM_CXX_COMPILER}
    INPUT ${source}
    OUTPUT_EXTENSION ll
    FLAGS -c -emit-llvm -S -fno-discard-value-names
  )
endforeach()