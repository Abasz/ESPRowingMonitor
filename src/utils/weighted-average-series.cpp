#include <algorithm>

#include "weighted-average-series.h"

WeightedAverageSeries::WeightedAverageSeries(const unsigned char _maxSeriesLength, unsigned short _maxAllocationCapacity) : maxSeriesLength(_maxSeriesLength), maxAllocationCapacity(_maxAllocationCapacity), weightSeries(_maxSeriesLength, _maxAllocationCapacity), weightedSeries(_maxSeriesLength, _maxAllocationCapacity) {}

size_t WeightedAverageSeries::size() const
{
    return weightedSeries.size();
}

size_t WeightedAverageSeries::capacity() const
{
    return weightedSeries.capacity();
}

Configurations::precision WeightedAverageSeries::average() const
{
    if (weightedSeries.size() == 0 || weightedSeries.sum() == 0)
    {
        return 0.0;
    }

    return (weightedSeries.sum() / weightSeries.sum());
}

void WeightedAverageSeries::push(const Configurations::precision value, const Configurations::precision weight)
{
    weightedSeries.push(value);
    weightSeries.push(weight);
}

void WeightedAverageSeries::reset()
{
    weightedSeries.reset();
    weightSeries.reset();
}
