set (SOURCE ${CMAKE_CURRENT_LIST_DIR}/hello_world.c)

compile_fixture(
  COMPILER ${CMAKE_C_COMPILER}
  INPUT ${SOURCE}
  OUTPUT_EXTENSION bc
  FLAGS -c -emit-llvm
)
compile_fixture(
  COMPILER ${CMAKE_C_COMPILER}
  INPUT ${SOURCE}
  OUTPUT_EXTENSION ll
  FLAGS -c -emit-llvm
)