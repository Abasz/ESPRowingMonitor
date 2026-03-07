// NOLINTBEGIN(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while)

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

#include "../../../src/utils/series/exponential-weighted-average.h"

using Catch::Matchers::WithinRel;

TEST_CASE("ExponentialWeightedAverage")
{
    SECTION("should initialize with default parameters")
    {
        const ExponentialWeightedAverage avg;

        REQUIRE(avg.average() == 0.0);
    }

    SECTION("should calculate decay factor based on window size")
    {
        // After push(100, 1): weightedSum = 100, totalWeight = 1
        // After push(50, 1): weightedSum = 100 * 0.9 + 50 = 140, totalWeight = 0.9 + 1 = 1.9
        ExponentialWeightedAverage avgWindow10(10, 0);
        avgWindow10.push(100.0, 1.0);
        avgWindow10.push(50.0, 1.0);

        CHECK_THAT(avgWindow10.average(), WithinRel(73.6842105, 0.00001));

        // After push(100, 1): weightedSum = 100, totalWeight = 1
        // After push(50, 1): weightedSum = 100 * 0.8 + 50 = 130, totalWeight = 0.8 + 1 = 1.8
        ExponentialWeightedAverage avgWindow5(5, 0);
        avgWindow5.push(100.0, 1.0);
        avgWindow5.push(50.0, 1.0);

        CHECK_THAT(avgWindow5.average(), WithinRel(72.2222222, 0.00001));
    }

    SECTION("should use initial buffer as starting weight and sum")
    {
        // Initial average = 50 / 50 = 1.0
        // windowSize = 10: decayFactor = 0.9
        // After push(100, 1): weightedSum = 50 * 0.9 + 100 = 145, totalWeight = 50 * 0.9 + 1 = 46
        ExponentialWeightedAverage avg(10, 50);

        CHECK_THAT(avg.average(), WithinRel(1.0, 0.00001));

        avg.push(100.0, 1.0);

        CHECK_THAT(avg.average(), WithinRel(3.152174, 0.00001));
    }

    SECTION("push method")
    {
        SECTION("should add single value correctly")
        {
            ExponentialWeightedAverage avg(50, 0);
            avg.push(100.0, 1.0);

            CHECK_THAT(avg.average(), WithinRel(100.0, 0.00001));
        }

        SECTION("should weight values by provided weight")
        {
            ExponentialWeightedAverage avg(50, 0);

            avg.push(100.0, 2.0);

            // weightedSum = 100 * 2 = 200
            // totalWeight = 2
            // average = 200 / 2 = 100
            CHECK_THAT(avg.average(), WithinRel(100.0, 0.00001));
        }

        SECTION("should apply exponential decay to previous values")
        {
            ExponentialWeightedAverage avg(10, 0);

            avg.push(100.0, 1.0);
            avg.push(50.0, 1.0);

            // weightedSum = 100 * 0.9 + 50 = 140
            // totalWeight = 1 * 0.9 + 1 = 1.9
            CHECK_THAT(avg.average(), WithinRel(73.6842105, 0.00001));
        }

        SECTION("should handle zero weight")
        {
            ExponentialWeightedAverage avg(50, 0);

            avg.push(100.0, 0.0);

            CHECK(avg.average() == 0.0);
        }

        SECTION("should handle multiple pushes with varying weights")
        {
            ExponentialWeightedAverage avg(10, 0);

            avg.push(100.0, 1.0);
            avg.push(200.0, 0.5);
            avg.push(50.0, 2.0);

            // After push(200, 0.5): weightedSum = 90 + 100 = 190, totalWeight = 0.9 + 0.5 = 1.4
            // After push(50, 2): weightedSum = 171 + 100 = 271, totalWeight = 1.26 + 2 = 3.26
            CHECK_THAT(avg.average(), WithinRel(83.128834, 0.00001));
        }

        SECTION("should handle negative values")
        {
            ExponentialWeightedAverage avg(50, 0);

            avg.push(-50.0, 1.0);

            CHECK_THAT(avg.average(), WithinRel(-50.0, 0.00001));
        }
    }

    SECTION("average method")
    {
        SECTION("should return 0 when totalWeight is 0")
        {
            const ExponentialWeightedAverage avg(50, 0);

            CHECK(avg.average() == 0.0);
        }

        SECTION("should return correct weighted average")
        {
            ExponentialWeightedAverage avg(50, 0);

            avg.push(100.0, 1.0);
            avg.push(100.0, 1.0);
            avg.push(100.0, 1.0);

            CHECK_THAT(avg.average(), WithinRel(100.0, 0.00001));
        }
    }

    SECTION("decay method")
    {
        SECTION("should apply custom decay factor")
        {
            ExponentialWeightedAverage avg(50, 0);

            avg.push(100.0, 1.0);
            const auto beforeDecay = avg.average();

            avg.decay(0.5);

            // weightedSum = 100 * 0.5 = 50
            // totalWeight = 1 * 0.5 = 0.5
            // average = 50 / 0.5 = 100 (same ratio preserved)
            const auto afterDecay = avg.average();

            CHECK_THAT(beforeDecay, WithinRel(100.0, 0.00001));
            CHECK_THAT(afterDecay, WithinRel(100.0, 0.00001));
        }

        SECTION("should reduce influence of accumulated values on subsequent pushes")
        {
            ExponentialWeightedAverage avg(50, 0);

            avg.push(100.0, 1.0);
            avg.decay(0.1);

            avg.push(50.0, 1.0);

            // After decay: weightedSum = 10, totalWeight = 0.1
            // After push: weightedSum = 10 * 0.98 + 50 = 59.8, totalWeight = 0.098 + 1 = 1.098
            CHECK_THAT(avg.average(), WithinRel(54.4626, 0.00001));
        }

        SECTION("should handle decay factor of 0")
        {
            ExponentialWeightedAverage avg(50, 0);

            avg.push(100.0, 1.0);
            avg.decay(0.0);

            REQUIRE(avg.average() == 0.0);
        }
    }

    SECTION("reset method")
    {
        SECTION("should reset to initial buffer values")
        {
            ExponentialWeightedAverage avg(50, 100);

            avg.push(500.0, 1.0);
            avg.push(500.0, 1.0);

            avg.reset();

            CHECK_THAT(avg.average(), WithinRel(1.0, 0.00001));
        }

        SECTION("should reset to zero when initial buffer is 0")
        {
            ExponentialWeightedAverage avg(50, 0);

            avg.push(100.0, 1.0);
            avg.push(200.0, 1.0);

            avg.reset();

            REQUIRE(avg.average() == 0.0);
        }

        SECTION("should allow new pushes after reset")
        {
            ExponentialWeightedAverage avg(50, 0);

            avg.push(100.0, 1.0);
            avg.reset();
            avg.push(50.0, 1.0);

            CHECK_THAT(avg.average(), WithinRel(50.0, 0.00001));
        }
    }

    SECTION("edge cases")
    {
        SECTION("should handle very small window size")
        {
            ExponentialWeightedAverage avg(1, 0); // decayFactor = 0

            avg.push(100.0, 1.0);
            avg.push(50.0, 1.0);

            // With decayFactor = 0, only the last value matters
            CHECK_THAT(avg.average(), WithinRel(50.0, 0.00001));
        }

        SECTION("should handle very large window size")
        {
            ExponentialWeightedAverage avg(1000, 0);

            avg.push(100.0, 1.0);
            avg.push(200.0, 1.0);

            CHECK_THAT(avg.average(), WithinRel(150.02501, 0.00001));
        }

        SECTION("should handle very small values")
        {
            ExponentialWeightedAverage avg(50, 0);

            avg.push(0.000001, 1.0);

            CHECK_THAT(avg.average(), WithinRel(0.000001, 0.00001));
        }

        SECTION("should handle very large values")
        {
            ExponentialWeightedAverage avg(50, 0);

            avg.push(1e12, 1.0);

            CHECK_THAT(avg.average(), WithinRel(1e12, 0.00001));
        }
    }
}
// NOLINTEND(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while)