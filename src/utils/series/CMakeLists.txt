add_library(series INTERFACE)

target_sources(
  series
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cyclic-error-filter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/exponential-weighted-average.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/ols-linear-series.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/series.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/ts-linear-series.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/ts-quadratic-series.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/weighted-average-series.cpp)
