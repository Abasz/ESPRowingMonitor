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
double TSLinearSeries::calculateSlope(unsigned char pointOne, unsigned char pointTwo) const
{
    auto seriesXPointOne = seriesX[pointOne];
    auto seriesXPointTwo = seriesX[pointTwo];

    if (pointOne != pointTwo && seriesXPointOne != seriesXPointTwo)
    {
        return (seriesY[pointTwo] - seriesY[pointOne]) /
               (seriesXPointTwo - seriesXPointOne);
    }

    return 0.0;
}

double TSLinearSeries::coefficientA() const
{
    return a;
}

// EXEC_TIME_15: approx 1670us
double TSLinearSeries::median() const
{
    if (!slopes.empty())
    {
        vector<double> flattened;

        for (const auto &slope : slopes)
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

void TSLinearSeries::push(double pointX, double pointY)
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
        auto result = 0.0;
        while (i < slopes.size())
        {
            result = calculateSlope(i, slopes.size());
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
    vector<vector<double>> clear;
    slopes.swap(clear);
    a = 0;
}