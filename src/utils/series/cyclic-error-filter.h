#pragma once

#include <algorithm>
#include <array>
#include <vector>

#include "../configuration.h"
#include "./exponential-weighted-average.h"
#include "./ols-linear-series.h"
#include "./series.h"

using std::vector;

class CyclicErrorFilter
{
    class SlotErrorTracker
    {
    private:
        static constexpr unsigned char bufferSize = 5;

        static constexpr Configurations::precision medianSaturation = 0.004;
        static constexpr Configurations::precision maxBoost = 5.0;
        static constexpr Configurations::precision medianThreshold = 0.001;
        static constexpr Configurations::precision signThreshold = 1;

        std::array<Configurations::precision, bufferSize> buffer{};

        unsigned char count = 0;
        unsigned char head = 0;
        signed char signSum = 0;

    public:
        void push(Configurations::precision deviation);
        [[nodiscard]] Configurations::precision median() const;
        [[nodiscard]] Configurations::precision meanSign() const;
        [[nodiscard]] Configurations::precision calculateBoost() const;
        void reset();
    };

    unsigned short maxAllocationCapacity;
    unsigned short recordingBufferCapacity;

    unsigned char numberOfSlots;
    Configurations::precision aggressiveness;

    Configurations::precision regressionSlope = 0;
    Configurations::precision regressionIntercept = 0;
    Configurations::precision goodnessOfFit = 0;

    vector<ExponentialWeightedAverage> filterArray;
    vector<Configurations::precision> filterConfig;
    vector<SlotErrorTracker> slotErrorTrackers;

    vector<unsigned long> recordedRelativePosition;
    vector<Configurations::precision> recordedAbsolutePosition;
    vector<Configurations::precision> recordedRawValue;

    Series raw;
    Series clean;
    OLSLinearSeries rawOlsSeries;
    OLSLinearSeries cleanOlsSeries;

    unsigned int cursor = 0;
    Configurations::precision filterSum = 0;
    Configurations::precision weightCorrection = 1;
    unsigned short dataPointCount = 0;

    void updateFilter(unsigned long position, Configurations::precision rawValue, Configurations::precision cleanValue);

public:
    constexpr explicit CyclicErrorFilter(
        const unsigned char _numberOfSlots,
        const unsigned char _impulseDataArrayLength,
        const Configurations::precision _aggressiveness,
        const unsigned short _recordingBufferCapacity,
        const unsigned short _maxAllocationCapacity = 1'000)
        : maxAllocationCapacity(_maxAllocationCapacity),
          recordingBufferCapacity(_recordingBufferCapacity),
          numberOfSlots(_numberOfSlots),
          aggressiveness(_aggressiveness),
          raw(_impulseDataArrayLength),
          clean(_impulseDataArrayLength),
          rawOlsSeries(0, _recordingBufferCapacity, _maxAllocationCapacity),
          cleanOlsSeries(0, _recordingBufferCapacity, _maxAllocationCapacity),
          filterSum(_numberOfSlots)
    {
        filterArray.reserve(_numberOfSlots);
        filterConfig.reserve(_numberOfSlots);
        slotErrorTrackers.reserve(_numberOfSlots);
        recordedRelativePosition.reserve(_recordingBufferCapacity);
        recordedAbsolutePosition.reserve(_recordingBufferCapacity);
        recordedRawValue.reserve(_recordingBufferCapacity);

        for (unsigned char i = 0; i < _numberOfSlots; i++)
        {
            const auto capacity = std::max<unsigned short>(15U, std::min<unsigned short>(_recordingBufferCapacity, 50U));
            filterArray.emplace_back(capacity, capacity);
            filterConfig.push_back(1);
            slotErrorTrackers.emplace_back();
        }
    }

    [[nodiscard]] const Series &rawSeries() const;
    [[nodiscard]] const Series &cleanSeries() const;

    void applyFilter(unsigned long position, Configurations::precision rawValue);
    void recordRawDatapoint(unsigned long relativePosition, Configurations::precision absolutePosition, Configurations::precision rawValue);
    void processNextRawDatapoint();
    void updateRegressionCoefficients(Configurations::precision slope, Configurations::precision intercept, Configurations::precision _goodnessOfFit);
    [[nodiscard]] bool isPotentiallyMisaligned();
    [[nodiscard]] bool isStabilized() const;
    void restart();
    void reset();
};
