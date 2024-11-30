#pragma once

#include <vector>

#include "../configuration.h"
#include "./series.h"

using std::vector;

class TSLinearSeries
{
    unsigned char maxSeriesLength;
    unsigned short maxSlopeSeriesLength = ((maxSeriesLength - 2) * (maxSeriesLength - 1)) / 2;
    bool shouldRecalculateB = true;
    bool shouldRecalculateA = true;
    Configurations::precision a = 0;
    Configurations::precision b = 0;

    Series seriesX;
    Series seriesY;
    vector<vector<Configurations::precision>> slopes;

    Configurations::precision calculateSlope(unsigned char pointOne, unsigned char pointTwo) const;

public:
    constexpr explicit TSLinearSeries(const unsigned char _maxSeriesLength = 0, const unsigned short _maxAllocationCapacity = 1'000) : maxSeriesLength(_maxSeriesLength), seriesX(_maxSeriesLength, _maxAllocationCapacity), seriesY(_maxSeriesLength, _maxAllocationCapacity)
    {
        if (_maxSeriesLength > 0)
        {
            slopes.reserve(_maxSeriesLength);
        }
    }

    Configurations::precision yAtSeriesBegin() const;
    Configurations::precision median() const;
    Configurations::precision coefficientA();
    Configurations::precision coefficientB();
    size_t size() const;

    void push(Configurations::precision pointX, Configurations::precision pointY);
    void reset();
};