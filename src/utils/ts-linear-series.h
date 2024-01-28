#pragma once

#include <vector>

#include "configuration.h"
#include "series.h"

class TSLinearSeries
{
    unsigned char maxSeriesLength = 0;
    unsigned short maxSlopeSeriesLength = 0;
    Configurations::precision a = 0;

    Series seriesX;
    Series seriesY;
    vector<vector<Configurations::precision>> slopes;

    Configurations::precision calculateSlope(unsigned char pointOne, unsigned char pointTwo) const;

public:
    explicit TSLinearSeries(unsigned char _maxSeriesLength = 0, unsigned short _maxAllocationCapacity = 1000);

    Configurations::precision median() const;
    Configurations::precision coefficientA() const;

    void push(Configurations::precision pointX, Configurations::precision pointY);
    void reset();
};