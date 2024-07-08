#pragma once

#include <vector>

#include "./configuration.h"
#include "./series.h"

using std::vector;

class TSLinearSeries
{
    unsigned char maxSeriesLength = 0;
    unsigned short maxSlopeSeriesLength = 0;
    bool shouldRecalculateB = true;
    bool shouldRecalculateA = true;
    Configurations::precision a = 0;
    Configurations::precision b = 0;

    Series seriesX;
    Series seriesY;
    vector<vector<Configurations::precision>> slopes;

    Configurations::precision calculateSlope(unsigned char pointOne, unsigned char pointTwo) const;

public:
    explicit TSLinearSeries(unsigned char _maxSeriesLength = 0, unsigned short _maxAllocationCapacity = 1'000);

    Configurations::precision yAtSeriesBegin() const;
    Configurations::precision median() const;
    Configurations::precision coefficientA();
    Configurations::precision coefficientB();
    size_t size() const;

    void push(Configurations::precision pointX, Configurations::precision pointY);
    void reset();
};