set (SOURCE ${CMAKE_CURRENT_LIST_DIR}/inline_example.cpp)

compile_fixture(
  COMPILER ${CMAKE_CXX_COMPILER}
  INPUT ${SOURCE}
  OUTPUT_EXTENSION ll
  FLAGS -c -emit-llvm -S -O3
)
compile_fixture(
  COMPILER ${CMAKE_CXX_COMPILER}
  INPUT ${SOURCE}
  OUTPUT_EXTENSION bc
  FLAGS -c -emit-llvm
)