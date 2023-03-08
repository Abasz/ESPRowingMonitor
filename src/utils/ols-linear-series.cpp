#include "ols-linear-series.h"

OLSLinearSeries::OLSLinearSeries(unsigned char _maxSeriesLength) : maxSeriesLength(_maxSeriesLength),
                                                                   sumX(_maxSeriesLength),
                                                                   sumXSquare(_maxSeriesLength),
                                                                   sumY(_maxSeriesLength),
                                                                   sumYSquare(_maxSeriesLength),
                                                                   sumXY(_maxSeriesLength) {}

void OLSLinearSeries::resetData()
{
    sumX.reset();
    sumXSquare.reset();
    sumY.reset();
    sumYSquare.reset();
    sumXY.reset();
}

void OLSLinearSeries::push(unsigned long long x, unsigned long long y)
{
    sumX.push(x);
    sumXSquare.push(x * x);
    sumY.push(y);
    sumYSquare.push(y * y);
    sumXY.push(x * y);
}

unsigned long long OLSLinearSeries::yAtSeriesBegin() const
{
    return sumY.seriesArray[0];
}

double OLSLinearSeries::slope() const
{
    if (sumX.size() > 0 && sumX.sum() > 0)
    {
        return (double)(sumX.size() * sumXY.sum() - sumX.sum() * sumY.sum()) / (double)(sumX.size() * sumXSquare.sum() - sumX.sum() * sumX.sum());
    }
    else
    {
        return 0.0;
    }
}

double OLSLinearSeries::goodnessOfFit() const
{
    // This function returns the R^2 as a goodness of fit indicator
    if (sumX.size() >= 2 && sumX.sum() > 0)
    {
        auto slope = (double)(sumX.size() * sumXY.sum() - sumX.sum() * sumY.sum()) / (double)(sumX.size() * sumXSquare.sum() - sumX.sum() * sumX.sum());
        auto intercept = (double)(sumY.sum() - (slope * sumX.sum())) / sumX.size();
        auto sse = sumYSquare.sum() - (intercept * sumY.sum()) - (slope * sumXY.sum());
        auto sst = sumYSquare.sum() - (double)(sumY.sum() * sumY.sum()) / sumX.size();
        return 1 - (sse / sst);
    }
    else
    {
        return 0;
    }
}

unsigned char OLSLinearSeries::size() const
{
    return sumY.seriesArray.size();
}