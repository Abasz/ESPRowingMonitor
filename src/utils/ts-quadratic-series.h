#pragma once

#include <vector>

#include "series.h"

using std::vector;

class TSQuadraticSeries
{
    double a = 0;
    double b = 0;
    unsigned char maxSeriesLength = 0;
    vector<vector<double>> seriesA;
    Series seriesX;
    Series seriesY;

    double calculateA(unsigned char pointOne, unsigned char pointThree) const;
    static double matrixMedian(vector<vector<double>> inputMatrix);

public:
    explicit TSQuadraticSeries(unsigned char _maxSeriesLength = 0);
    double firstDerivativeAtPosition(unsigned char position) const;
    double secondDerivativeAtPosition(unsigned char position) const;
    void push(double pointX, double pointY);
};
