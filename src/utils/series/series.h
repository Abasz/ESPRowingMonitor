#pragma once

#include <vector>

#include "../configuration.h"

using std::size_t;

class Series
{
    unsigned char maxSeriesLength;
    unsigned short maxAllocationCapacity;
    Configurations::precision seriesSum = 0;
    std::vector<Configurations::precision> seriesArray;

public:
    constexpr explicit Series(
        const unsigned char _maxSeriesLength = 0,
        const unsigned short initialCapacity = Configurations::defaultAllocationCapacity,
        const unsigned short _maxAllocationCapacity = 1'000) : maxSeriesLength(_maxSeriesLength), maxAllocationCapacity(_maxAllocationCapacity)
    {
        seriesArray.reserve(_maxSeriesLength > 0 ? _maxSeriesLength : initialCapacity);
    }

    const Configurations::precision &operator[](size_t index) const;

    size_t size() const;
    size_t capacity() const;
    Configurations::precision average() const;
    Configurations::precision median() const;
    Configurations::precision sum() const;

    void push(Configurations::precision value);
    void reset();
};
