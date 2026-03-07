#include "./exponential-weighted-average.h"

#include "../configuration.h"

void ExponentialWeightedAverage::push(const Configurations::precision value, const Configurations::precision weight)
{
    const auto weightedValue = value * weight;
    weightedSum = weightedSum * decayFactor + weightedValue;
    totalWeight = totalWeight * decayFactor + weight;
}

void ExponentialWeightedAverage::decay(const Configurations::precision decayFactorOverride)
{
    weightedSum = weightedSum * decayFactorOverride;
    totalWeight = totalWeight * decayFactorOverride;
}

Configurations::precision ExponentialWeightedAverage::average() const
{
    return totalWeight > 0.0 ? weightedSum / totalWeight : 0.0;
}

void ExponentialWeightedAverage::reset()
{
    weightedSum = initialBuffer;
    totalWeight = initialBuffer;
}
