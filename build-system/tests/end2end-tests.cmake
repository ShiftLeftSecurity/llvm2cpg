function(enable_end2end_tests)
  if (NOT PATH_TO_OCULAR AND NOT PATH_TO_JOERN)
    message("-- PATH_TO_OCULAR or PATH_TO_JOERN are not present. Skipping End-to-end tests.")
    add_custom_target(run-end2end-tests)
    return()
  endif()

  if (PATH_TO_OCULAR)
    set (ANALYZER_DIR ${PATH_TO_OCULAR})
    set (ANALYZER ocular.sh)
  elseif(PATH_TO_JOERN)
    set (ANALYZER_DIR ${PATH_TO_JOERN})
    set (ANALYZER joern)
  endif()

  execute_process(
    COMMAND lit --show-tests ${CMAKE_CURRENT_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE LIT_TESTS
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  set (LIT_FLAGS
    PATH_TO_LLVM=${PATH_TO_LLVM}
    SOURCE_DIR=${CMAKE_SOURCE_DIR}
    LLVM2CPG=$<TARGET_FILE:llvm2cpg>
    OCULAR_DIR=${PATH_TO_OCULAR}
    JOERN_DIR=${PATH_TO_JOERN}
    ANALYZER_DIR=${ANALYZER_DIR}
    ANALYZER=${ANALYZER_DIR}/${ANALYZER}
    CLANG=${PATH_TO_LLVM}/bin/clang
    CLANGXX=${PATH_TO_LLVM}/bin/clang++
    FILECHECK=filecheck
    TEST_DATA_DIR=${CMAKE_SOURCE_DIR}/tests/test-data
  )

  string(REPLACE "\n" ";" TEST_LIST ${LIT_TESTS})
  foreach (test_name ${TEST_LIST})
    string(REGEX MATCH "end2end tests :: (.*)" match ${test_name})
    if (match)
      set(LIT_COMMAND
        ${LIT_FLAGS}
        lit -vv --filter="${CMAKE_MATCH_1}"
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
      string(REPLACE "/" "-" test ${CMAKE_MATCH_1})
      add_custom_target(run-end2end-test-${test}
        COMMAND ${LIT_COMMAND}
        DEPENDS llvm2cpg
        )
    endif()
  endforeach ()

  set(LIT_COMMAND
    ${LIT_FLAGS}
    lit -vv -j1
    ${CMAKE_CURRENT_SOURCE_DIR}
    )
  add_custom_target(run-end2end-tests
    COMMAND ${LIT_COMMAND}
    DEPENDS llvm2cpg
    )
endfunction()