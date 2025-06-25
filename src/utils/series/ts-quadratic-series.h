#pragma once

#include <vector>

#include "../configuration.h"
#include "./series.h"

using std::vector;

class TSQuadraticSeries
{
    unsigned char maxSeriesLength;
    unsigned short maxSeriesAInnerLength = ((maxSeriesLength - 2) * (maxSeriesLength - 1)) / 2;
    unsigned short maxSeriesALength;
    unsigned short maxAllocationCapacity;
    unsigned short initialCapacity;

    Configurations::precision a = 0;
    Configurations::precision b = 0;
    Configurations::precision c = 0;
    vector<vector<Configurations::precision>> seriesA;
    Series seriesX;
    Series seriesY;

    Configurations::precision calculateA(unsigned char pointOne, unsigned char pointTwo, unsigned char pointThree) const;
    Configurations::precision seriesAMedian() const;

    static constexpr unsigned short calculateMaxSeriesALength(const unsigned short seriesLength, const unsigned short seriesAInnerLength)
    {
        unsigned char baseValue = seriesAInnerLength;
        unsigned short sum = baseValue;
        for (unsigned char i = 0; i < seriesLength - 3; ++i)
        {
            baseValue -= seriesLength - i - 2;
            sum += baseValue;
        }
        return sum;
    }

    Configurations::precision projectX(Configurations::precision pointX) const;

public:
    constexpr explicit TSQuadraticSeries(
        const unsigned char _maxSeriesLength = 0,
        const unsigned short _initialCapacity = Configurations::defaultAllocationCapacity,
        const unsigned short _maxAllocationCapacity = 1'000) : maxSeriesLength(_maxSeriesLength), maxSeriesALength(calculateMaxSeriesALength(_maxSeriesLength, maxSeriesAInnerLength)), maxAllocationCapacity(_maxAllocationCapacity), initialCapacity(_initialCapacity), seriesX(_maxSeriesLength, _initialCapacity, _maxAllocationCapacity), seriesY(_maxSeriesLength, _initialCapacity, _maxAllocationCapacity)
    {
        if (_maxSeriesLength > 0)
        {
            seriesA.reserve(_maxSeriesLength - 3);
        }
    }

    Configurations::precision firstDerivativeAtPosition(unsigned char position) const;
    Configurations::precision secondDerivativeAtPosition(unsigned char position) const;
    Configurations::precision goodnessOfFit() const;
    void push(Configurations::precision pointX, Configurations::precision pointY);
};
