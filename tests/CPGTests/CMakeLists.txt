generate_fixture_factory(FIXTURES_FACTORY_HEADER)

set(SOURCES
  ${FIXTURES_FACTORY_HEADER}
  GTestRunner.cpp

  CPGMethodTests.cpp
  BitcodeLoaderTests.cpp
  CPGTests.cpp
  )

add_executable(llvm2cpg-tests ${SOURCES})
target_compile_options(llvm2cpg-tests PRIVATE ${LLVM2CPG_CXX_FLAGS})
target_link_libraries(llvm2cpg-tests CPG google-test)
target_include_directories(llvm2cpg-tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_add_test_dependencies(llvm2cpg-tests)