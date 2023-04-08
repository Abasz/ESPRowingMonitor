#pragma once

#include <vector>

#include "../settings.h"

using std::size_t;
using std::vector;

class Series
{
    unsigned char maxSeriesLength = 0UL;
    Settings::precision seriesSum = 0;
    vector<Settings::precision> seriesArray;

public:
    explicit Series(unsigned char _maxSeriesLength = 0);

    Settings::precision const &operator[](size_t index) const;

    size_t size() const;
    Settings::precision average() const;
    Settings::precision median() const;
    Settings::precision sum() const;

    void push(Settings::precision value);
    void reset();
};
