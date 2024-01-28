#include <algorithm>

#include "series.h"

Series::Series(const unsigned char _maxSeriesLength, unsigned short _maxAllocationCapacity) : maxSeriesLength(_maxSeriesLength), maxAllocationCapacity(_maxAllocationCapacity)
{
    seriesArray.reserve(_maxSeriesLength == 0 ? Configurations::minimumRecoveryTime / Configurations::rotationDebounceTimeMin : _maxSeriesLength);
}

const Configurations::precision &Series::operator[](size_t index) const
{
    return seriesArray[index];
};

size_t Series::size() const
{
    return seriesArray.size();
}

size_t Series::capacity() const
{
    return seriesArray.capacity();
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
        const unsigned int mid = seriesArray.size() / 2;
        vector<Configurations::precision> sortedArray(mid + 1);
        // Note: it has been tested that partial_sort_copy implementation performs better than nth_element in any constellation for this situation. I assume it is the copying to the new array that cost more than O(n) time complexity of nth_element brings to the table
        partial_sort_copy(begin(seriesArray), end(seriesArray), begin(sortedArray), end(sortedArray));

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
        seriesArray.erase(begin(seriesArray));
    }

    // do manual memory reallocation via reserve if size is not known for better memory management
    if (maxSeriesLength == 0 && seriesArray.capacity() < seriesArray.size() + 1)
    {
        const auto maxCapacity = std::min<unsigned int>(maxAllocationCapacity, 1000U);

        if (seriesArray.capacity() * 2 > maxCapacity)
        {
            seriesArray.reserve(std::max<unsigned int>(maxCapacity, seriesArray.size() + 10U));
        }
    }

    seriesArray.push_back(value);
    seriesSum += value;
}

void Series::reset()
{
    vector<Configurations::precision> clear;
    const auto maxCapacity = maxAllocationCapacity;

    clear.reserve(maxSeriesLength == 0 ? std::min<unsigned int>(seriesArray.size(), maxCapacity) : maxSeriesLength);
    seriesArray.swap(clear);

    seriesSum = 0;
}

Configurations::precision Series::sum() const
{
    return seriesSum;
}
