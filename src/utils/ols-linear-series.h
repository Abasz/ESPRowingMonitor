#pragma once

#include "series.h"

using std::vector;

class OLSLinearSeries
{
    // TODO: convert this to a template to take advantage of generics as double is not always needed; so here the Series type would be the same as the TSLInear
    unsigned char maxSeriesLength = 0;
    Series sumX;
    Series sumXSquare;
    Series sumY;
    Series sumYSquare;
    Series sumXY;

public:
    explicit OLSLinearSeries(unsigned char _maxSeriesLength = 0);

    void resetData();
    // TODO: convert this to a template to take advantage of generics as double is not always needed; so here the Series type would be the same as the linear
    void push(unsigned long long x, unsigned long long y);
    unsigned long long yAtSeriesBegin() const;
    double slope() const;
    double goodnessOfFit() const;
    unsigned char size() const;
};