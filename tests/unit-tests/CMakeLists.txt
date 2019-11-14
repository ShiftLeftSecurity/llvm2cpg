generate_fixture_factory(FIXTURES_FACTORY_HEADER)

set(SOURCES
  ${FIXTURES_FACTORY_HEADER}
  GTestRunner.cpp

  CPGMethodTests.cpp
  BitcodeLoaderTests.cpp
  CPGTests.cpp
  )

add_executable(unit-tests ${SOURCES})
target_compile_options(unit-tests PRIVATE ${LLVM2CPG_CXX_FLAGS})
target_link_libraries(unit-tests CPG google-test)
target_include_directories(unit-tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_add_test_dependencies(unit-tests)

add_custom_target(run-unit-tests
  COMMAND $<TARGET_FILE:unit-tests>
)