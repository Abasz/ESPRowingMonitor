# =============================================================================
# E2E Test Executable
# =============================================================================

file(GLOB UNIT_TEST_MOCKS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(e2e-test EXCLUDE_FROM_ALL)

target_sources(e2e-test PRIVATE ${UNIT_TEST_MOCKS})

target_link_libraries(e2e-test PRIVATE rower_engine series project_options
                                       FakeIt::FakeIt-standalone)

target_include_directories(e2e-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(
  e2e-test
  PRIVATE LOG_CALIBRATION BOARD_PROFILE="profiles/generic.board-profile.h"
          ROWER_PROFILE="profiles/$(ROWER_PROFILE).rower-profile.h"
          USE_CUSTOM_SETTINGS=$(USE_CUSTOM_SETTINGS))

set_target_properties(
  e2e-test
  PROPERTIES OUTPUT_NAME "e2e_test"
             SUFFIX ".out"
             E2E_SCRIPTS_DIR ${CMAKE_CURRENT_BINARY_DIR})

# build-e2e: Wrapper script to build e2e-test with profile
configure_file(
  ${_SCRIPTS_DIR}/build-e2e.sh.in ${CMAKE_CURRENT_BINARY_DIR}/build-e2e @ONLY
  FILE_PERMISSIONS OWNER_EXECUTE OWNER_READ)

# =============================================================================
# Clean-E2E Target (handles profile changes)
# =============================================================================

add_custom_target(
  clean-e2e
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan
          "Rower profile: $(ROWER_PROFILE)"
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/clean_e2e" $(ROWER_PROFILE)
  COMMENT "Cleaning build for e2e tests if profile changes")

add_dependencies(e2e-test clean-e2e)

# Track profile changes via environment file
add_custom_command(
  TARGET e2e-test
  POST_BUILD
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/$(ROWER_PROFILE).environment
  COMMENT "Tracking rower profile changes for e2e tests")

# clean-e2e: Cleans build when profile changes
configure_file(${_SCRIPTS_DIR}/clean-e2e.sh.in
               ${CMAKE_CURRENT_BINARY_DIR}/clean_e2e.configured @ONLY)
file(
  GENERATE
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/clean_e2e
  INPUT ${CMAKE_CURRENT_BINARY_DIR}/clean_e2e.configured
  TARGET e2e-test FILE_PERMISSIONS OWNER_EXECUTE OWNER_READ)
