#pragma once

#include <vector>

#include "series.h"

class TSLinearSeries
{
    unsigned char maxSeriesLength = 0;
    double a = 0;

    Series seriesX;
    Series seriesY;
    vector<vector<double>> slopes;

    double calculateSlope(unsigned char pointOne, unsigned char pointTwo) const;
    void removeFirstRow();

public:
    explicit TSLinearSeries(unsigned char _maxSeriesLength = 0);

    double median() const;
    double coefficientA() const;

    void push(double pointX, double pointY);
    void reset();
};