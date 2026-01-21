add_library(led INTERFACE)

target_sources(led INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/led.service.cpp)
