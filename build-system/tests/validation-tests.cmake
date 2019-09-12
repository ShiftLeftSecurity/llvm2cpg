if (NOT PATH_TO_CODEPROPERTYGRAPH)
  message("-- PATH_TO_CODEPROPERTYGRAPH is not present. Skipping validation tests.")
endif()

function(validate_cpg fixture_name)
  if (NOT PATH_TO_CODEPROPERTYGRAPH)
    return()
  endif()

  get_fixture_output(${fixture_name} bitcode)

  set (output_dir ${CMAKE_CURRENT_BINARY_DIR}/${fixture_name})
  file(MAKE_DIRECTORY ${output_dir})

  set (cpg ${output_dir}/cpg.bin.zip)
  add_custom_command(OUTPUT ${cpg}
    COMMAND $<TARGET_FILE:cpg-proto-writer> -output-dir=${output_dir} ${bitcode}
    DEPENDS ${bitcode} build-${fixture_name}-fixture cpg-proto-writer
    )

  add_custom_target(validate-${fixture_name}
    COMMAND cmake -E echo "Validating ${cpg}"
    COMMAND ${PATH_TO_CODEPROPERTYGRAPH}/cpgvalidator.sh ${cpg}
    DEPENDS ${cpg}
    WORKING_DIRECTORY ${PATH_TO_CODEPROPERTYGRAPH}
    )
endfunction()