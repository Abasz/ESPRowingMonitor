#include <algorithm>

#include "series.h"

Series::Series(const unsigned char _maxSeriesLength) : maxSeriesLength(_maxSeriesLength)
{
    if (_maxSeriesLength > 0)
    {
        seriesArray.reserve(_maxSeriesLength);
    }
}

Configurations::precision const &Series::operator[](size_t index) const
{
    return seriesArray[index];
};

size_t Series::size() const
{
    return seriesArray.size();
}

Configurations::precision Series::average() const
{
    if (!seriesArray.empty())
    {
        return seriesSum / (Configurations::precision)seriesArray.size();
    }

    return 0.0;
}

Configurations::precision Series::median() const
{
    if (!seriesArray.empty())
    {
        unsigned int const mid = seriesArray.size() / 2;
        vector<Configurations::precision> sortedArray(seriesArray);
        partial_sort(begin(sortedArray), begin(sortedArray) + mid + 1, end(sortedArray));

        return seriesArray.size() % 2 != 0
                   ? sortedArray[mid]
                   : (sortedArray[mid - 1] + sortedArray[mid]) / 2;
    }

    return 0.0;
}

void Series::push(const Configurations::precision value)
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

Configurations::precision Series::sum() const
{
    return seriesSum;
}
