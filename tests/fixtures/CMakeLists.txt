add_subdirectory(basic_c_support)
add_subdirectory(basic_cxx_support)
add_subdirectory(llvm_support)
add_subdirectory(bitcode_extraction)

get_property(fixture_dependencies GLOBAL PROPERTY TEST_FIXTURES_DEPENDENCIES)
add_custom_target(generate-fixtures
  DEPENDS ${fixture_dependencies}
)