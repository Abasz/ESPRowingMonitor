#include <algorithm>
#include <numeric>

#include "ts-linear-series.h"

TSLinearSeries::TSLinearSeries(const unsigned char _maxSeriesLength, unsigned short _maxAllocationCapacity) : maxSeriesLength(_maxSeriesLength), maxSlopeSeriesLength(((_maxSeriesLength - 2) * (_maxSeriesLength - 1)) / 2), seriesX(_maxSeriesLength, _maxAllocationCapacity), seriesY(_maxSeriesLength, _maxAllocationCapacity)
{
    if (_maxSeriesLength > 0)
    {
        slopes.reserve(_maxSeriesLength);
    }
}

Configurations::precision TSLinearSeries::calculateSlope(const unsigned char pointOne, const unsigned char pointTwo) const
{
    const auto seriesXPointOne = seriesX[pointOne];
    const auto seriesXPointTwo = seriesX[pointTwo];

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

Configurations::precision TSLinearSeries::median() const
{
    if (!slopes.empty())
    {
        vector<Configurations::precision> flattened;
        if (maxSlopeSeriesLength > 0)
        {
            flattened.reserve(maxSlopeSeriesLength);
        }

        for (const auto &slope : slopes)
        {
            flattened.insert(end(flattened), begin(slope), end(slope));
        }

        const unsigned int mid = flattened.size() / 2;

        std::nth_element(begin(flattened), begin(flattened) + mid, end(flattened));

        if (flattened.size() % 2 != 0)
        {
            return flattened[mid];
        }

        return (flattened[mid] + *std::max_element(begin(flattened), begin(flattened) + mid)) / 2;
    }

    return 0.0;
}

void TSLinearSeries::push(const Configurations::precision pointX, const Configurations::precision pointY)
{
    seriesX.push(pointX);
    seriesY.push(pointY);

    if (maxSeriesLength > 0 && slopes.size() >= maxSeriesLength)
    {
        // the maximum of the array has been reached, we have to create room
        // in the 2D array by removing the first row from the table
        slopes.erase(begin(slopes));
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
            const auto result = calculateSlope(i, slopes.size());
            slopes[i].push_back(result);
            i++;
        }
        a = median();
    }
    // add an empty array at the end to store futurs results for the most recent points
    slopes.push_back({});
    if (maxSeriesLength > 0)
    {
        slopes[slopes.size() - 1].reserve(maxSeriesLength - 2);
    }
}

void TSLinearSeries::reset()
{
    seriesX.reset();
    seriesY.reset();

    vector<vector<Configurations::precision>> clear;
    clear.reserve(maxSeriesLength > 0 ? maxSeriesLength : slopes.size());
    slopes.swap(clear);

    a = 0;
}