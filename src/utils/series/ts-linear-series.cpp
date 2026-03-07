#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>

#include "./ts-linear-series.h"

#include "../configuration.h"
#include "./series.h"

using std::vector;

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

Configurations::precision TSLinearSeries::xAtSeriesEnd() const
{
    return seriesX.back();
}

Configurations::precision TSLinearSeries::xAtSeriesBegin() const
{
    return seriesX.front();
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
    if (seriesX.size() < 2)
    {
        return 0.0;
    }

    if (shouldRecalculateB)
    {
        a = median();

        const auto seriesXSize = seriesX.size() - 1;
        Series intercepts(maxSeriesLength, initialCapacity, seriesXSize);

        auto i = 0U;
        while (i < seriesXSize)
        {
            intercepts.push((seriesY[i] - (a * seriesX[i])));
            ++i;
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

    std::ranges::copy(slopes | std::views::join, std::back_inserter(flattened));

    const auto flattenedSize = flattened.size();
    const unsigned int mid = flattenedSize / 2;

    std::ranges::nth_element(flattened, begin(flattened) + mid);

    if ((flattenedSize & 1) != 0)
    {
        return flattened[mid];
    }
    return (flattened[mid] + *std::ranges::max_element(cbegin(flattened), cbegin(flattened) + mid)) / 2;
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
        const auto seriesXPoints = seriesX.size() - 1;
        const auto slopesSize = slopes.size();
        auto i = 0U;
        while (i < seriesXPoints)
        {
            const auto result = calculateSlope(i, slopesSize);
            slopes[i].push_back(result);
            ++i;
        }
    }

    // Add an empty array at the end to store future results for the most recent points
    slopes.emplace_back();
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