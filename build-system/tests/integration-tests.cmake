define_property(GLOBAL
  PROPERTY INTEGRATION_TESTS
  BRIEF_DOCS "Holds a list of all integration tests"
  FULL_DOCS "See below")

function(_record_integration_test test_name)
  set_property(GLOBAL APPEND PROPERTY INTEGRATION_TESTS ${test_name})
endfunction()

function(add_integration_test test_name)
  _record_integration_test(${test_name})
  foreach(fixture_name ${ARGN})
    get_fixture_output(${fixture_name} output)
    set (bitcode_files ${bitcode_files} ${output})
    set (bitcode_dependencies ${bitcode_dependencies} build-${fixture_name}-fixture)
  endforeach()

  set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
  set (cpg ${output_dir}/cpg.bin.zip)

  file(MAKE_DIRECTORY ${output_dir})
  add_custom_command(OUTPUT ${cpg}
    COMMAND $<TARGET_FILE:cpg-proto-writer> -output-dir=${output_dir} ${bitcode_files}
    DEPENDS cpg-proto-writer ${bitcode_dependencies}
    )

  if (PATH_TO_CODEPROPERTYGRAPH)
    add_custom_target(validation-${test_name}
      COMMAND cmake -E echo "Validating ${cpg}"
      COMMAND ${PATH_TO_CODEPROPERTYGRAPH}/cpgvalidator.sh ${cpg}
      DEPENDS ${cpg}
      WORKING_DIRECTORY ${PATH_TO_CODEPROPERTYGRAPH}
      )
    set (validation_test validation-${test_name})
  endif()

  add_custom_target(integration-${test_name}
    COMMAND cmake -E echo "Running integration test: ${cpg}"
    COMMAND sbt "testOnly io.shiftleft.llvm2cpgintegration.${test_name}"
    DEPENDS ${cpg}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )
  set (integration_test integration-${test_name})
  add_custom_target(run-full-${test_name}
    DEPENDS ${validaion_test} ${integration_test}
    )
endfunction()

function(enable_integration_tests)
  get_property(tests GLOBAL PROPERTY INTEGRATION_TESTS)
  foreach(test_name ${tests})
    set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
    set (cpg ${output_dir}/cpg.bin.zip)
    set (${test_name} ${cpg})
    set (cpg_files ${cpg_files} ${cpg})
    set (test_targets ${test_targets} integration-${test_name})
  endforeach()

  set (test_path ${CMAKE_SOURCE_DIR}/tests/integration-tests/src/test/scala/io/shiftleft/llvm2cpgintegration)

  set (scala_in ${test_path}/TestCpgPaths.scala.in)
  set (scala_out ${test_path}/TestCpgPaths.scala)
  configure_file(${scala_in} ${scala_out} @ONLY)

  add_custom_target(prepare-integration-tests
    DEPENDS ${cpg_files}
    )

  add_custom_target(run-integration-tests
    DEPENDS ${test_targets}
    )
endfunction()