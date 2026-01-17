cmake_minimum_required(VERSION 3.28)

# Set project name and version
project(
    ESP-Rowing-Monitor
    VERSION 6.2.1
    LANGUAGES CXX
)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# search for clang-tidy
find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-21 clang-tidy-20 clang-tidy-19 clang-tidy-18 REQUIRED)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.12.0
)

FetchContent_Declare(
  FakeIt
  GIT_REPOSITORY https://github.com/eranpeer/FakeIt.git
  GIT_TAG 2.5.0
)

FetchContent_MakeAvailable(Catch2)
FetchContent_MakeAvailable(FakeIt)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
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
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES_COMMON
    "${LIB_DIR}/utils/series/ols-linear-series.cpp"
    "${LIB_DIR}/utils/series/series.cpp"
    "${LIB_DIR}/utils/series/ts-quadratic-series.cpp"
    "${LIB_DIR}/utils/series/weighted-average-series.cpp"
    "${LIB_DIR}/utils/series/ts-linear-series.cpp"
    "${LIB_DIR}/utils/series/exponential-weighted-average.cpp"
    "${LIB_DIR}/utils/series/cyclic-error-filter.cpp"

    "${LIB_DIR}/rower/stroke.service.cpp"
    "${LIB_DIR}/rower/flywheel.service.cpp"
    "${LIB_DIR}/rower/stroke.controller.cpp"
)

set(SOURCES_UNIT_TEST
    ${SOURCES_COMMON}

    "${LIB_DIR}/utils/power-manager/power-manager.controller.cpp"
    "${LIB_DIR}/utils/power-manager/power-manager.service.cpp"
    "${LIB_DIR}/utils/EEPROM/EEPROM.service.cpp"
    "${LIB_DIR}/utils/ota-updater/ota-updater.service.cpp"

    "${LIB_DIR}/peripherals/peripherals.controller.cpp"
    "${LIB_DIR}/peripherals/sd-card/sd-card.service.cpp"
    "${LIB_DIR}/peripherals/led/led.service.cpp"

    "${LIB_DIR}/peripherals/bluetooth/ble-services/base-metrics.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/ble-services/extended-metrics.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/ble-services/extended-metrics.notifications.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/ble-services/battery.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/ble-services/ota.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/ble-services/settings.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/ble-services/device-info.service.cpp"
    "${LIB_DIR}/peripherals/bluetooth/callbacks/control-point.callbacks.cpp"
    "${LIB_DIR}/peripherals/bluetooth/callbacks/subscription-manager.callbacks.cpp"
    "${LIB_DIR}/peripherals/bluetooth/callbacks/ota.callbacks.cpp"
    "${LIB_DIR}/peripherals/bluetooth/callbacks/connection-manager.callbacks.cpp"
    "${LIB_DIR}/peripherals/bluetooth/bluetooth.controller.cpp"

    "${UNIT_TEST_DIR}/bluetooth/ble-services/settings.service.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/ble-services/battery.service.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/ble-services/device-info.service.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/ble-services/ota.service.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/ble-services/base-metrics.service.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/ble-services/extended-metrics.service.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/ble-services/extended-metrics.service.notifications.spec.cpp"
    
    "${UNIT_TEST_DIR}/bluetooth/bluetooth.control-point.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/bluetooth.controller.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/bluetooth.notifications.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/bluetooth.subscription-manager.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/bluetooth.ota-callback.spec.cpp"
    "${UNIT_TEST_DIR}/bluetooth/bluetooth.connection-manager.spec.cpp"

    "${UNIT_TEST_DIR}/peripherals.controller.spec.cpp"
    "${UNIT_TEST_DIR}/led.service.spec.cpp"
    "${UNIT_TEST_DIR}/EEPROM.service.spec.cpp"
    "${UNIT_TEST_DIR}/power-manager.service.spec.cpp"
    "${UNIT_TEST_DIR}/ota-updater.service.spec.cpp"
    "${UNIT_TEST_DIR}/power-manager.controller.spec.cpp"
    "${UNIT_TEST_DIR}/sd-card.service.spec.cpp"
    "${UNIT_TEST_DIR}/globals.spec.cpp"

    "${UNIT_TEST_DIR}/rower/stroke.controller.spec.cpp"
    "${UNIT_TEST_DIR}/rower/stroke.service.spec.cpp"
    "${UNIT_TEST_DIR}/rower/flywheel.service.spec.cpp"

    "${UNIT_TEST_DIR}/series/series.spec.cpp"
    "${UNIT_TEST_DIR}/series/ts-linear-series.spec.cpp"
    "${UNIT_TEST_DIR}/series/ols-linear-series.spec.cpp"
    "${UNIT_TEST_DIR}/series/weighted-average-series.spec.cpp"
    "${UNIT_TEST_DIR}/series/ts-quadratic-series.spec.cpp"
    "${UNIT_TEST_DIR}/series/exponential-weighted-average.spec.cpp"
    "${UNIT_TEST_DIR}/series/cyclic-error-filter.spec.cpp"

    "${UNIT_TEST_DIR}/include/main.cpp"
    "${UNIT_TEST_DIR}/include/Update.cpp"
    "${UNIT_TEST_DIR}/include/SdFat.cpp"
    "${UNIT_TEST_DIR}/include/led_strip.cpp"
    "${UNIT_TEST_DIR}/include/NimBLEDevice.cpp"
    "${UNIT_TEST_DIR}/include/Arduino.cpp"
    "${UNIT_TEST_DIR}/include/esp_ota_ops.cpp"
    "${UNIT_TEST_DIR}/include/ArduinoLog.cpp"
    "${UNIT_TEST_DIR}/include/globals.cpp"
)

