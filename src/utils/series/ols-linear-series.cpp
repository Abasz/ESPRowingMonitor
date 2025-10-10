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
    const auto seriesXSize = seriesX.size();
    const auto seriesXSum = seriesX.sum();

    if (seriesXSize < 2 || seriesXSum == 0)
    {
        return 0.0;
    }

    return ((Configurations::precision)seriesXSize * seriesXY.sum() - seriesXSum * seriesY.sum()) / ((Configurations::precision)seriesXSize * seriesXSquare.sum() - seriesXSum * seriesXSum);
}

Configurations::precision OLSLinearSeries::goodnessOfFit() const
{
    const auto seriesXSize = seriesX.size();
    const auto seriesXSum = seriesX.sum();

    // This function returns the R^2 as a goodness of fit indicator
    if (seriesXSize < 2 || seriesXSum == 0)
    {
        return 0;
    }

    const auto seriesYSum = seriesY.sum();
    const auto seriesXYSum = seriesXY.sum();
    const auto seriesYSquareSum = seriesYSquare.sum();

    const auto slope = ((Configurations::precision)seriesXSize * seriesXYSum - seriesXSum * seriesYSum) / ((Configurations::precision)seriesXSize * seriesXSquare.sum() - seriesXSum * seriesXSum);
    const auto intercept = (seriesYSum - (slope * seriesXSum)) / (Configurations::precision)seriesXSize;
    const auto sse = seriesYSquareSum - (intercept * seriesYSum) - (slope * seriesXYSum);
    const auto sst = seriesYSquareSum - (seriesYSum * seriesYSum) / (Configurations::precision)seriesXSize;
    return 1 - (sse / sst);
}

size_t OLSLinearSeries::size() const
{
    return seriesY.size();
}