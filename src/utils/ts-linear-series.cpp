#include <algorithm>
#include <numeric>

#include "ts-linear-series.h"

TSLinearSeries::TSLinearSeries(unsigned char _maxSeriesLength) : maxSeriesLength(_maxSeriesLength), seriesX(_maxSeriesLength), seriesY(_maxSeriesLength)
{
    if (_maxSeriesLength > 0)
    {
        slopes.reserve(_maxSeriesLength);
    }
}

// EXEC_TIME_15: approx 450us
Configurations::precision TSLinearSeries::calculateSlope(unsigned char pointOne, unsigned char pointTwo) const
{
    auto const seriesXPointOne = seriesX[pointOne];
    auto const seriesXPointTwo = seriesX[pointTwo];

    if (pointOne != pointTwo && seriesXPointOne != seriesXPointTwo)
    {
        return (seriesY[pointTwo] - seriesY[pointOne]) /
               (seriesXPointTwo - seriesXPointOne);
    }

    return 0.0;
}

Configurations::precision TSLinearSeries::coefficientA() const
{
    return a;
}

// EXEC_TIME_15: approx 1670us
Configurations::precision TSLinearSeries::median() const
{
    if (!slopes.empty())
    {
        vector<Configurations::precision> flattened;

        for (auto const &slope : slopes)
        {
            flattened.insert(end(flattened), begin(slope), end(slope));
        }

        unsigned int mid = flattened.size() / 2;
        partial_sort(begin(flattened), begin(flattened) + mid + 1, end(flattened));

        return flattened.size() % 2 != 0
                   ? flattened[mid]
                   : (flattened[mid - 1] + flattened[mid]) / 2;
    }

    return 0.0;
}

void TSLinearSeries::push(Configurations::precision pointX, Configurations::precision pointY)
{
    seriesX.push(pointX);
    seriesY.push(pointY);

    if (maxSeriesLength > 0 && slopes.size() >= maxSeriesLength)
    {
        // the maximum of the array has been reached, we have to create room
        // in the 2D array by removing the first row from the table
        removeFirstRow();
    }

    // invariant: the indices of the X and Y array now match up with the
    // row numbers of the slopes array. So, the slope of (X[0],Y[0]) and (X[1],Y[1]
    // will be stored in slopes[0][.].

    // Calculate the slopes of this new point
    if (seriesX.size() > 1)
    {
        // there are at least two points in the X and Y arrays, so let's add the new datapoint
        auto i = 0U;
        while (i < slopes.size())
        {
            auto const result = calculateSlope(i, slopes.size());
            slopes[i].push_back(result);
            i++;
        }
    }
    // add an empty array at the end to store futurs results for the most recent points
    slopes.push_back({});
    if (maxSeriesLength > 0)
    {
        slopes[slopes.size() - 1].reserve(maxSeriesLength);
    }
    // calculate the median of the slopes
    if (seriesX.size() > 1)
    {
        a = median();
    }
}

void TSLinearSeries::removeFirstRow()
{
    slopes.erase(slopes.begin());
}

void TSLinearSeries::reset()
{
    seriesX.reset();
    seriesY.reset();
    vector<vector<Configurations::precision>> clear;
    slopes.swap(clear);
    a = 0;
}