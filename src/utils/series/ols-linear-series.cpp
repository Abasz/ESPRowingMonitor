#include "./ols-linear-series.h"

void OLSLinearSeries::reset()
{
    seriesX.reset();
    seriesXSquare.reset();
    seriesY.reset();
    seriesYSquare.reset();
    seriesXY.reset();
}

void OLSLinearSeries::push(const Configurations::precision pointX, const Configurations::precision pointY)
{
    seriesX.push(pointX);
    seriesXSquare.push(pointX * pointX);
    seriesY.push(pointY);
    seriesYSquare.push(pointY * pointY);
    seriesXY.push(pointX * pointY);
}

Configurations::precision OLSLinearSeries::yAtSeriesBegin() const
{
    return seriesY[0];
}

Configurations::precision OLSLinearSeries::slope() const
{
    if (seriesX.size() < 2 || seriesX.sum() == 0)
    {
        return 0.0;
    }

    return ((Configurations::precision)seriesX.size() * seriesXY.sum() - seriesX.sum() * seriesY.sum()) / ((Configurations::precision)seriesX.size() * seriesXSquare.sum() - seriesX.sum() * seriesX.sum());
}

Configurations::precision OLSLinearSeries::goodnessOfFit() const
{
    // This function returns the R^2 as a goodness of fit indicator
    if (seriesX.size() < 2 || seriesX.sum() == 0)
    {
        return 0;
    }

    const auto slope = ((Configurations::precision)seriesX.size() * seriesXY.sum() - seriesX.sum() * seriesY.sum()) / ((Configurations::precision)seriesX.size() * seriesXSquare.sum() - seriesX.sum() * seriesX.sum());
    const auto intercept = (seriesY.sum() - (slope * seriesX.sum())) / (Configurations::precision)seriesX.size();
    const auto sse = seriesYSquare.sum() - (intercept * seriesY.sum()) - (slope * seriesXY.sum());
    const auto sst = seriesYSquare.sum() - (seriesY.sum() * seriesY.sum()) / (Configurations::precision)seriesX.size();
    return 1 - (sse / sst);
}

size_t OLSLinearSeries::size() const
{
    return seriesY.size();
}