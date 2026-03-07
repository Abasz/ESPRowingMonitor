#pragma once

#include <cstddef>
#include <vector>

#include "../configuration.h"
#include "./series.h"

using std::vector;

class TSLinearSeries
{
    unsigned char maxSeriesLength;
    unsigned short maxSlopeSeriesLength = (maxSeriesLength * (maxSeriesLength - 1)) / 2;
    unsigned short initialCapacity;
    bool shouldRecalculateB = true;
    bool shouldRecalculateA = true;
    Configurations::precision a = 0;
    Configurations::precision b = 0;

    Series seriesX;
    Series seriesY;
    vector<vector<Configurations::precision>> slopes;

    [[nodiscard]] Configurations::precision calculateSlope(unsigned char pointOne, unsigned char pointTwo) const;

public:
    constexpr explicit TSLinearSeries(
        const unsigned char _maxSeriesLength = 0,
        const unsigned short _initialCapacity = Configurations::defaultAllocationCapacity,
        const unsigned short _maxAllocationCapacity = 1'000)
        : maxSeriesLength(_maxSeriesLength),
          initialCapacity(_initialCapacity),
          seriesX(_maxSeriesLength, _initialCapacity, _maxAllocationCapacity),
          seriesY(_maxSeriesLength, _initialCapacity, _maxAllocationCapacity)
    {
        if (_maxSeriesLength > 0)
        {
            slopes.reserve(_maxSeriesLength);
        }
    }

    [[nodiscard]] Configurations::precision yAtSeriesBegin() const;
    [[nodiscard]] Configurations::precision xAtSeriesEnd() const;
    [[nodiscard]] Configurations::precision xAtSeriesBegin() const;
    [[nodiscard]] Configurations::precision median() const;
    Configurations::precision coefficientA();
    Configurations::precision coefficientB();
    [[nodiscard]] size_t size() const;

    void push(Configurations::precision pointX, Configurations::precision pointY);
    void reset();
};