// NOLINTBEGIN(readability-magic-numbers)
#include "catch2/catch_test_macros.hpp"

#include "../../../src/utils/configuration.h"
#include "../../../src/utils/series/series.h"

TEST_CASE("Series")
{
    SECTION("when maxSeriesLength is provided should initialize with capacity of maxSeriesLength")
    {
        const auto maxSeriesLength = 10;
        Series series(maxSeriesLength);
        REQUIRE(series.capacity() == maxSeriesLength);
    }
    SECTION("when maxSeriesLength is not provided")
    {
        const auto maxCapacity = 500U;
        Series series(0, Configurations::defaultAllocationCapacity, maxCapacity);

        SECTION("should initialize with capacity of Configurations::defaultAllocationCapacity")
        {
            REQUIRE(series.capacity() == Configurations::defaultAllocationCapacity);
        }

        SECTION("should cap maxCapacity at 1000")
        {
            const auto extremeHighMaxCapacity = 1'200;
            Series seriesMaxCapacity(0, Configurations::defaultAllocationCapacity, extremeHighMaxCapacity);

            for (auto i = 0U; i < 999; ++i)
            {
                seriesMaxCapacity.push(0.1);
            }

            REQUIRE(seriesMaxCapacity.capacity() == 1'000);
        }

        SECTION("should set maxCapacity to maxAllocationCapacity when that is below 1000")
        {
            for (auto i = 0U; i < maxCapacity - 1; ++i)
            {
                series.push(0.1);
            }
            REQUIRE(series.capacity() == maxCapacity);
        }

        SECTION("should use default allocator (i.e. double) when new capacity would be below maxCapacity")
        {
            const auto initialCapacity = (unsigned int)series.capacity();
            for (auto i = 0U; i < initialCapacity + 1; ++i)
            {
                series.push(0.1);
            }
            REQUIRE(series.capacity() == initialCapacity * 2UL);
        }

        SECTION("should set new capacity to maxCapacity when default allocation would yield higher capacity than maxCapacity")
        {
            unsigned int capacityStep = Configurations::defaultAllocationCapacity;

            while (capacityStep <= maxCapacity / 2)
            {
                capacityStep *= 2;
            }

            for (auto i = 0U; i < capacityStep + 1; ++i)
            {
                series.push(0.1);
            }
            REQUIRE(series.capacity() == maxCapacity);
        }

        SECTION("should increase capacity by 10 when required new capacity exceeds maxCapacity")
        {
            for (auto i = 0U; i < maxCapacity + 1; ++i)
            {
                series.push(0.1);
            }
            REQUIRE(series.capacity() == maxCapacity + 10);
        }
    }
}
// NOLINTEND(readability-magic-numbers)