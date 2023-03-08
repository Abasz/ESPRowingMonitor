#include <algorithm>
#include <numeric>

#include "ts-linear-series.h"

TSLinearSeries::TSLinearSeries(unsigned char _maxSeriesLength) : maxSeriesLength(_maxSeriesLength), seriesX(_maxSeriesLength), seriesY(_maxSeriesLength)
{
}

// EXEC_TIME_15: approx 450us
double TSLinearSeries::calculateSlope(unsigned char pointOne, unsigned char pointTwo) const
{
    auto seriesXPointOne = seriesX.seriesArray.at(pointOne);
    auto seriesXPointTwo = seriesX.seriesArray.at(pointTwo);

    if (pointOne != pointTwo && seriesXPointOne != seriesXPointTwo)
    {
        return (seriesY.seriesArray.at(pointTwo) - seriesY.seriesArray.at(pointOne)) /
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
    if (slopes.size() > 0)
    {
        vector<double> flattened;

        for (auto &slope : slopes)
            flattened.insert(end(flattened), begin(slope), end(slope));

        auto mid = flattened.size() / 2;
        partial_sort(begin(flattened), begin(flattened) + mid + 1, end(flattened));

        return flattened.size() % 2 != 0
                   ? flattened[mid]
                   : (flattened[mid - 1] + flattened[mid]) / 2;
    }

    return 0.0;
}

void TSLinearSeries::push(double x, double y)
{
    seriesX.push(x);
    seriesY.push(y);

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