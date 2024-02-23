#pragma once

#include "configuration.h"
#include "series.h"

class OLSLinearSeries
{
    unsigned char maxSeriesLength = 0;
    Series sumX;
    Series sumXSquare;
    Series sumY;
    Series sumYSquare;
    Series sumXY;

public:
    explicit OLSLinearSeries(unsigned char _maxSeriesLength = 0, unsigned short _maxAllocationCapacity = 1'000);

    Configurations::precision yAtSeriesBegin() const;
    Configurations::precision slope() const;
    Configurations::precision goodnessOfFit() const;
    size_t size() const;

    void push(Configurations::precision pointX, Configurations::precision pointY);
    void reset();
};