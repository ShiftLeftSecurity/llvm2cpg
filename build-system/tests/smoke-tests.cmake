define_property(GLOBAL
  PROPERTY INTEGRATION_TESTS
  BRIEF_DOCS "Holds a list of all smoke tests"
  FULL_DOCS "No way")

function(_record_smoke_test test_name)
  set_property(GLOBAL APPEND PROPERTY SMOKE_TESTS ${test_name})
endfunction()

function(add_smoke_test test_name)
  _record_smoke_test(${test_name})
  foreach(fixture_name ${ARGN})
    get_fixture_output(${fixture_name} output)
    set (bitcode_files ${bitcode_files} ${output})
    set (bitcode_dependencies ${bitcode_dependencies} build-${fixture_name}-fixture)
  endforeach()

  set (cpg ${CMAKE_CURRENT_BINARY_DIR}/${test_name}.cpg.bin.zip)

  add_custom_command(OUTPUT ${cpg}
    COMMAND $<TARGET_FILE:cpg-proto-writer> -output-dir=${CMAKE_CURRENT_BINARY_DIR} -output-name=${test_name}.cpg.bin.zip ${bitcode_files}
    DEPENDS cpg-proto-writer ${bitcode_dependencies} ${bitcode_files}
  )
  add_custom_target(smoke-${test_name} ALL
    DEPENDS ${cpg}
  )
endfunction()

function(enable_smoke_tests)
  get_property(tests GLOBAL PROPERTY SMOKE_TESTS)
  foreach(test_name ${tests})
    set (smoke_tests ${smoke_tests} smoke-${test_name})
  endforeach()

  add_custom_target(run-smoke-tests
    COMMAND ${CMAKE_COMMAND} -E echo "Running all smoke tests"
    DEPENDS ${smoke_tests}
  )
endfunction()