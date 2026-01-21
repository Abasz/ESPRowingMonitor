add_library(ota_updater INTERFACE)

target_sources(ota_updater
               INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ota-updater.service.cpp)
