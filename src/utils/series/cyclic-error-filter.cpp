#include <algorithm>
#include <cmath>
#include <iterator>

#include "./cyclic-error-filter.h"

#include "../configuration.h"
#include "./exponential-weighted-average.h"
#include "./ols-linear-series.h"
#include "./series.h"

void CyclicErrorFilter::SlotErrorTracker::push(const Configurations::precision deviation)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    const auto oldValue = buffer[head];
    if (count == bufferSize)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions)
        signSum -= static_cast<signed char>(oldValue > 0) - static_cast<signed char>(oldValue < 0);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions)
    signSum += static_cast<signed char>(deviation > 0) - static_cast<signed char>(deviation < 0);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    buffer[head] = deviation;
    head = (head + 1) % bufferSize;
    if (count < bufferSize)
    {
        count++;
    }
}

Configurations::precision CyclicErrorFilter::SlotErrorTracker::median() const
{
    std::array<Configurations::precision, bufferSize> sorted(buffer);

    const auto mid = count / 2;
    std::ranges::nth_element(begin(sorted), std::next(begin(sorted), mid), std::next(begin(sorted), count));

    if ((count & 1) != 0)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        return sorted[mid];
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    return (sorted[mid] + *std::ranges::max_element(begin(sorted), std::next(begin(sorted), mid))) / 2;
}

Configurations::precision CyclicErrorFilter::SlotErrorTracker::meanSign() const
{
    if (count == 0)
    {
        return 0;
    }

    return static_cast<Configurations::precision>(signSum) / static_cast<Configurations::precision>(count);
}

Configurations::precision CyclicErrorFilter::SlotErrorTracker::calculateBoost() const
{
    if (count < bufferSize || std::abs(median()) < medianThreshold || std::abs(meanSign()) < signThreshold)
    {
        return 1.0;
    }

    const auto med = median();
    const auto medianNorm = std::min(std::abs(med) / medianSaturation, static_cast<Configurations::precision>(1.0));
    const auto confidence = medianNorm * std::abs(meanSign());
    const auto boost = 1.0 + confidence * (maxBoost - 1.0);

    return boost;
}

void CyclicErrorFilter::SlotErrorTracker::reset()
{
    count = 0;
    head = 0;
    signSum = 0;
}

const Series &CyclicErrorFilter::rawSeries() const
{
    return raw;
}

const Series &CyclicErrorFilter::cleanSeries() const
{
    return clean;
}

void CyclicErrorFilter::applyFilter(const unsigned long position, const Configurations::precision rawValue)
{
    raw.push(rawValue);
    clean.push(rawValue * filterConfig[position % numberOfSlots] * weightCorrection);
}

void CyclicErrorFilter::recordRawDatapoint(const unsigned long relativePosition, const Configurations::precision absolutePosition, const Configurations::precision rawValue)
{
    if (aggressiveness == 0)
    {
        return;
    }

    recordedRelativePosition.push_back(relativePosition);
    recordedAbsolutePosition.push_back(absolutePosition);
    recordedRawValue.push_back(rawValue);

    if (!isStabilized())
    {
        return;
    }

    const auto cleanValue = rawValue * filterConfig[relativePosition % numberOfSlots] * weightCorrection;

    rawOlsSeries.push((rawOlsSeries.size() > 0 ? rawOlsSeries.xAtSeriesEnd() : 0) + rawValue, rawValue);
    cleanOlsSeries.push((cleanOlsSeries.size() > 0 ? cleanOlsSeries.xAtSeriesEnd() : 0) + cleanValue, cleanValue);
}

void CyclicErrorFilter::processNextRawDatapoint()
{
    if (recordedRawValue.empty())
    {
        return;
    }

    if (cursor >= recordedRawValue.size())
    {
        restart();

        return;
    }

    const auto perfectCurrentDt = regressionSlope * static_cast<Configurations::precision>(recordedAbsolutePosition[cursor]) + regressionIntercept;
    updateFilter(recordedRelativePosition[cursor], recordedRawValue[cursor], perfectCurrentDt);
    cursor++;
}

void CyclicErrorFilter::updateRegressionCoefficients(const Configurations::precision slope, const Configurations::precision intercept, const Configurations::precision _goodnessOfFit)
{
    regressionSlope = slope;
    regressionIntercept = intercept;
    goodnessOfFit = _goodnessOfFit;
}

