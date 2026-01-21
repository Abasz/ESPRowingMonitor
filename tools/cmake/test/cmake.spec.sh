#!/bin/bash
# =============================================================================
# CMake Build Verification Script
# Validates all targets, scripts, and functionality after refactoring
# =============================================================================

set -o pipefail
set -o nounset

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly YELLOW=$'\033[33m'
readonly BLUE=$'\033[34m'
readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

# Paths - use launch location (current working directory)
PROJECT_ROOT="${PROJECT_ROOT:-$PWD}"
readonly BUILD_DIR="${PROJECT_ROOT}/build"

# Debug mode (set DEBUG=1 to enable)
DEBUG="${DEBUG:-0}"

# Test state
FAILURES=()
TESTS_RUN=0
TESTS_PASSED=0

# =============================================================================
# Helper Functions
# =============================================================================

log_header() {
    printf '\n%s%s=== %s ===%s\n' "$BOLD" "$BLUE" "$1" "$RESET"
}

log_pass() {
    printf ' ... %s[PASS]%s\n' "$GREEN" "$RESET"
    ((TESTS_RUN++))
    ((TESTS_PASSED++))
}

log_fail() {
    printf ' ... %s[FAIL]%s\n' "$RED" "$RESET"
    ((TESTS_RUN++))
    FAILURES+=("$1")
}

log_skip() {
    printf '%s[SKIP]%s %s\n' "$YELLOW" "$RESET" "$1"
}

log_test_start() {
    printf '%s' "$1"
}

run_test_common() {
    local -r verbose="$1"
    local -r name="$2"
    shift 2
    
    log_test_start "$name"
    
    local output
    if output=$("$@" 2>&1); then
        log_pass
        return 0
    else
        log_fail "$name"
        if [[ "$verbose" == true ]]; then
            printf '%s%sOutput:%s\n' "$YELLOW" "$BOLD" "$RESET"
            printf '%s\n' "$output" | head -20
        fi
        return 1
    fi
}

run_test() {
    run_test_common false "$@"
}

run_test_verbose() {
    run_test_common true "$@"
}

file_exists_executable() {
    #shellcheck disable=SC2317 # This function is called implicitly
    local -r file="$1"
    #shellcheck disable=SC2317 # This function is called implicitly
    [[ -x "$file" ]]
}

file_contains() {
    local -r file="$1"
    local -r pattern="$2"
    grep -q -- "$pattern" "$file" 2>/dev/null
}

cleanup_build() {
    rm -rf "${BUILD_DIR}"
}

setup_build() {
    cleanup_build
    mkdir -p "${BUILD_DIR}"
    (
        cd "${BUILD_DIR}" || exit 1
        cmake ..
    ) > /dev/null 2>&1
}

