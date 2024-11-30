#pragma once

#include "../configuration.h"
#include "./series.h"

using std::size_t;

class WeightedAverageSeries
{
    Series weightSeries;
    Series weightedSeries;

public:
    constexpr explicit WeightedAverageSeries(const unsigned char _maxSeriesLength = 0, const unsigned short _maxAllocationCapacity = 1'000) : weightSeries(_maxSeriesLength, _maxAllocationCapacity), weightedSeries(_maxSeriesLength, _maxAllocationCapacity) {}

    size_t size() const;
    size_t capacity() const;
    Configurations::precision average() const;

    void push(Configurations::precision value, Configurations::precision weight);
    void reset();
};
