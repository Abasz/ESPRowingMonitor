find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-22 clang-tidy-21
                                  clang-tidy-20 REQUIRED)

# Fall back the lint target to C++23 when the project uses C++26 but the found
# clang-tidy is version 21 or older, determined from its versioned filename.
set(LINT_CXX_STANDARD ${CMAKE_CXX_STANDARD})
if(CMAKE_CXX_STANDARD GREATER_EQUAL 26)
  get_filename_component(clang_tidy_name "${CLANG_TIDY_EXE}" NAME)
  if(clang_tidy_name MATCHES "^clang-tidy-([0-9]+)$")
    if(CMAKE_MATCH_1 LESS 22)
      set(LINT_CXX_STANDARD 23)
      message(
        STATUS
          "clang-tidy ${CMAKE_MATCH_1} found; lint target falls back to C++23")
    endif()
  endif()
  unset(clang_tidy_name)
endif()
find_program(IWYU_PATH NAMES include-what-you-use iwyu)

# =============================================================================
# Unit Test Sources (local to this directory)
# =============================================================================

file(GLOB UNIT_TEST_MOCKS CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp)
# Using GLOB_RECURSE for test specs since they change frequently
file(GLOB_RECURSE UNIT_TEST_SPECS CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/*.spec.cpp)

set(UNIT_TEST_SOURCES ${UNIT_TEST_MOCKS} ${UNIT_TEST_SPECS})

# =============================================================================
# Helper for configuring different unit test targets
# =============================================================================
function(configure_unit_test_target target_name)
  target_sources(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/src/main.cpp)

  target_include_directories(${target_name}
                             PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

  target_link_libraries(
    ${target_name}
    PRIVATE rower_engine
            utils
            peripherals
            project_options
            project_warnings
            Catch2::Catch2WithMain
            FakeIt::FakeIt-catch)

  target_compile_definitions(${target_name} PRIVATE UNIT_TEST)
endfunction()

# ========================================================================
# Standard Unit Test Executable
# =============================================================================

add_executable(unit-test ${UNIT_TEST_SOURCES})

configure_unit_test_target(unit-test)

set_target_properties(
  unit-test
  PROPERTIES OUTPUT_NAME "test"
             SUFFIX ".out"
             UNIT_TEST_SCRIPTS_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(
  run-unit-test
  COMMAND unit-test
  DEPENDS unit-test
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Running unit tests")

# =============================================================================
# Unit Test with Lint (clang-tidy, include-what-you-use - optional -, link what
# you use)
# =============================================================================

if(NOT IWYU_PATH)
  message(WARNING "Could not find the program include-what-you-use")
endif()

add_executable(unit-test-lint EXCLUDE_FROM_ALL ${UNIT_TEST_SOURCES})

configure_unit_test_target(unit-test-lint)

set_target_properties(
  unit-test-lint
  PROPERTIES OUTPUT_NAME "test-lint"
             SUFFIX ".out"
             CXX_STANDARD ${LINT_CXX_STANDARD}
             CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
             LINT_SCRIPTS_DIR "${CMAKE_CURRENT_BINARY_DIR}"
             CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH}"
             CXX_LINK_WHAT_YOU_USE TRUE)

add_custom_target(
  run-unit-test-lint
  COMMAND unit-test-lint
  DEPENDS unit-test-lint
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Running unit tests with lint")

# =============================================================================
# Test Coverage
# =============================================================================

add_executable(test-coverage EXCLUDE_FROM_ALL ${UNIT_TEST_SOURCES})

configure_unit_test_target(test-coverage)

target_compile_options(test-coverage PRIVATE --coverage)

target_link_libraries(test-coverage PRIVATE gcov)

set_target_properties(
  test-coverage
  PROPERTIES OUTPUT_NAME "test-coverage"
             SUFFIX ".out"
             COVERAGE_SCRIPTS_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Run tests after building coverage target
add_custom_command(
  TARGET test-coverage
  POST_BUILD
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMAND $<TARGET_FILE:test-coverage>
  COMMENT "Running unit tests for coverage")
