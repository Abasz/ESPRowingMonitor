add_library(eeprom INTERFACE)

target_sources(eeprom INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/EEPROM.service.cpp)
