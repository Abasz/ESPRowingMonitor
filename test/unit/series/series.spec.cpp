// NOLINTBEGIN(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while)
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

#include "../../../src/utils/configuration.h"
#include "../../../src/utils/series/series.h"

TEST_CASE("Series")
{
    SECTION("operator[] should return value at given index")
    {
        Series series(5);
        series.push(1.5);
        series.push(2.5);
        series.push(3.5);

        REQUIRE(series[0] == 1.5);
        REQUIRE(series[1] == 2.5);
        REQUIRE(series[2] == 3.5);
    }

    SECTION("front method should return first element")
    {
        Series series(5);
        series.push(10.0);
        series.push(20.0);
        series.push(30.0);

        REQUIRE(series.front() == 10.0);
    }

    SECTION("back method should return last element")
    {
        Series series(5);
        series.push(10.0);
        series.push(20.0);
        series.push(30.0);

        REQUIRE(series.back() == 30.0);
    }

    SECTION("size method should return number of elements")
    {
        Series series(10);

        REQUIRE(series.size() == 0);

        series.push(1.0);
        REQUIRE(series.size() == 1);

        series.push(2.0);
        series.push(3.0);
        REQUIRE(series.size() == 3);
    }

    SECTION("average method")
    {
        SECTION("should return 0 for empty series")
        {
            Series series(5);
            REQUIRE(series.average() == 0.0);
        }

        SECTION("should return correct average for populated series")
        {
            Series series(5);
            series.push(10.0);
            series.push(20.0);
            series.push(30.0);

            CHECK_THAT(series.average(), Catch::Matchers::WithinRel(20.0, 0.00001));
        }
    }

    SECTION("median method")
    {
        SECTION("should return 0 for empty series")
        {
            Series series(5);
            REQUIRE(series.median() == 0.0);
        }

        SECTION("should return correct median for odd number of elements")
        {
            Series series(5);
            series.push(3.0);
            series.push(1.0);
            series.push(2.0);

            REQUIRE(series.median() == 2.0);
        }

        SECTION("should return correct median for even number of elements")
        {
            Series series(5);
            series.push(4.0);
            series.push(1.0);
            series.push(3.0);
            series.push(2.0);

            CHECK_THAT(series.median(), Catch::Matchers::WithinRel(2.5, 0.00001));
        }
    }

    SECTION("sum method")
    {
        SECTION("should return 0 for empty series")
        {
            Series series(5);
            REQUIRE(series.sum() == 0.0);
        }

        SECTION("should return correct sum for populated series")
        {
            Series series(5);
            series.push(10.0);
            series.push(20.0);
            series.push(30.0);

            REQUIRE(series.sum() == 60.0);
        }
    }

    SECTION("reset method should clear the series")
    {
        Series series(5);
        series.push(10.0);
        series.push(20.0);
        series.push(30.0);

        series.reset();

        REQUIRE(series.size() == 0);
        REQUIRE(series.sum() == 0.0);
        REQUIRE(series.average() == 0.0);
    }

    SECTION("when maxSeriesLength is")
    {
        SECTION("exceeded should roll window")
        {
            const auto maxSeriesLength = 3;
            Series series(maxSeriesLength);

            series.push(1.0);
            series.push(2.0);
            series.push(3.0);

            REQUIRE(series.size() == 3);
            REQUIRE(series.sum() == 6.0);
            REQUIRE(series.front() == 1.0);

            series.push(4.0);

            REQUIRE(series.size() == 3);
            REQUIRE(series.sum() == 9.0);
            REQUIRE(series.front() == 2.0);
            REQUIRE(series.back() == 4.0);

            series.push(5.0);

            REQUIRE(series.size() == 3);
            REQUIRE(series.sum() == 12.0);
            REQUIRE(series.front() == 3.0);
            REQUIRE(series.back() == 5.0);
        }

        SECTION("provided should initialize with capacity of maxSeriesLength")
        {
            const auto maxSeriesLength = 10;
            Series series(maxSeriesLength);
            REQUIRE(series.capacity() == maxSeriesLength);
        }

        SECTION("not provided")
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
}
// NOLINTEND(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while)