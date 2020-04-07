include(${CMAKE_CURRENT_LIST_DIR}/fixtures.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/validation-tests.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/integration-tests.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/smoke-tests.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/end2end-tests.cmake)

add_custom_target(run-all-tests
  DEPENDS run-unit-tests run-smoke-tests run-integration-tests run-end2end-tests
)