# Monitor build output until pattern appears N times, then terminate
# Usage: run_build_until_pattern "test_name" "pattern" count max_seconds  "build_command"
run_build_until_pattern() {
    local -r test_name="$1"
    local -r pattern="$2"
    local -r target_count="${3:-3}"
    local -r max_timeout="${4:-60}"
    shift 4
    
    # Skip separator if present
    [[ "$1" == "--" ]] && shift
    
    kill_build_group() {
        local -r pid="$1"
        kill -- -"$pid" 2>/dev/null || true
        sleep 0.5
        kill -9 -"$pid" 2>/dev/null || true
        wait "$pid" 2>/dev/null || true
    }
    
    cleanup(){
        trap - INT TERM
        
        echo -e "${build_pid} ${TEMP_FILE}"
        if [[ -n "${build_pid-}" ]]; then
            kill_build_group "$build_pid"
            build_pid=
        fi
        
        if [[ -n "${TEMP_FILE-}" && -f "$TEMP_FILE" ]]; then
            rm -f "$TEMP_FILE"
            TEMP_FILE=
        fi
    }
    
    reset_lines() {
        printf '\r\033[K'  # Clear timer line
        if [[ "$DEBUG" == "0" ]]; then
            printf '\033[1A'
            printf '\r\033[K'  # Clear test name line
            log_test_start "$test_name"
        fi
    }
    
    log_test_start "$test_name"
    printf '\n'
    
    [[ "$DEBUG" == "1" ]] && printf '%s[DEBUG] Running: %s%s\n' "$YELLOW" "$*" "$RESET"
    [[ "$DEBUG" == "1" ]] && printf '%s[DEBUG] Looking for pattern "%s" (need %s matches)%s\n' "$YELLOW" "$pattern" "$target_count" "$RESET"
    
    TEMP_FILE=$(mktemp) || {
        printf '%sError: mktemp failed%s\n' "$RED" "$RESET" >&2
        exit 1
    }
    # trap cleanup EXIT  # Cleanup on all exits (including Ctrl-C)
    
    trap 'cleanup; exit 1' INT
    trap 'cleanup; exit 1' TERM
    
    # Start build in new session/process group using setsid
    setsid "$@" > "$TEMP_FILE" 2>&1 &
    build_pid=$!
    
    [[ "$DEBUG" == "1" ]] && printf '%s[DEBUG] Build PID: %s (session leader)%s\n' "$YELLOW" "$build_pid" "$RESET"
    
    local match_count=0
    local elapsed=0
    sleep 5
    while kill -0 "$build_pid" 2>/dev/null && [[ $elapsed -lt $max_timeout ]]; do
        # Count pattern matches in accumulated output (grep -c always outputs a number)
        if [[ -f "$TEMP_FILE" ]]; then
            match_count=$(grep -c -- "$pattern" "$TEMP_FILE" 2>/dev/null || true)
            # Handle case where grep finds no matches (outputs empty or just newline)
            match_count=${match_count:-0}
        fi
        
        printf '\r   Elapsed: %ss | Matches: %s/%s' "$elapsed" "$match_count" "$target_count"
        
        if [[ $match_count -ge $target_count ]]; then
            cleanup
            reset_lines
            log_pass
            return 0
        fi
        
        sleep 1
        
        ((elapsed++))
    done
    
    # Check why loop exited: timeout or natural completion
    local build_exit_code=0
    if ! kill -0 "$build_pid" 2>/dev/null; then
        
        # Build finished naturally
        wait "$build_pid" 2>/dev/null || true
        build_exit_code=$?
        reset_lines
        [[ "$DEBUG" == "1" ]] && printf '%s[DEBUG] Build completed (exit: %s, matches: %s)%s\n' "$YELLOW" "$build_exit_code" "$match_count" "$RESET"
        
        # Success if we got enough matches OR build succeeded (handles cached builds)
        if [[ $match_count -ge $target_count ]] || [[ $build_exit_code -eq 0 ]]; then
            cleanup
            reset_lines
            log_pass
            return 0
        fi
    else
        reset_lines
        
        [[ "$DEBUG" == "1" ]] && printf '%s[DEBUG] Timeout reached, killing process group...%s\n' "$YELLOW" "$RESET"
        kill_build_group "$build_pid"
    fi
    
    reset_lines
    log_fail "$test_name"
    
    if ! kill -0 "$build_pid" 2>/dev/null && [[ $build_exit_code -ne 0 ]]; then
        printf '%s(Build failed with exit code %s)%s\n' "$YELLOW" "$build_exit_code" "$RESET"
    else
        printf '%s(Build timed out after %s seconds)%s\n' "$YELLOW" "$max_timeout" "$RESET"
    fi
    
    printf '%s%sOutput:%s\n' "$YELLOW" "$BOLD" "$RESET"
    tail -20 "$TEMP_FILE"
    
    cleanup
    return 1
}

# =============================================================================
# Test Groups
# =============================================================================

test_cmake_configuration() {
    log_header "CMake Configuration & Generation"
    
    run_test_verbose "CMake should configure successfully" \
    bash -c "rm -rf '${BUILD_DIR}' && mkdir -p '${BUILD_DIR}' && cd '${BUILD_DIR}' && cmake .."
    
    run_test "Build system should expose unit-test target" \
    bash -c "cmake --build '${BUILD_DIR}' --target help 2>/dev/null | grep -q unit-test"
    
    run_test "Build system should expose e2e-test target" \
    bash -c "cmake --build '${BUILD_DIR}' --target help 2>/dev/null | grep -q e2e-test"
    
    run_test "Build system should expose test-coverage target" \
    bash -c "cmake --build '${BUILD_DIR}' --target help 2>/dev/null | grep -q test-coverage"
}

test_generated_scripts() {
    log_header "Generated Scripts"
    
    local -r scripts=(
        "${BUILD_DIR}/test/get-rower-profile"
        "${BUILD_DIR}/test/e2e/build-e2e"
        "${BUILD_DIR}/test/e2e/clean_e2e"
        "${BUILD_DIR}/test/calibration/run-calibration"
    )
    
    for script in "${scripts[@]}"; do
        local name
        name=$(basename "$script")
        run_test "Script '$name' should exist and be executable" \
        file_exists_executable "$script"
    done
}

