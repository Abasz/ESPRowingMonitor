#include <algorithm>
#include <numeric>
#include <vector>

#include "./ts-linear-series.h"

using std::vector;

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

    if (pointOne == pointTwo || seriesXPointOne == seriesXPointTwo)
    {
        return 0.0;
    }

    return (seriesY[pointTwo] - seriesY[pointOne]) /
           (seriesXPointTwo - seriesXPointOne);
}

Configurations::precision TSLinearSeries::yAtSeriesBegin() const
{
    return seriesY[0];
}

Configurations::precision TSLinearSeries::coefficientA()
{
    if (shouldRecalculateA)
    {
        a = median();
        shouldRecalculateA = false;
    }

    return a;
}

Configurations::precision TSLinearSeries::coefficientB()
{
    if (shouldRecalculateB)
    {
        a = median();

        auto i = 0U;
        Series intercepts(maxSeriesLength, seriesX.size() - 1);
        while (i < seriesX.size() - 1)
        {
            intercepts.push((seriesY[i] - (a * seriesX[i])));
            i++;
        }
        b = intercepts.median();
        shouldRecalculateB = false;
    }

    return b;
}

Configurations::precision TSLinearSeries::median() const
{
    if (slopes.empty())
    {
        return 0.0;
    }

    vector<Configurations::precision> flattened;
    if (maxSlopeSeriesLength > 0)
    {
        flattened.reserve(maxSlopeSeriesLength);
    }

    for (const auto &slope : slopes)
    {
        flattened.insert(cend(flattened), cbegin(slope), cend(slope));
    }

    const unsigned int mid = flattened.size() / 2;

    std::nth_element(begin(flattened), begin(flattened) + mid, end(flattened));

    if (flattened.size() % 2 != 0)
    {
        return flattened[mid];
    }

    return (flattened[mid] + *std::max_element(cbegin(flattened), cbegin(flattened) + mid)) / 2;
}

void TSLinearSeries::push(const Configurations::precision pointX, const Configurations::precision pointY)
{
    seriesX.push(pointX);
    seriesY.push(pointY);
    shouldRecalculateA = true;
    shouldRecalculateB = true;

    if (maxSeriesLength > 0 && slopes.size() >= maxSeriesLength)
    {
        // The maximum of the array has been reached, we have to create room in the 2D array by removing the first row from the table
        slopes.erase(begin(slopes));
    }

    // Invariant: the indices of the X and Y array now match up with the row numbers of the slopes array. So, the slope of (X[0],Y[0]) and (X[1],Y[1] will be stored in slopes[0][.].

    // Calculate the slopes of this new point
    if (seriesX.size() > 1)
    {
        // There are at least two points in the X and Y arrays, so let's add the new datapoint
        auto i = 0U;
        while (i < seriesX.size() - 1)
        {
            const auto result = calculateSlope(i, slopes.size());
            slopes[i].push_back(result);
            i++;
        }
    }

    // Add an empty array at the end to store future results for the most recent points
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

size_t TSLinearSeries::size() const
{
    return seriesY.size();
}