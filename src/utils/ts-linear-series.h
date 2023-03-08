#pragma once

#include <vector>

#include "series.h"

class TSLinearSeries
{
    unsigned char maxSeriesLength = 0;
    double a = 0;

    // TODO: convert this to a template to take advantage of generics as double is not always needed; so here the Series type would be the same as the TSLInear
    Series seriesX;
    Series seriesY;
    vector<vector<double>> slopes;

    double calculateSlope(unsigned char pointOne, unsigned char pointTwo) const;
    void removeFirstRow();

public:
    explicit TSLinearSeries(unsigned char _maxSeriesLength = 0);
    double median() const;

    double coefficientA() const;
    // TODO: convert this to a template to take advantage of generics as double is not always needed
    void push(double x, double y);
    void reset();
};