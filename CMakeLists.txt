cmake_minimum_required(VERSION 3.28)

project(
  ESP-Rowing-Monitor
  VERSION 7.1.0
  DESCRIPTION "ESP32-based rowing machine monitor"
  HOMEPAGE_URL "https://github.com/Abasz/ESPRowingMonitor"
  LANGUAGES CXX)

# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
  message(
    FATAL_ERROR
      "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles."
  )
endif()

# =============================================================================
# Build Configuration
# =============================================================================

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Project-Wide Constants
# =============================================================================

set(_SCRIPTS_DIR
    "${CMAKE_SOURCE_DIR}/tools/cmake/scripts"
    CACHE INTERNAL "Path to CMake script templates")

# =============================================================================
# Compiler Options Interface Libraries
# =============================================================================
# NOTE: Libraries use INTERFACE type because source files contain compile-time
# branching (UNIT_TEST, USE_CUSTOM_SETTINGS, etc.) that requires recompilation
# for different targets. See src/utils/macros.h

add_library(project_options INTERFACE)
target_compile_options(project_options INTERFACE $<$<CONFIG:Debug>:-ggdb -Og>
                                                 $<$<CONFIG:Release>:-O3> -Wall)

add_library(project_warnings INTERFACE)
target_compile_options(
  project_warnings
  INTERFACE -Wextra
            -Wpedantic
            -Wnon-virtual-dtor
            -Wcast-align
            -Wunused
            -Wshadow
            -Woverloaded-virtual
            -Wnull-dereference
            -Wformat=2)

# =============================================================================
# Dependencies
# =============================================================================

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.12.0
  FIND_PACKAGE_ARGS)

FetchContent_Declare(
  FakeIt
  GIT_REPOSITORY https://github.com/eranpeer/FakeIt.git
  GIT_TAG 2.5.0)

FetchContent_MakeAvailable(Catch2 FakeIt)

add_subdirectory(src)
add_subdirectory(test)
