add_library(sdcard INTERFACE)

target_sources(sdcard INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sd-card.service.cpp)

target_include_directories(sdcard INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