test_generator_expressions() {
    log_header "Generator Expressions"
    
    local -r run_cal="${BUILD_DIR}/test/calibration/run-calibration"
    
    log_test_start "TARGET_FILE should be expanded in run-calibration"
    if file_contains "$run_cal" 'e2e_test.out' && ! file_contains "$run_cal" '$<TARGET_FILE'; then
        log_pass
    else
        log_fail "TARGET_FILE should be expanded in run-calibration"
    fi
    
    log_test_start "TARGET_PROPERTY should be expanded in run-calibration"
    if file_contains "$run_cal" '/test/e2e/build-e2e' && ! file_contains "$run_cal" '$<TARGET_PROPERTY'; then
        log_pass
    else
        log_fail "TARGET_PROPERTY should be expanded in run-calibration"
    fi
}

test_build_targets() {
    log_header "Build Targets"
    
    setup_build
    
    run_build_until_pattern \
    "unit-test target should build successfully" \
    '\.spec\.cpp' 3 110 -- \
    cmake --build "${BUILD_DIR}" --target unit-test --parallel 4
    
    run_test_verbose "run-unit-test target should execute the unit test file" \
    bash -c "cmake --build '${BUILD_DIR}' --target run-unit-test --parallel 4 2>&1"
    
    run_test_verbose "test-coverage target should build successfully" \
    bash -c "cmake --build '${BUILD_DIR}' --target test-coverage --parallel 4"
    
    log_test_start "test-coverage target should run successfully"
    local gcda_count
    gcda_count=$(find "${BUILD_DIR}" -name "*.gcda" 2>/dev/null | wc -l)
    if [[ "$gcda_count" -gt 0 ]]; then
        printf ' (%s .gcda files)' "$gcda_count"
        log_pass
    else
        log_fail "Coverage data should be generated (no .gcda files found)"
    fi
    
    run_test_verbose "e2e-test target should build with set rower profile" \
    bash -c "cmake --build '${BUILD_DIR}' --target e2e-test --parallel 4 -- ROWER_PROFILE=oldDanube USE_CUSTOM_SETTINGS=false"
    
    run_build_until_pattern \
    "unit-test-lint target should compile spec files with clang-tidy" \
    '\.spec\.cpp' 3 110 -- \
    bash -c "cmake --build '${BUILD_DIR}' --target unit-test-lint --parallel 4 2>&1"
}

test_get_rower_profile() {
    log_header "get-rower-profile Script"
    
    local -r script="${BUILD_DIR}/test/get-rower-profile"
    local result
    
    log_test_start "get-rower-profile should return 'oldDanube' for 'oldDanube-firebeetle2-s3'"
    if result=$("$script" "oldDanube-firebeetle2-s3" 2>&1); then
        if [[ "$result" == "oldDanube" ]]; then
            log_pass
        else
            log_fail "get-rower-profile should return 'oldDanube' for 'oldDanube-firebeetle2-s3' (got: $result)"
        fi
    else
        log_fail "get-rower-profile should return 'oldDanube' for 'oldDanube-firebeetle2-s3'"
    fi
    
    log_test_start "get-rower-profile should return 'custom' for 'custom'"
    if result=$("$script" "custom" 2>&1); then
        if [[ "$result" == "custom" ]]; then
            log_pass
        else
            log_fail "get-rower-profile should return 'custom' for 'custom' (got: $result)"
        fi
    else
        log_fail "get-rower-profile should return 'custom' for 'custom'"
    fi
    
    log_test_start "get-rower-profile should return 'genericAir' for unknown profile"
    if result=$("$script" "unknownProfile-someBoard" 2>&1); then
        if [[ "$result" == "genericAir" ]]; then
            log_pass
        else
            log_fail "get-rower-profile should return 'genericAir' for unknown profile (got: $result)"
        fi
    else
        log_fail "get-rower-profile should return 'genericAir' for unknown profile"
    fi
}

