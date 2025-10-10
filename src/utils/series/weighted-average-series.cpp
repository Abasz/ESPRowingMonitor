#include <algorithm>

#include "./weighted-average-series.h"

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
    const auto weightSeriesSum = weightSeries.sum();    

    if (weightedSeries.size() == 0 || weightSeriesSum == 0)
    {
        return 0.0;
    }

    return (weightedSeries.sum() / weightSeriesSum);
}

void WeightedAverageSeries::push(const Configurations::precision value, const Configurations::precision weight)
{
    weightedSeries.push(value * weight);
    weightSeries.push(weight);
}

void WeightedAverageSeries::reset()
{
    weightedSeries.reset();
    weightSeries.reset();
}
