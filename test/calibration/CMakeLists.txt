# =============================================================================
# Calibration-Specific Scripts
# =============================================================================

# run-calibration: Orchestrates calibration tests
configure_file(${_SCRIPTS_DIR}/run-calibration.sh.in
               ${CMAKE_CURRENT_BINARY_DIR}/run-calibration.configured @ONLY)
file(
  GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/run-calibration"
  INPUT ${CMAKE_CURRENT_BINARY_DIR}/run-calibration.configured
  FILE_PERMISSIONS OWNER_EXECUTE OWNER_READ)
