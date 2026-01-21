set(TEST_UTILITY_SCRIPTS_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(e2e)
add_subdirectory(unit)
add_subdirectory(calibration)

# =============================================================================
# Utility Scripts
# =============================================================================

# get-rower-profile: Utility used by both e2e and calibration
configure_file(
  ${_SCRIPTS_DIR}/get-rower-profile.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/get-rower-profile @ONLY FILE_PERMISSIONS
  OWNER_EXECUTE OWNER_READ)