bool CyclicErrorFilter::isStabilized() const
{
    return dataPointCount >= recordingBufferCapacity;
}

void CyclicErrorFilter::restart()
{
    if (recordedRawValue.empty() && rawOlsSeries.size() == 0)
    {
        return;
    }

    vector<unsigned long> clearRelativePosition;
    vector<Configurations::precision> clearAbsolutePosition;
    vector<Configurations::precision> clearRawValue;

    const auto optimalCapacity = std::min<unsigned int>(recordedRelativePosition.size(), maxAllocationCapacity);

    clearRelativePosition.reserve(optimalCapacity);
    clearAbsolutePosition.reserve(optimalCapacity);
    clearRawValue.reserve(optimalCapacity);

    recordedRelativePosition.swap(clearRelativePosition);
    recordedAbsolutePosition.swap(clearAbsolutePosition);
    recordedRawValue.swap(clearRawValue);

    rawOlsSeries.reset();
    cleanOlsSeries.reset();

    cursor = 0;
}

void CyclicErrorFilter::reset()
{
    restart();
    filterSum = numberOfSlots;
    weightCorrection = 1;
    dataPointCount = 0;
    regressionSlope = 0;
    regressionIntercept = 0;
    goodnessOfFit = 0;

    for (unsigned char i = 0; i < numberOfSlots; i++)
    {
        filterArray[i].reset();
        filterConfig[i] = 1;
        slotErrorTrackers[i].reset();
    }
}

void CyclicErrorFilter::updateFilter(const unsigned long position, const Configurations::precision rawValue, const Configurations::precision cleanValue)
{
    const auto slot = static_cast<unsigned char>(position % numberOfSlots);
    const auto correctionFactor = cleanValue / rawValue;

    // Clamp correctionFactor to reasonable range around current average
    const auto absoluteMaxDeviation = 0.02;
    const auto minCorrectionFactor = filterConfig[slot] * (1.0 - absoluteMaxDeviation);
    const auto maxCorrectionFactor = filterConfig[slot] * (1.0 + absoluteMaxDeviation);
    const auto clampedCorrectionFactor = std::clamp(correctionFactor, minCorrectionFactor, maxCorrectionFactor);

    const auto weightCorrectedCorrectionFactor = ((clampedCorrectionFactor - 1) * aggressiveness) + 1;

    const auto signedDev = (clampedCorrectionFactor - filterConfig[slot]) / filterConfig[slot];
    slotErrorTrackers[slot].push(signedDev);

    const auto boost = slotErrorTrackers[slot].calculateBoost();
    const auto weight = goodnessOfFit * boost;

    filterArray[slot].push(weightCorrectedCorrectionFactor, weight);

    filterSum -= filterConfig[slot];
    filterConfig[slot] = filterArray[slot].average();
    filterSum += filterConfig[slot];

    if (!isStabilized())
    {
        dataPointCount++;
    }

    if (filterSum != 0)
    {
        weightCorrection = numberOfSlots / filterSum;
    }
}

bool CyclicErrorFilter::isPotentiallyMisaligned()
{
    if (!isStabilized())
    {
        return false;
    }

    const auto rawR2 = rawOlsSeries.goodnessOfFit();
    const auto cleanR2 = cleanOlsSeries.goodnessOfFit();

    // If clean R² is significantly worse than raw R², filter is adding noise
    const Configurations::precision volatilityMargin = 0.8;
    if (cleanR2 >= rawR2 * volatilityMargin)
    {
        return false;
    }

    // Calculate misalignment severity: how much worse clean is compared to raw
    // Ratio of 1.0 means clean R² equals threshold (5% worse), higher means more misaligned
    const auto threshold = rawR2 * volatilityMargin;
    const auto misalignmentRatio = (threshold - cleanR2) / threshold;

    // Map misalignment to decay factor:
    const Configurations::precision maxDecay = 0.5;
    const Configurations::precision minDecay = 0.1;
    const auto decayFactor = maxDecay - (misalignmentRatio * (maxDecay - minDecay));

    for (auto &array : filterArray)
    {
        array.decay(decayFactor);
    }

    return true;
}