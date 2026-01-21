add_library(power_manager INTERFACE)

target_sources(
  power_manager
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/power-manager.controller.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/power-manager.service.cpp)
