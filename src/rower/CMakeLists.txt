add_library(rower_engine INTERFACE)

target_sources(
  rower_engine
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/flywheel.service.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/stroke.controller.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/stroke.service.cpp)
