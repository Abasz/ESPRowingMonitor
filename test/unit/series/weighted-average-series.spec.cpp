// NOLINTBEGIN(readability-magic-numbers)
#include "../include/catch_amalgamated.hpp"

#include "../../../src/utils/configuration.h"
#include "../../../src/utils/series/weighted-average-series.h"

TEST_CASE("WeightedAverageSeries")
{
    SECTION("when maxSeriesLength is provided should initialize with capacity of maxSeriesLength")
    {
        const auto maxSeriesLength = 10;
        WeightedAverageSeries weightedAverageSeries(maxSeriesLength);
        REQUIRE(weightedAverageSeries.capacity() == maxSeriesLength);
    }

    SECTION("when maxSeriesLength is not provided")
    {
        const auto maxCapacity = 500U;
        WeightedAverageSeries weightedAverageSeries(0, maxCapacity);

        SECTION("should initialize with capacity of Configurations::minimumRecoveryTime / Configurations::rotationDebounceTimeMin")
        {
            REQUIRE(weightedAverageSeries.capacity() == Configurations::minimumRecoveryTime / Configurations::rotationDebounceTimeMin);
        }

        SECTION("should cap maxCapacity at 1000")
        {
            const auto extremeHighMaxCapacity = 1'200;
            WeightedAverageSeries weightedAverageSeriesMaxCapacity(0, extremeHighMaxCapacity);

            for (auto i = 0U; i < 999; i++)
            {
                weightedAverageSeriesMaxCapacity.push(0.1, 0.1);
            }

            REQUIRE(weightedAverageSeriesMaxCapacity.capacity() == 1'000);
        }

        SECTION("should set maxCapacity to maxAllocationCapacity when that is below 1000")
        {
            for (auto i = 0U; i < maxCapacity - 1; i++)
            {
                weightedAverageSeries.push(0.1, 0.1);
            }
            REQUIRE(weightedAverageSeries.capacity() == maxCapacity);
        }

        SECTION("should use default allocator (i.e. double) when new capacity would be below maxCapacity")
        {
            const auto initialCapacity = weightedAverageSeries.capacity();
            for (auto i = 0U; i < initialCapacity + 1; i++)
            {
                weightedAverageSeries.push(0.1, 0.1);
            }
            REQUIRE(weightedAverageSeries.capacity() == initialCapacity * 2);
        }

        SECTION("should set new capacity to maxCapacity when default allocation would yield higher capacity than maxCapacity")
        {
            unsigned int capacityStep = Configurations::minimumRecoveryTime / Configurations::rotationDebounceTimeMin;

            while (capacityStep <= maxCapacity / 2)
            {
                capacityStep *= 2;
            }

            for (auto i = 0U; i < capacityStep + 1; i++)
            {
                weightedAverageSeries.push(0.1, 0.1);
            }
            REQUIRE(weightedAverageSeries.capacity() == maxCapacity);
        }

        SECTION("should increase capacity by 10 when required new capacity exceeds maxCapacity")
        {
            for (auto i = 0U; i < maxCapacity + 1; i++)
            {
                weightedAverageSeries.push(0.1, 0.1);
            }
            REQUIRE(weightedAverageSeries.capacity() == maxCapacity + 10);
        }
    }

    SECTION("average method should calculate the weighted average correctly")
    {
        SECTION("should return 0 if there are no elements")
        {
            WeightedAverageSeries weightedAverageSeries;

            REQUIRE(weightedAverageSeries.average() == 0.0);
        }

        SECTION("should return the correct weighted average for a series of elements")
        {
            WeightedAverageSeries weightedAverageSeries;

            weightedAverageSeries.push(1.0, 1.0);
            weightedAverageSeries.push(2.0, 2.0);
            weightedAverageSeries.push(3.0, 3.0);

            REQUIRE_THAT(weightedAverageSeries.average(), Catch::Matchers::WithinRel((1.0 * 1.0 + 2.0 * 2.0 + 3.0 * 3.0) / (1.0 + 2.0 + 3.0), 0.0000001));
        }

        SECTION("reset method should clear all elements")
        {
            WeightedAverageSeries weightedAverageSeries;

            weightedAverageSeries.push(1.0, 1.0);
            weightedAverageSeries.push(2.0, 2.0);

            weightedAverageSeries.reset();

            REQUIRE(weightedAverageSeries.size() == 0);
            REQUIRE(weightedAverageSeries.average() == 0.0);
        }

        SECTION("push method should add elements to both series")
        {
            WeightedAverageSeries weightedAverageSeries;

            weightedAverageSeries.push(1.0, 1.0);
            weightedAverageSeries.push(2.0, 2.0);

            REQUIRE(weightedAverageSeries.size() == 2);
            REQUIRE(weightedAverageSeries.capacity() >= 2);
        }
    }
}
// NOLINTEND(readability-magic-numbers)