#pragma once

#include <vector>

#include "../settings.h"
#include "series.h"

class TSLinearSeries
{
    unsigned char maxSeriesLength = 0;
    Settings::precision a = 0;

    Series seriesX;
    Series seriesY;
    vector<vector<Settings::precision>> slopes;

    Settings::precision calculateSlope(unsigned char pointOne, unsigned char pointTwo) const;
    void removeFirstRow();

public:
    explicit TSLinearSeries(unsigned char _maxSeriesLength = 0);

    Settings::precision median() const;
    Settings::precision coefficientA() const;

    void push(Settings::precision pointX, Settings::precision pointY);
    void reset();
};