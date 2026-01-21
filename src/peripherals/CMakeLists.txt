add_subdirectory(led)
add_subdirectory(sd-card)
add_subdirectory(bluetooth)

add_library(peripherals INTERFACE)

target_sources(peripherals
               INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/peripherals.controller.cpp)

target_link_libraries(peripherals INTERFACE led sdcard bluetooth)
