#pragma once

#include "../settings.h"
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

    Settings::precision yAtSeriesBegin() const;
    Settings::precision slope() const;
    Settings::precision goodnessOfFit() const;
    unsigned char size() const;

    void push(Settings::precision pointX, Settings::precision pointY);
    void reset();
};