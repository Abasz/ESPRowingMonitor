#pragma once

#include <vector>

using std::vector;

class Series
{
    unsigned char maxSeriesLength = 0UL;
    double seriesSum = 0ULL;

public:
    explicit Series(unsigned char _maxSeriesLength = 0);

    // TODO: convert this to a template to take advantage of generics as double is not always needed
    vector<double> seriesArray;

    unsigned char size() const;
    double median() const;
    double sum() const;

    void push(double value);
    void reset();
};
