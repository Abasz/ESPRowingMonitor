#pragma once

#include "series.h"

using std::vector;

class OLSLinearSeries
{
    unsigned char maxSeriesLength = 0;
    Series sumX;
    Series sumXSquare;
    Series sumY;
    Series sumYSquare;
    Series sumXY;

public:
    explicit OLSLinearSeries(unsigned char _maxSeriesLength = 0);

    double yAtSeriesBegin() const;
    double slope() const;
    double goodnessOfFit() const;
    unsigned char size() const;

    void push(double pointX, double pointY);
    void reset();
};