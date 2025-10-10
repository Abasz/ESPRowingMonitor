#include <algorithm>
#include <numeric>

#include "./ts-linear-series.h"
#include "./ts-quadratic-series.h"

using std::vector;

Configurations::precision TSQuadraticSeries::firstDerivativeAtPosition(const unsigned char position) const
{
    const auto seriesXSize = seriesX.size();
    if (seriesXSize < 3 || position >= seriesXSize)
    {
        return 0;
    }

    return a * 2 * seriesX[position] + b;
}

Configurations::precision TSQuadraticSeries::secondDerivativeAtPosition(const unsigned char position) const
{
    const auto seriesXSize = seriesX.size();
    if (seriesXSize < 3 || position >= seriesXSize)
    {
        return 0;
    }

    return a * 2;
}

void TSQuadraticSeries::push(const Configurations::precision pointX, const Configurations::precision pointY)
{
    if (maxSeriesLength > 0 && seriesX.size() >= maxSeriesLength)
    {
        // The maximum of the array has been reached, we have to create room in the 2D array by removing the first row from the A-table
        seriesA.erase(begin(seriesA));
    }

    seriesX.push(pointX);
    seriesY.push(pointY);

    // Invariant: the indices of the X and Y array now match up with the row numbers of the A array. So, the A of (X[0],Y[0]) and (X[1],Y[1] will be stored in A[0][.].

    const auto seriesXSize = seriesX.size();
    if (seriesXSize < 3)
    {
        a = 0;
        b = 0;
        c = 0;

        return;
    }

    // Calculate the coefficients of this new point if we have three or more points in the series

    seriesA.push_back({});
    if (maxSeriesAInnerLength > 0)
    {
        seriesA[seriesA.size() - 1].reserve(maxSeriesAInnerLength);
    }

    // There are at least two points in the X and Y arrays, so let's add the new datapoint
    const auto seriesXInnerLength = seriesXSize - 2;
    auto i = 0U;
    auto j = 0U;
    while (i < seriesXInnerLength)
    {
        j = i + 1;
        const auto seriesXIndexLength = seriesXSize - 1;
        while (j < seriesXIndexLength)
        {
            seriesA[i].push_back(calculateA(i, j, seriesXIndexLength));
            j++;
        }
        ++i;
    }
    a = seriesAMedian();

    TSLinearSeries linearResidue(maxSeriesLength, initialCapacity, maxAllocationCapacity);
    i = 0;
    while (i < seriesXSize)
    {
        const auto seriesXPointI = seriesX[i];

        linearResidue.push(
            seriesXPointI,
            seriesY[i] - a * (seriesXPointI * seriesXPointI));
        ++i;
    }
    b = linearResidue.coefficientA();
    c = linearResidue.coefficientB();
}

Configurations::precision TSQuadraticSeries::calculateA(const unsigned char pointOne, const unsigned char pointTwo, const unsigned char pointThree) const
{
    const auto xPointOne = seriesX[pointOne];
    const auto xPointTwo = seriesX[pointTwo];
    const auto xPointThree = seriesX[pointThree];

    if (xPointOne == xPointTwo || xPointOne == xPointThree || xPointTwo == xPointThree)
    {
        return 0.0;
    }

    const auto yPointThree = seriesY[pointThree];
    const auto yPointTwo = seriesY[pointTwo];

    return (xPointOne * (yPointThree - yPointTwo) +
            seriesY[pointOne] * (xPointTwo - xPointThree) +
            (xPointThree * yPointTwo - xPointTwo * yPointThree)) /
           ((xPointOne - xPointTwo) * (xPointOne - xPointThree) * (xPointTwo - xPointThree));
}

Configurations::precision TSQuadraticSeries::seriesAMedian() const
{
    vector<Configurations::precision> flattened;
    if (maxSeriesALength > 0)
    {
        flattened.reserve(maxSeriesALength);
    }

    for (const auto &input : seriesA)
    {
        flattened.insert(cend(flattened), cbegin(input), end(input));
    }

    const auto flattenedSize = flattened.size();
    const unsigned int mid = flattenedSize / 2;

    std::nth_element(begin(flattened), begin(flattened) + mid, end(flattened));

    if (flattenedSize % 2 != 0)
    {
        return flattened[mid];
    }

    return (flattened[mid] + *std::max_element(cbegin(flattened), cbegin(flattened) + mid)) / 2;
}

// This function returns the R^2 as a goodness of fit indicator
Configurations::precision TSQuadraticSeries::goodnessOfFit() const
{
    const auto seriesXSize = seriesX.size();
    if (seriesXSize < 3)
    {
        return 0.0;
    }

    Configurations::precision sse = 0.0;
    Configurations::precision sst = 0.0;

    auto i = 0U;
    while (i < seriesXSize)
    {
        const auto seriesYI = seriesY[i];
        const auto projectedX = projectX(seriesX[i]);
        const auto averageY = seriesY.average();

        const auto seriesYProjectedXDiff = seriesYI - projectedX;
        const auto seriesYIAverageYDiff = seriesYI - averageY;

        sse += seriesYProjectedXDiff * seriesYProjectedXDiff;
        sst += seriesYIAverageYDiff * seriesYIAverageYDiff;
        ++i;
    }

    if (sst == 0 || sse > sst)
    {
        return 0;
    }

    if (sse == 0)
    {
        return 1;
    }

    return 1 - (sse / sst);
}

Configurations::precision TSQuadraticSeries::projectX(Configurations::precision pointX) const
{
    if (seriesX.size() < 3)
    {
        return 0.0;
    }

    return ((a * pointX * pointX) + (b * pointX) + c);
}