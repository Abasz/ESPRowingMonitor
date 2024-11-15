cmake_minimum_required(VERSION 3.22)

# Set project name and version
project(
    ESP-Rowing-Monitor
    VERSION 5.3
    LANGUAGES CXX
)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMPILER_FLAGS -ggdb -Og -Wall)

set(LIB_DIR src)
set(TEST_DIR test)
set(E2E_TEST_DIR ${TEST_DIR}/e2e)
set(UNIT_TEST_DIR ${TEST_DIR}/unit)
set(INCLUDES_UNIT_TEST
    ${UNIT_TEST_DIR}/include/
)

# Source files for e2e tests and unit tests
set(SOURCES_UNIT_TEST
    "${LIB_DIR}/utils/power-manager/power-manager.controller.cpp"
    "${LIB_DIR}/utils/power-manager/power-manager.service.cpp"
    "${LIB_DIR}/utils/EEPROM/EEPROM.service.cpp"
    "${LIB_DIR}/utils/ota-updater/ota-updater.service.cpp"

    "${LIB_DIR}/utils/series/ols-linear-series.cpp"
    "${LIB_DIR}/utils/series/series.cpp"
    "${LIB_DIR}/utils/series/ts-quadratic-series.cpp"
    "${LIB_DIR}/utils/series/weighted-average-series.cpp"
    "${LIB_DIR}/utils/series/ts-linear-series.cpp"

    "${LIB_DIR}/rower/stroke.service.cpp"
    "${LIB_DIR}/rower/flywheel.service.cpp"
    "${LIB_DIR}/rower/stroke.controller.cpp"

    "${LIB_DIR}/peripherals/peripherals.controller.cpp"
    "${LIB_DIR}/peripherals/sd-card/sd-card.service.cpp"

    "${LIB_DIR}/peripherals/bluetooth/ble-services/base-metrics.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/ble-services/extended-metrics.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/ble-services/battery.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/ble-services/ota.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/ble-services/settings.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/ble-services/device-info.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/callbacks/control-point.callbacks.cpp"
    "${LIB_DIR}/peripherals/bluetooth/callbacks/chunked-notify.callbacks.cpp"
    "${LIB_DIR}/peripherals/bluetooth/callbacks/ota.callbacks.cpp"
    "${LIB_DIR}/peripherals/bluetooth/callbacks/server.callbacks.cpp"
    "${LIB_DIR}/peripherals/bluetooth/bluetooth.controller.cpp"
    "${LIB_DIR}/peripherals/bluetooth/bluetooth.notifications.cpp"

    "${UNIT_TEST_DIR}/bluetooth/ble-services/settings.service.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/ble-services/battery.service.spec.cpp"

    "${UNIT_TEST_DIR}/bluetooth/bluetooth.control-point.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/bluetooth.controller.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/bluetooth.notifications.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/bluetooth.chunked-notify-metrics.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/bluetooth.ota-callback.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/bluetooth.server-callbacks.spec.cpp"

    "${UNIT_TEST_DIR}/peripherals.controller.spec.cpp"
    "${UNIT_TEST_DIR}/EEPROM.service.spec.cpp"
    "${UNIT_TEST_DIR}/power-manager.service.spec.cpp"
    "${UNIT_TEST_DIR}/ota-updater.service.spec.cpp"
    "${UNIT_TEST_DIR}/power-manager.controller.spec.cpp"
    "${UNIT_TEST_DIR}/sd-card.service.spec.cpp"

    "${UNIT_TEST_DIR}/rower/stroke.controller.spec.cpp"
    "${UNIT_TEST_DIR}/rower/stroke.service.spec.cpp"
    "${UNIT_TEST_DIR}/rower/flywheel.service.spec.cpp"

    "${UNIT_TEST_DIR}/series/series.spec.cpp"
    "${UNIT_TEST_DIR}/series/ts-linear-series.spec.cpp"
    "${UNIT_TEST_DIR}/series/ols-linear-series.spec.cpp"
    "${UNIT_TEST_DIR}/series/weighted-average-series.spec.cpp"
    "${UNIT_TEST_DIR}/series/ts-quadratic-series.spec.cpp"

    "${UNIT_TEST_DIR}/include/main.cpp"
    "${UNIT_TEST_DIR}/include/Update.cpp"
    "${UNIT_TEST_DIR}/include/FastLED.cpp"
    "${UNIT_TEST_DIR}/include/SdFat.cpp"
    "${UNIT_TEST_DIR}/include/NimBLEDevice.cpp"
    "${UNIT_TEST_DIR}/include/Arduino.cpp"
    "${UNIT_TEST_DIR}/include/esp_ota_ops.cpp"
    "${UNIT_TEST_DIR}/include/ArduinoLog.cpp"
    "${UNIT_TEST_DIR}/include/globals.cpp"
    "${UNIT_TEST_DIR}/include/catch_amalgamated.cpp"
)