test_e2e_profile_switching() {
    log_header "E2E Profile Switching"
    
    local -r e2e_exe="${BUILD_DIR}/test/e2e/e2e_test.out"
    local -r old_env_file="${BUILD_DIR}/test/e2e/oldDanube.environment"
    local -r new_env_file="${BUILD_DIR}/test/e2e/kettlerStroker.environment"
    
    run_test_verbose "e2e-test should build with oldDanube profile" \
    bash -c "cmake --build '${BUILD_DIR}' --target e2e-test --parallel 4 -- ROWER_PROFILE=oldDanube USE_CUSTOM_SETTINGS=false"
    
    log_test_start "Initial profile environment file should exist"
    if [[ -f "$old_env_file" ]]; then
        log_pass
    else
        log_fail "Initial profile environment file should exist (expected: $old_env_file)"
    fi
    
    local modificationTimeInitial
    modificationTimeInitial=$(stat -c %Y "$e2e_exe" 2>/dev/null || printf '0')
    
    sleep 1
    run_test "e2e-test should not recompile with same profile" \
    bash -c "cmake --build '${BUILD_DIR}' --target e2e-test --parallel 4 -- ROWER_PROFILE=oldDanube USE_CUSTOM_SETTINGS=false"
    
    local modificationTimeSecond
    modificationTimeSecond=$(stat -c %Y "$e2e_exe" 2>/dev/null || printf '0')
    
    log_test_start "Same profile should not trigger recompilation"
    if [[ "$modificationTimeInitial" == "$modificationTimeSecond" ]]; then
        log_pass
    else
        log_fail "Same profile should not trigger recompilation (file was modified)"
    fi
    
    run_test_verbose "e2e-test should build with kettlerStroker profile" \
    bash -c "cmake --build '${BUILD_DIR}' --target e2e-test --parallel 4 -- ROWER_PROFILE=kettlerStroker USE_CUSTOM_SETTINGS=false"
    
    local modificationTimeThird
    modificationTimeThird=$(stat -c %Y "$e2e_exe" 2>/dev/null || printf '0')
    
    log_test_start "Different profile should trigger recompilation"
    if [[ "$modificationTimeSecond" != "$modificationTimeThird" ]]; then
        log_pass
    else
        log_fail "Different profile should trigger recompilation (file was NOT modified)"
    fi
    
    log_test_start "Environment file should switch to new profile"
    if [[ -f "$new_env_file" ]] && [[ ! -f "$old_env_file" ]]; then
        log_pass
    else
        if [[ ! -f "$new_env_file" ]]; then
            log_fail "Environment file should switch to new profile (new file not created: $new_env_file)"
            elif [[ -f "$old_env_file" ]]; then
            log_fail "Environment file should switch to new profile (old file still exists: $old_env_file)"
        fi
    fi
}

test_calibration() {
    log_header "Calibration Tests"
    
    setup_build
    
    local -r run_cal="${BUILD_DIR}/test/calibration/run-calibration"
    
    if [[ -d "${PROJECT_ROOT}/test/calibration/oldDanube" ]]; then
        log_test_start "run-calibration script should execute for oldDanube"
        local output
        if output=$("$run_cal" "oldDanube-firebeetle2-s3" 2>&1); then
            log_pass
        else
            log_fail "run-calibration script should execute for oldDanube"
            printf '%s\n' "$output" | tail -10
        fi
    else
        log_skip "No calibration data for oldDanube"
    fi
}

# =============================================================================
# Main
# =============================================================================

main() {
    printf '%s%s\n' "$BOLD" "$BLUE"
    printf '==============================================\n'
    printf '  CMake Build Verification Script\n'
    printf '==============================================\n'
    printf '%s\n' "$RESET"
    
    cd "$PROJECT_ROOT" || exit 1
    
    test_cmake_configuration
    test_generated_scripts
    test_generator_expressions
    test_build_targets
    test_get_rower_profile
    test_e2e_profile_switching
    test_calibration
    
    log_header "Summary"
    printf 'Tests run:    %s\n' "$TESTS_RUN"
    printf 'Tests passed: %s%s%s\n' "$GREEN" "$TESTS_PASSED" "$RESET"
    printf 'Tests failed: %s%s%s\n' "$RED" "${#FAILURES[@]}" "$RESET"
    
    if [[ ${#FAILURES[@]} -gt 0 ]]; then
        printf '\n%s%sFailed tests:%s\n' "$RED" "$BOLD" "$RESET"
        for failure in "${FAILURES[@]}"; do
            printf '  %s✗%s %s\n' "$RED" "$RESET" "$failure"
        done
        exit 1
    else
        printf '\n%s%sAll tests passed!%s\n' "$GREEN" "$BOLD" "$RESET"
        exit 0
    fi
}

main "$@"
