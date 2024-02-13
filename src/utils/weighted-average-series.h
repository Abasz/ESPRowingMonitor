#pragma once

#include <vector>

#include "configuration.h"
#include "series.h"

using std::size_t;
using std::vector;

class WeightedAverageSeries
{
    unsigned char maxSeriesLength = 0;
    unsigned short maxAllocationCapacity = 1000;
    Series weightSeries;
    Series weightedSeries;

public:
    explicit WeightedAverageSeries(unsigned char _maxSeriesLength = 0, unsigned short _maxAllocationCapacity = 1000);

    const Configurations::precision &operator[](size_t index) const;

    size_t size() const;
    size_t capacity() const;
    Configurations::precision average() const;

    void push(Configurations::precision value, Configurations::precision weight);
    void reset();
};