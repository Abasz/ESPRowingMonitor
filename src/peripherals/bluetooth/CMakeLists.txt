add_library(bluetooth INTERFACE)

target_sources(
  bluetooth
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/bluetooth.controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ble-services/base-metrics.service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ble-services/battery.service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ble-services/device-info.service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ble-services/extended-metrics.notifications.service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ble-services/extended-metrics.service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ble-services/ota.service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ble-services/settings.service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/callbacks/connection-manager.callbacks.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/callbacks/control-point.callbacks.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/callbacks/ota.callbacks.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/callbacks/subscription-manager.callbacks.cpp)
