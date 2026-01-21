add_subdirectory(series)
add_subdirectory(power-manager)
add_subdirectory(EEPROM)
add_subdirectory(ota-updater)

add_library(utils INTERFACE)

target_link_libraries(utils INTERFACE series power_manager eeprom ota_updater)