set(SOURCES_E2E_TEST
    ${SOURCES_COMMON}

    "${E2E_TEST_DIR}/main.cpp"
    "${E2E_TEST_DIR}/test.array.cpp"
    "${E2E_TEST_DIR}/ArduinoLog.cpp"
    "${E2E_TEST_DIR}/globals.cpp"
)

set(SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake/scripts)

configure_file(
    ${SCRIPTS_DIR}/get-rower-profile.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/get-rower-profile
    @ONLY
    FILE_PERMISSIONS OWNER_EXECUTE OWNER_READ
)

# Create e2e target
add_executable(e2e-test EXCLUDE_FROM_ALL ${SOURCES_E2E_TEST})
add_custom_target(clean-e2e
    COMMAND /bin/sh -c '[ -z $(ROWER_PROFILE) ] && { ${CMAKE_COMMAND} -E cmake_echo_color --red \'"ERROR: Rower profile is missing"\'\; exit 1\;} || exit 0'
    COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "Rower profile: $(ROWER_PROFILE)"
    COMMAND "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/e2e-test.dir/clean_e2e" $(ROWER_PROFILE)
)
target_link_libraries(
    e2e-test PRIVATE 
    FakeIt::FakeIt-standalone
)

add_dependencies(e2e-test clean-e2e)

target_include_directories(e2e-test PRIVATE ${E2E_TEST_DIR})
target_compile_options(e2e-test PRIVATE ${COMPILER_FLAGS})
target_compile_definitions(e2e-test PRIVATE
    LOG_CALIBRATION
    BOARD_PROFILE="profiles/generic.board-profile.h"
    ROWER_PROFILE="profiles/$(ROWER_PROFILE).rower-profile.h"
    USE_CUSTOM_SETTINGS=$(USE_CUSTOM_SETTINGS)
)

set_target_properties(
    e2e-test
    PROPERTIES
        OUTPUT_NAME "e2e_test"
        SUFFIX ".out"
)

configure_file(
    ${SCRIPTS_DIR}/clean-e2e.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/e2e-test.dir/clean_e2e.configured
    @ONLY
)
file(
    GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/e2e-test.dir/clean_e2e
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/e2e-test.dir/clean_e2e.configured
    TARGET e2e-test
    FILE_PERMISSIONS OWNER_EXECUTE OWNER_READ
)

configure_file(
    ${SCRIPTS_DIR}/build-e2e.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/build-e2e
    @ONLY
    FILE_PERMISSIONS OWNER_EXECUTE OWNER_READ
)

add_custom_command(
    TARGET e2e-test
    POST_BUILD
    WORKING_DIRECTORY ${PROJECT_ROOT_DIR}
    COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/$(ROWER_PROFILE).environment
)

# Helper function to create unit test targets
function(add_unit_test_target target_name output_name enable_clang_tidy exclude_from_all)
    if(exclude_from_all)
        add_executable(${target_name} EXCLUDE_FROM_ALL ${SOURCES_UNIT_TEST})
    else()
        add_executable(${target_name} ${SOURCES_UNIT_TEST})
    endif()
    
    target_include_directories(${target_name} PRIVATE ${INCLUDES_UNIT_TEST})
    target_compile_options(${target_name} PRIVATE ${COMPILER_FLAGS})
    target_compile_options(${target_name} PRIVATE -Wextra -Wpedantic -pedantic -Wnon-virtual-dtor -Wcast-align -Wunused -Wshadow -Woverloaded-virtual -Wnull-dereference -Wformat=2)
    target_compile_definitions(${target_name} PRIVATE UNIT_TEST)
    target_link_libraries(
        ${target_name} PRIVATE 
        Catch2::Catch2WithMain
        FakeIt::FakeIt-catch
    )
    
    set_target_properties(${target_name} PROPERTIES
        OUTPUT_NAME "${output_name}"
        SUFFIX ".out"
    )
    
    if(enable_clang_tidy)
        set_target_properties(${target_name} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--extra-arg=-stdlib=libc++"
        )
    endif()
    
    # Create run target
    add_custom_target(run-${target_name}
        COMMAND ${target_name}
        DEPENDS ${target_name}
        WORKING_DIRECTORY ${PROJECT_ROOT_DIR}
        COMMENT "Running ${output_name}"
    )
endfunction()

# Create unit test targets
add_unit_test_target(unit-test "test" FALSE FALSE)
add_unit_test_target(unit-test-lint "test-lint" TRUE TRUE)

# Create code coverage target
add_executable(test-coverage EXCLUDE_FROM_ALL ${SOURCES_UNIT_TEST})

target_include_directories(test-coverage PRIVATE ${INCLUDES_UNIT_TEST})
target_compile_options(test-coverage PRIVATE ${COMPILER_FLAGS} --coverage)
target_compile_definitions(test-coverage PRIVATE UNIT_TEST)
target_link_libraries(
    test-coverage PRIVATE
    gcov
    Catch2::Catch2WithMain
    FakeIt::FakeIt-catch
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
    WORKING_DIRECTORY ${PROJECT_ROOT_DIR}
    COMMAND $<TARGET_FILE:test-coverage>
)

configure_file(
    ${SCRIPTS_DIR}/run-calibration.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/run-calibration.configured
    @ONLY
)
file(
    GENERATE
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/run-calibration"
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/run-calibration.configured
    FILE_PERMISSIONS OWNER_EXECUTE OWNER_READ
)
