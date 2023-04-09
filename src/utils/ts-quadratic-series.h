#pragma once

#include <vector>

#include "configuration.h"
#include "series.h"

using std::vector;

class TSQuadraticSeries
{
    Configurations::precision a = 0;
    Configurations::precision b = 0;
    unsigned char maxSeriesLength = 0;
    vector<vector<Configurations::precision>> seriesA;
    Series seriesX;
    Series seriesY;

    Configurations::precision calculateA(unsigned char pointOne, unsigned char pointThree) const;
    static Configurations::precision matrixMedian(vector<vector<Configurations::precision>> inputMatrix);

public:
    explicit TSQuadraticSeries(unsigned char _maxSeriesLength = 0);
    Configurations::precision firstDerivativeAtPosition(unsigned char position) const;
    Configurations::precision secondDerivativeAtPosition(unsigned char position) const;
    void push(Configurations::precision pointX, Configurations::precision pointY);
};
