#include <algorithm>
#include <numeric>

#include "./ts-linear-series.h"
#include "./ts-quadratic-series.h"

using std::vector;

TSQuadraticSeries::TSQuadraticSeries(const unsigned char _maxSeriesLength, unsigned short _maxAllocationCapacity) : maxSeriesLength(_maxSeriesLength), maxSeriesAInnerLength(((_maxSeriesLength - 2) * (_maxSeriesLength - 1)) / 2), maxSeriesALength(calculateMaxSeriesALength()), maxAllocationCapacity(_maxAllocationCapacity), seriesX(_maxSeriesLength, _maxAllocationCapacity), seriesY(_maxSeriesLength, _maxAllocationCapacity)
{
    if (_maxSeriesLength > 0)
    {
        seriesA.reserve(_maxSeriesLength - 3);
    }
}

Configurations::precision TSQuadraticSeries::firstDerivativeAtPosition(const unsigned char position) const
{
    if (seriesX.size() < 3 || position >= seriesX.size())
    {
        return 0;
    }

    return a * 2 * seriesX[position] + b;
}

Configurations::precision TSQuadraticSeries::secondDerivativeAtPosition(const unsigned char position) const
{
    if (seriesX.size() < 3 || position >= seriesX.size())
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

    if (seriesX.size() < 3)
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
    auto i = 0U;
    auto j = 0U;

    while (i < seriesX.size() - 2)
    {
        j = i + 1;
        while (j < seriesX.size() - 1)
        {
            seriesA[i].push_back(calculateA(i, j, seriesX.size() - 1));
            j++;
        }
        i++;
    }
    a = seriesAMedian();

    TSLinearSeries linearResidue(maxSeriesLength, maxAllocationCapacity);
    i = 0;
    while (i < seriesX.size())
    {
        const auto seriesXPointI = seriesX[i];
        linearResidue.push(
            seriesXPointI,
            seriesY[i] - a * (seriesXPointI * seriesXPointI));
        i++;
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

    const unsigned int mid = flattened.size() / 2;

    std::nth_element(begin(flattened), begin(flattened) + mid, end(flattened));

    if (flattened.size() % 2 != 0)
    {
        return flattened[mid];
    }

    return (flattened[mid] + *std::max_element(cbegin(flattened), cbegin(flattened) + mid)) / 2;
}

Configurations::precision TSQuadraticSeries::goodnessOfFit() const
{
    // This function returns the R^2 as a goodness of fit indicator
    if (seriesX.size() < 3)
    {
        return 0.0;
    }

    auto i = 0U;
    Configurations::precision sse = 0.0;
    Configurations::precision sst = 0.0;

    while (i < seriesX.size())
    {
        const auto projectedX = projectX(seriesX[i]);
        sse += (seriesY[i] - projectedX) * (seriesY[i] - projectedX);
        const auto averageY = seriesY.average();
        sst += (seriesY[i] - averageY) * (seriesY[i] - averageY);
        i++;
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

Configurations::precision TSQuadraticSeries::projectX(Configurations::precision valueX) const
{
    if (seriesX.size() < 3)
    {
        return 0.0;
    }

    return ((a * valueX * valueX) + (b * valueX) + c);
}

constexpr unsigned short TSQuadraticSeries::calculateMaxSeriesALength() const
{
    unsigned char baseValue = maxSeriesAInnerLength;
    unsigned short sum = baseValue;
    for (unsigned char i = 0; i < maxSeriesLength - 3; ++i)
    {
        baseValue -= maxSeriesLength - i - 2;
        sum += baseValue;
    }
    return sum;
}