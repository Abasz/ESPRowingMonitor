#include <algorithm>
#include <numeric>

#include "ts-linear-series.h"
#include "ts-quadratic-series.h"

TSQuadraticSeries::TSQuadraticSeries(const unsigned char _maxSeriesLength) : maxSeriesLength(_maxSeriesLength), seriesX(_maxSeriesLength), seriesY(_maxSeriesLength)
{
    if (_maxSeriesLength > 0)
    {
        seriesA.reserve(_maxSeriesLength);
    }
}

Configurations::precision TSQuadraticSeries::firstDerivativeAtPosition(const unsigned char position) const
{
    if (seriesX.size() > 2 && position < seriesX.size())
    {
        return a * 2 * seriesX[position] + b;
    }

    return 0;
}

Configurations::precision TSQuadraticSeries::secondDerivativeAtPosition(const unsigned char position) const
{
    if (seriesX.size() > 2 && position < seriesX.size())
    {
        return a * 2;
    }

    return 0;
}

void TSQuadraticSeries::push(const Configurations::precision pointX, const Configurations::precision pointY)
{
    TSLinearSeries linearResidue(maxSeriesLength);

    seriesX.push(pointX);
    seriesY.push(pointY);

    if (maxSeriesLength > 0 && seriesA.size() >= maxSeriesLength)
    {
        // the maximum of the array has been reached, we have to create room
        // in the 2D array by removing the first row from the A-table
        seriesA.erase(seriesA.begin());
    }

    // invariant: the indices of the X and Y array now match up with the
    // row numbers of the A array. So, the A of (X[0],Y[0]) and (X[1],Y[1]
    // will be stored in A[0][.].

    // Add an empty array at the end to store futurs results for the most recent points
    seriesA.push_back({});
    if (maxSeriesLength > 0)
    {
        seriesA[seriesA.size() - 1].reserve(maxSeriesLength);
    }

    // calculate the coefficients of this new point
    if (seriesX.size() > 2)
    {
        // there are at least two points in the X and Y arrays, so let's add the new datapoint
        auto i = 0U;
        while (i < seriesX.size() - 2U)
        {
            seriesA[seriesX.size() - 1].push_back(calculateA(i, seriesX.size() - 1));
            i++;
        }
        a = matrixMedian(seriesA);

        i = 0;
        while (i < seriesX.size() - 1U)
        {
            auto const seriesXPointI = seriesX[i];
            linearResidue.push(
                seriesXPointI,
                seriesY[i] - a * (seriesXPointI * seriesXPointI));
            i++;
        }
        b = linearResidue.coefficientA();
    }
    else
    {
        a = 0;
        b = 0;
    }
}

// EXEC_TIME_15: approx 2000us
Configurations::precision TSQuadraticSeries::calculateA(const unsigned char pointOne, const unsigned char pointThree) const
{
    auto const xPointOne = seriesX[pointOne];
    auto const xPointThree = seriesX[pointThree];

    if (pointOne + 1 < pointThree && xPointOne != xPointThree)
    {
        Series results(maxSeriesLength);
        auto pointTwo = pointOne + 1;

        auto const xPointTwo = seriesX[pointTwo];

        while (
            pointOne < pointTwo &&
            pointTwo < pointThree &&
            xPointOne != xPointTwo &&
            xPointTwo != xPointThree)
        {
            // for the underlying math, see https://www.quora.com/How-do-I-find-a-quadratic-equation-from-points/answer/Robert-Paxson
            auto const xPointTwo = seriesX[pointTwo];
            auto const yPointThree = seriesY[pointThree];
            auto const yPointTwo = seriesY[pointTwo];

            results.push(
                (xPointOne * (yPointThree - yPointTwo) +
                 seriesY[pointOne] * (xPointTwo - xPointThree) +
                 (xPointThree * yPointTwo - xPointTwo * yPointThree)) /
                ((xPointOne - xPointTwo) * (xPointOne - xPointThree) * (xPointTwo - xPointThree)));
            pointTwo += 1;
        }

        return results.median();
    }

    return 0.0;
}

// EXEC_TIME_15: approx 800us
Configurations::precision TSQuadraticSeries::matrixMedian(const vector<vector<Configurations::precision>> inputMatrix)
{
    if (inputMatrix.size() > 1)
    {
        vector<Configurations::precision> flattened;

        for (auto &input : inputMatrix)
        {
            flattened.insert(end(flattened), begin(input), end(input));
        }

        unsigned int mid = flattened.size() / 2;
        partial_sort(begin(flattened), begin(flattened) + mid + 1, end(flattened));

        return flattened.size() % 2 != 0
                   ? flattened[mid]
                   : (flattened[mid - 1] + flattened[mid]) / 2;
    }

    return 0.0;
}
