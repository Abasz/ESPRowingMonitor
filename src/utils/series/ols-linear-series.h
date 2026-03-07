#pragma once

#include <cstddef>

#include "../configuration.h"
#include "./series.h"

class OLSLinearSeries
{
    Series seriesX;
    Series seriesXSquare;
    Series seriesY;
    Series seriesYSquare;
    Series seriesXY;

public:
    constexpr explicit OLSLinearSeries(
        const unsigned char _maxSeriesLength = 0,
        const unsigned short _initialCapacity = Configurations::defaultAllocationCapacity,
        const unsigned short _maxAllocationCapacity = 1'000)
        : seriesX(_maxSeriesLength, _initialCapacity, _maxAllocationCapacity),
          seriesXSquare(_maxSeriesLength, _initialCapacity, _maxAllocationCapacity),
          seriesY(_maxSeriesLength, _initialCapacity, _maxAllocationCapacity),
          seriesYSquare(_maxSeriesLength, _initialCapacity, _maxAllocationCapacity),
          seriesXY(_maxSeriesLength, _initialCapacity, _maxAllocationCapacity)
    {
    }

    [[nodiscard]] Configurations::precision yAtSeriesBegin() const;
    [[nodiscard]] Configurations::precision xAtSeriesBegin() const;
    [[nodiscard]] Configurations::precision xAtSeriesEnd() const;
    [[nodiscard]] Configurations::precision slope() const;
    [[nodiscard]] Configurations::precision intercept() const;
    [[nodiscard]] Configurations::precision goodnessOfFit() const;
    [[nodiscard]] size_t size() const;

    void push(Configurations::precision pointX, Configurations::precision pointY);
    void reset();
};