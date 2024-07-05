#pragma once

#include "./configuration.h"
#include "./series.h"

class OLSLinearSeries
{
    unsigned char maxSeriesLength = 0;
    Series seriesX;
    Series seriesXSquare;
    Series seriesY;
    Series seriesYSquare;
    Series seriesXY;

public:
    explicit OLSLinearSeries(unsigned char _maxSeriesLength = 0, unsigned short _maxAllocationCapacity = 1'000);

    Configurations::precision yAtSeriesBegin() const;
    Configurations::precision slope() const;
    Configurations::precision goodnessOfFit() const;
    size_t size() const;

    void push(Configurations::precision pointX, Configurations::precision pointY);
    void reset();
};