set(SOURCES_E2E_TEST
    "${LIB_DIR}/utils/series/ols-linear-series.cpp"
    "${LIB_DIR}/utils/series/series.cpp"
    "${LIB_DIR}/utils/series/ts-quadratic-series.cpp"
    "${LIB_DIR}/utils/series/weighted-average-series.cpp"
    "${LIB_DIR}/utils/series/ts-linear-series.cpp"

    "${LIB_DIR}/rower/stroke.service.cpp"
    "${LIB_DIR}/rower/flywheel.service.cpp"
    "${LIB_DIR}/rower/stroke.controller.cpp"

    "${E2E_TEST_DIR}/main.cpp"
    "${E2E_TEST_DIR}/test.array.cpp"
    "${E2E_TEST_DIR}/ArduinoLog.cpp"
    "${E2E_TEST_DIR}/globals.cpp"
)

file(
    GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/get-rower-profile
    CONTENT
"#!/bin/bash
set -o errexit
set -o nounset
set -o pipefail

targetEnvironment=\"\${1:-}\"

rowerProfile=$(awk -v targetEnvironment=\"\${targetEnvironment}\" '
    /^\\[env:/ { in_section = 0 }                                       # Reset flag when entering a new section
    \$0 ~ \"\\\\[env:\" targetEnvironment \"\\\\]\" { in_section = 1 }  # Set flag when the target section is found
    in_section && /-D ROWER_PROFILE/ {
        match(\$0, /\\/([^\\/\\.]+)\\./, matches)                       # Extract text between slash and dot
        print matches[1]
        exit                                                            # Exit after finding the desired line
    }
' \"${CMAKE_CURRENT_SOURCE_DIR}/platformio.ini\")

echo \"\${rowerProfile:-generic}\"
"
    FILE_PERMISSIONS OWNER_EXECUTE
)

# Create e2e target
add_executable(e2e-test EXCLUDE_FROM_ALL ${SOURCES_E2E_TEST})
add_custom_target(clean-e2e
    COMMAND /bin/sh -c \'[ -z $(ROWER_PROFILE) ] && { ${CMAKE_COMMAND} -E cmake_echo_color --red \"ERROR: Rower profile is missing\"$<SEMICOLON> exit 1$<SEMICOLON>} || exit 0'
    COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan \"Rower profile: $(ROWER_PROFILE)\"
    COMMAND "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/e2e-test.dir/clean_e2e" $(ROWER_PROFILE)
)

add_dependencies(e2e-test clean-e2e)

target_include_directories(e2e-test PRIVATE ${E2E_TEST_DIR})
target_compile_options(e2e-test PRIVATE ${COMPILER_FLAGS})
target_compile_definitions(e2e-test PRIVATE
    LOG_CALIBRATION
    USE_CUSTOM_SETTINGS
    [[BOARD_PROFILE="profiles/generic.board-profile.h"]]
    [[ROWER_PROFILE="profiles/$(ROWER_PROFILE).rower-profile.h"]]
)

set_target_properties(
    e2e-test
    PROPERTIES
        OUTPUT_NAME "e2e_test"
        SUFFIX ".out"
)

set(TARGET_OBJECTS "$<JOIN:$<TARGET_OBJECTS:e2e-test>,' '>")
set(TARGET_OUTPUT "$<SHELL_PATH:$<TARGET_FILE:e2e-test>>")
file(
    GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/e2e-test.dir/clean_e2e
    CONTENT
"#!/bin/bash
set -o errexit
set -o nounset
set -o pipefail

rowerProfile=\"$1\"

if [[ ! -f \"${CMAKE_CURRENT_BINARY_DIR}/\${rowerProfile\}.environment\" ]]; then
    ${CMAKE_COMMAND} -E cmake_echo_color \"[  0%] New rower profile, cleaning files...\"
    ${CMAKE_COMMAND} -E rm -f '${TARGET_OUTPUT}' '${TARGET_OBJECTS}' \"${CMAKE_CURRENT_BINARY_DIR}/\"*.environment
fi
"
    TARGET e2e-test
    FILE_PERMISSIONS OWNER_EXECUTE
)

file(
    GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/build-e2e
    CONTENT
"#!/bin/bash
set -o errexit
set -o nounset
set -o pipefail

targetEnvironment=\"\${1:-unknown}\"

${CMAKE_COMMAND} -E cmake_echo_color --cyan \"Target environment: \${targetEnvironment}\"

rowerProfile=$(\"${CMAKE_CURRENT_BINARY_DIR}/get-rower-profile\" \"\${targetEnvironment}\")

${CMAKE_COMMAND} --build \"${CMAKE_CURRENT_BINARY_DIR}\" --target e2e-test --parallel $(getconf _NPROCESSORS_ONLN) -- ROWER_PROFILE=\${rowerProfile}
"
    TARGET e2e-test
    FILE_PERMISSIONS OWNER_EXECUTE
)

add_custom_command(
    TARGET e2e-test
    POST_BUILD
    WORKING_DIRECTORY ../.
    COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/$(ROWER_PROFILE).environment
)

# Create unit test target
add_executable(unit-test ${SOURCES_UNIT_TEST})

target_include_directories(unit-test PRIVATE ${INCLUDES_UNIT_TEST})
target_compile_options(unit-test PRIVATE ${COMPILER_FLAGS})
target_compile_definitions(unit-test PRIVATE UNIT_TEST)
set_target_properties(
    unit-test
    PROPERTIES
        OUTPUT_NAME "test"
        SUFFIX ".out"
)

add_custom_target(run-unit-test
    COMMAND unit-test
    DEPENDS unit-test
    WORKING_DIRECTORY ../.
    COMMENT "Running unit tests"
)

# Create code coverage target
add_executable(test-coverage EXCLUDE_FROM_ALL ${SOURCES_UNIT_TEST})

target_include_directories(test-coverage PRIVATE ${INCLUDES_UNIT_TEST})
target_compile_options(test-coverage PRIVATE ${COMPILER_FLAGS_UNIT_TEST} --coverage)
target_compile_definitions(test-coverage PRIVATE UNIT_TEST)
target_link_libraries(
    test-coverage
    gcov
)
set_target_properties(
    test-coverage
    PROPERTIES
        OUTPUT_NAME "test"
        SUFFIX ".out"
)

add_custom_command(
    TARGET test-coverage
    POST_BUILD
    WORKING_DIRECTORY ../.
    COMMAND $<TARGET_FILE:test-coverage>
)

# Create calibration target
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/run-calibration"
  CONTENT 
"#!/bin/bash

set -o errexit
set -o nounset
set -o pipefail

RED='\\033[31m'
GREEN='\\033[32m'
BLUE='\\033[34m'
UNDERLINE='\\033[4m'
RESET='\\033[0m'

targetEnvironment=\"\${1:-unknown}\"

\"${CMAKE_CURRENT_BINARY_DIR}/build-e2e\" \"\${targetEnvironment}\"

rowerProfile=\$(\"${CMAKE_CURRENT_BINARY_DIR}/get-rower-profile\" \"\${targetEnvironment}\")

calibrationDir=\"${CMAKE_SOURCE_DIR}/test/calibration/\${rowerProfile}\"
if [[ ! -d \${calibrationDir} ]]; then
    echo -e \"\${RED}No calibration for rower profile: \\\"\${rowerProfile}\\\"\${RESET}\"
    exit 1
fi

echo -e \"\${BLUE}\${UNDERLINE}Current calibration directory:\${RESET} \${calibrationDir}\"

calibrationOutputDir=\"\${calibrationDir}/output\"
mkdir -p \"\${calibrationOutputDir}\"

exitCode=0
for file in \"\${calibrationDir}\"/*test.txt; do
  echo -e \"file: \$(basename \"\${file}\")\"

  \"$<TARGET_FILE:e2e-test>\" \"\${file}\" > \"\${calibrationOutputDir}/\$(basename \"\${file}\")-output.txt\"

  actualStrokeCount=\$(grep -ic \"power\" \"\${calibrationOutputDir}/\$(basename \"\${file}\")-output.txt\")

  expectedStrokeCount=\$(echo \"\$(basename \"\${file}\")\" | cut -d'-' -f3)
  if [[ \"\${expectedStrokeCount}\" == \"\${actualStrokeCount}\" ]]; then
    passed=\"PASSED\"
    color=\"\${GREEN}\"
  else
    passed=\"FAILED\"
    color=\"\${RED}\"
    exitCode=1
  fi

  echo -e \"number of strokes: \${actualStrokeCount} \${color}\${passed}\${RESET}\"
done
exit \${exitCode}
"
  FILE_PERMISSIONS OWNER_EXECUTE
)
