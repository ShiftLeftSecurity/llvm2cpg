set (source ${CMAKE_CURRENT_LIST_DIR}/bitcode_extraction.c)

compile_fixture(
  COMPILER ${CMAKE_C_COMPILER}
  INPUT ${source}
  OUTPUT_EXTENSION ll
  FLAGS -c -emit-llvm -S -fno-discard-value-names
)
compile_fixture(
  COMPILER ${CMAKE_C_COMPILER}
  INPUT ${source}
  OUTPUT_EXTENSION bc
  FLAGS -c -emit-llvm -fno-discard-value-names
)
compile_fixture(
  COMPILER ${CMAKE_C_COMPILER}
  INPUT ${source}
  OUTPUT_EXTENSION lto.bc
  FLAGS -c -flto -fno-discard-value-names
)
compile_fixture(
  COMPILER ${CMAKE_C_COMPILER}
  INPUT ${source}
  OUTPUT_EXTENSION bin
  FLAGS -fembed-bitcode -fno-discard-value-names
)