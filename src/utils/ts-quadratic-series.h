#pragma once

#include <vector>

#include "../settings.h"
#include "series.h"

using std::vector;

class TSQuadraticSeries
{
    Settings::precision a = 0;
    Settings::precision b = 0;
    unsigned char maxSeriesLength = 0;
    vector<vector<Settings::precision>> seriesA;
    Series seriesX;
    Series seriesY;

    Settings::precision calculateA(unsigned char pointOne, unsigned char pointThree) const;
    static Settings::precision matrixMedian(vector<vector<Settings::precision>> inputMatrix);

public:
    explicit TSQuadraticSeries(unsigned char _maxSeriesLength = 0);
    Settings::precision firstDerivativeAtPosition(unsigned char position) const;
    Settings::precision secondDerivativeAtPosition(unsigned char position) const;
    void push(Settings::precision pointX, Settings::precision pointY);
};
