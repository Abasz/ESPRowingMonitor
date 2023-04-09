#pragma once

#include <vector>

#include "configuration.h"

using std::size_t;
using std::vector;

class Series
{
    unsigned char maxSeriesLength = 0UL;
    Configurations::precision seriesSum = 0;
    vector<Configurations::precision> seriesArray;

public:
    explicit Series(unsigned char _maxSeriesLength = 0);

    Configurations::precision const &operator[](size_t index) const;

    size_t size() const;
    Configurations::precision average() const;
    Configurations::precision median() const;
    Configurations::precision sum() const;

    void push(Configurations::precision value);
    void reset();
};
