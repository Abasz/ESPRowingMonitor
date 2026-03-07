#pragma once

#include <cstddef>

#include "../configuration.h"
#include "./series.h"

using std::size_t;

class WeightedAverageSeries
{
    Series weightSeries;
    Series weightedSeries;

public:
    constexpr explicit WeightedAverageSeries(
        const unsigned char _maxSeriesLength = 0,
        const unsigned short _initialCapacity = Configurations::defaultAllocationCapacity,
        const unsigned short _maxAllocationCapacity = 1'000)
        : weightSeries(_maxSeriesLength, _initialCapacity, _maxAllocationCapacity),
          weightedSeries(_maxSeriesLength, _initialCapacity, _maxAllocationCapacity)
    {
    }

    [[nodiscard]] size_t size() const;
    [[nodiscard]] size_t capacity() const;
    [[nodiscard]] Configurations::precision average() const;

    void push(Configurations::precision value, Configurations::precision weight);
    void reset();
};
