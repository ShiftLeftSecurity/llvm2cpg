add_subdirectory(basic_c_support)
add_subdirectory(llvm_support)

get_property(fixture_dependencies GLOBAL PROPERTY TEST_FIXTURES_DEPENDENCIES)
add_custom_target(generate-fixtures
  DEPENDS ${fixture_dependencies}
)