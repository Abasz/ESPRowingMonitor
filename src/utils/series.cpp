#include <algorithm>

#include "series.h"

Series::Series(unsigned char _maxSeriesLength) : maxSeriesLength(_maxSeriesLength)
{
    if (_maxSeriesLength > 0)
    {
        seriesArray.reserve(_maxSeriesLength);
    }
}

double const &Series::operator[](size_t index) const
{
    return seriesArray[index];
};

size_t Series::size() const
{
    return seriesArray.size();
}

double Series::average() const
{
    if (!seriesArray.empty())
    {
        return seriesSum / (double)seriesArray.size();
    }

    return 0.0;
}

double Series::median() const
{
    if (!seriesArray.empty())
    {
        unsigned int mid = seriesArray.size() / 2;
        vector<double> sortedArray(seriesArray);
        partial_sort(begin(sortedArray), begin(sortedArray) + mid + 1, end(sortedArray));

        return seriesArray.size() % 2 != 0
                   ? sortedArray[mid]
                   : (sortedArray[mid - 1] + sortedArray[mid]) / 2;
    }

    return 0.0;
}

void Series::push(double value)
{
    if (maxSeriesLength > 0 && seriesArray.size() >= maxSeriesLength)
    {
        // the maximum of the array has been reached, we have to create room by removing the first
        // value from the array
        seriesSum -= seriesArray[0];
        seriesArray.erase(seriesArray.begin());
    }

    seriesArray.push_back(value);
    seriesSum += value;
}

void Series::reset()
{
    seriesArray.clear();
    seriesSum = 0;
}

double Series::sum() const
{
    return seriesSum;
}
