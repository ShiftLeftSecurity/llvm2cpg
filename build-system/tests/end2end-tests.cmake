if (NOT PATH_TO_OCULAR)
  message("-- PATH_TO_OCULAR is not present. Skipping End-to-end tests.")
endif()

function(enable_end2end_tests)
  if (NOT PATH_TO_OCULAR)
    add_custom_target(run-end2end-tests)
    return()
  endif()

  set(LIT_COMMAND
    PATH_TO_LLVM=${PATH_TO_LLVM}
    SOURCE_DIR=${CMAKE_SOURCE_DIR}
    LLVM2CPG=$<TARGET_FILE:llvm2cpg>
    OCULAR_SH=${PATH_TO_OCULAR}/ocular.sh
    OCULAR_DIR=${PATH_TO_OCULAR}
    CLANG=${PATH_TO_LLVM}/bin/clang
    CLANGXX=${PATH_TO_LLVM}/bin/clang++
    FILECHECK=filecheck
    lit -vv
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

  add_custom_target(run-end2end-tests
    COMMAND ${LIT_COMMAND}
    DEPENDS llvm2cpg
    )
endfunction()