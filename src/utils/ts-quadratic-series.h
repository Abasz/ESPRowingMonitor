#pragma once

#include <vector>

#include "configuration.h"
#include "series.h"

using std::vector;

class TSQuadraticSeries
{
    unsigned char maxSeriesLength = 0;
    unsigned short maxSeriesAInnerLength = 0;
    unsigned short maxSeriesALength = 0;
    unsigned short maxAllocationCapacity = 1000U;

    Configurations::precision a = 0;
    Configurations::precision b = 0;
    Configurations::precision c = 0;
    vector<vector<Configurations::precision>> seriesA;
    Series seriesX;
    Series seriesY;

    Configurations::precision calculateA(unsigned char pointOne, unsigned char pointTwo, unsigned char pointThree) const;
    Configurations::precision seriesAMedian() const;
    constexpr unsigned short calculateMaxSeriesALength() const;

    Configurations::precision projectX(Configurations::precision pointX) const;

public:
    explicit TSQuadraticSeries(unsigned char _maxSeriesLength = 0, unsigned short _maxAllocationCapacity = 1000);
    Configurations::precision firstDerivativeAtPosition(unsigned char position) const;
    Configurations::precision secondDerivativeAtPosition(unsigned char position) const;
    Configurations::precision goodnessOfFit() const;
    void push(Configurations::precision pointX, Configurations::precision pointY);
};
