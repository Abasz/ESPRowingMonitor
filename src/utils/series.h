#pragma once

#include <vector>

#include "./configuration.h"

using std::size_t;

class Series
{
    unsigned char maxSeriesLength = 0;
    unsigned short maxAllocationCapacity = 1'000;
    Configurations::precision seriesSum = 0;
    std::vector<Configurations::precision> seriesArray;

public:
    explicit Series(unsigned char _maxSeriesLength = 0, unsigned short _maxAllocationCapacity = 1'000);

    const Configurations::precision &operator[](size_t index) const;

    size_t size() const;
    size_t capacity() const;
    Configurations::precision average() const;
    Configurations::precision median() const;
    Configurations::precision sum() const;

    void push(Configurations::precision value);
    void reset();
};
