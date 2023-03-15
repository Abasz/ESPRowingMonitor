#pragma once

#include <vector>

using std::size_t;
using std::vector;

class Series
{
    unsigned char maxSeriesLength = 0UL;
    double seriesSum = 0;
    vector<double> seriesArray;

public:
    explicit Series(unsigned char _maxSeriesLength = 0);

    double const &operator[](size_t index) const;

    size_t size() const;
    double average() const;
    double median() const;
    double sum() const;

    void push(double value);
    void reset();
};
