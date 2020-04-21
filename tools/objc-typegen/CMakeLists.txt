add_custom_command(OUTPUT Policies
  COMMENT "env LIBCLANG_PATH=${PATH_TO_LLVM}/lib python3 ${CMAKE_CURRENT_LIST_DIR}/objc-typegen.py"
  COMMAND pwd
  COMMAND env LIBCLANG_PATH=${PATH_TO_LLVM}/lib python3 ${CMAKE_CURRENT_LIST_DIR}/objc-typegen.py
  DEPENDS objc-typegen.py
  )

add_custom_target(run-objc-typegen DEPENDS Policies)