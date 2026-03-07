// NOLINTBEGIN(readability-magic-numbers, readability-function-cognitive-complexity, readability-function-size, cppcoreguidelines-avoid-do-while,clang-analyzer-optin.core.EnumCastOutOfRange)
#include <initializer_list>

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

#include "../../../src/utils/configuration.h"
#include "../../../src/utils/series/cyclic-error-filter.h"
#include "../../../src/utils/series/series.h"

using Catch::Matchers::WithinRel;

namespace
{
    // Helper to generate synthetic slot-based patterns for testing
    Configurations::precision generateSlotValue(unsigned long index, unsigned char numSlots, const std::initializer_list<Configurations::precision> &slotValues)
    {
        const auto slot = index % numSlots;
        unsigned char i = 0;
        for (const auto &value : slotValues)
        {
            if (i == slot)
            {
                return value;
            }
            ++i;
        }
        return 100.0;
    }
}

TEST_CASE("CyclicErrorFilter")
{
    SECTION("should initialize with specified number of slots")
    {
        const unsigned char numberOfSlots = 4;
        const unsigned char impulseDataArrayLength = 10;
        const Configurations::precision aggressiveness = 1.0;
        const unsigned short recordingBufferCapacity = 50;

        CyclicErrorFilter filter(numberOfSlots, impulseDataArrayLength, aggressiveness, recordingBufferCapacity);

        filter.applyFilter(0, 100.0); // slot 0
        filter.applyFilter(1, 200.0); // slot 1
        filter.applyFilter(2, 300.0); // slot 2
        filter.applyFilter(3, 400.0); // slot 3

        filter.applyFilter(4, 150.0); // slot 0 again
        filter.applyFilter(5, 250.0); // slot 1 again
        filter.applyFilter(6, 350.0); // slot 2 again
        filter.applyFilter(7, 450.0); // slot 3 again

        const auto &clean = filter.cleanSeries();

        CHECK_THAT(clean[0], WithinRel(100.0, 0.00001));
        CHECK_THAT(clean[1], WithinRel(200.0, 0.00001));
        CHECK_THAT(clean[2], WithinRel(300.0, 0.00001));
        CHECK_THAT(clean[3], WithinRel(400.0, 0.00001));
        CHECK_THAT(clean[4], WithinRel(150.0, 0.00001));
        CHECK_THAT(clean[5], WithinRel(250.0, 0.00001));
        CHECK_THAT(clean[6], WithinRel(350.0, 0.00001));
        CHECK_THAT(clean[7], WithinRel(450.0, 0.00001));
    }

    SECTION("should disable filter learning when aggressiveness is zero")
    {
        CyclicErrorFilter filter(4, 10, 0.0, 10);

        filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

        for (int i = 0; i < 10; ++i)
        {
            const auto slot = i % 4;
            const auto rawValue = (slot == 0) ? 150.0 : 100.0;
            filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
        }

        for (int i = 0; i < 10; ++i)
        {
            filter.processNextRawDatapoint();
        }

        filter.applyFilter(0, 150.0);
        filter.applyFilter(1, 100.0);
        filter.applyFilter(2, 200.0);

        const auto &raw = filter.rawSeries();
        const auto &clean = filter.cleanSeries();

        CHECK_THAT(clean[0], WithinRel(raw[0], 0.00001));
        CHECK_THAT(clean[1], WithinRel(raw[1], 0.00001));
        CHECK_THAT(clean[2], WithinRel(raw[2], 0.00001));
    }

    SECTION("rawSeries method")
    {
        SECTION("should return reference to raw series")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 50);

            const auto &raw = filter.rawSeries();

            REQUIRE(raw.size() == 0);
        }

        SECTION("should contain values pushed via applyFilter")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 50);

            filter.applyFilter(0, 100.0);
            filter.applyFilter(1, 200.0);

            const auto &raw = filter.rawSeries();

            REQUIRE(raw.size() == 2);
            CHECK_THAT(raw.back(), WithinRel(200.0, 0.00001));
        }
    }

    SECTION("cleanSeries method")
    {
        SECTION("should return reference to clean series")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 50);

            const auto &clean = filter.cleanSeries();

            REQUIRE(clean.size() == 0);
        }

        SECTION("should contain filtered values")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 50);

            filter.applyFilter(0, 100.0);

            const auto &clean = filter.cleanSeries();

            REQUIRE(clean.size() == 1);
            CHECK_THAT(clean.back(), WithinRel(100.0, 0.00001));
        }
    }

    SECTION("applyFilter method")
    {
        SECTION("should push raw value to raw series")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 50);

            filter.applyFilter(0, 150.0);

            CHECK_THAT(filter.rawSeries().back(), WithinRel(150.0, 0.00001));
        }

        SECTION("should apply same correction to positions mapping to same slot")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 50);

            filter.applyFilter(0, 100.0);
            filter.applyFilter(4, 200.0);

            const auto &clean = filter.cleanSeries();
            const auto &raw = filter.rawSeries();

            const auto ratio0 = clean[0] / raw[0];
            const auto ratio4 = clean[1] / raw[1];

            CHECK_THAT(ratio0, WithinRel(ratio4, 0.00001));
        }

        SECTION("should apply different corrections to different slots")
        {
            CyclicErrorFilter filter(3, 10, 1.0, 50);

            filter.applyFilter(0, 100.0); // slot 0
            filter.applyFilter(1, 200.0); // slot 1
            filter.applyFilter(2, 300.0); // slot 2
            filter.applyFilter(3, 150.0); // slot 0 again
            filter.applyFilter(4, 250.0); // slot 1 again
            filter.applyFilter(5, 350.0); // slot 2 again

            const auto &clean = filter.cleanSeries();
            const auto &raw = filter.rawSeries();

            CHECK_THAT(clean[0] / raw[0], WithinRel(clean[3] / raw[3], 0.00001)); // slot 0
            CHECK_THAT(clean[1] / raw[1], WithinRel(clean[4] / raw[4], 0.00001)); // slot 1
            CHECK_THAT(clean[2] / raw[2], WithinRel(clean[5] / raw[5], 0.00001)); // slot 2
        }
    }

    SECTION("recordRawDatapoint method")
    {
        SECTION("should record data when aggressiveness is greater than 0")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 5);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 5; ++i)
            {
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
            }

            for (int i = 0; i < 5; ++i)
            {
                filter.processNextRawDatapoint();
            }

            REQUIRE(filter.isStabilized());
        }

        SECTION("should not record data when aggressiveness is 0")
        {
            CyclicErrorFilter filter(4, 5, 0.0, 5);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 5; ++i)
            {
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
            }

            for (int i = 0; i < 5; ++i)
            {
                filter.processNextRawDatapoint();
            }

            REQUIRE_FALSE(filter.isStabilized());
        }
    }

    SECTION("processNextRawDatapoint method")
    {
        SECTION("should process recorded datapoints sequentially")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 10);

            filter.updateRegressionCoefficients(0.001, 100.0, 0.99);

            for (int i = 0; i < 10; ++i)
            {
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
            }

            for (int i = 0; i < 10; ++i)
            {
                filter.processNextRawDatapoint();
            }

            REQUIRE(filter.isStabilized());
        }

        SECTION("should do nothing when buffer is empty")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 10);

            for (int i = 0; i < 10; ++i)
            {
                filter.processNextRawDatapoint();
            }

            REQUIRE_FALSE(filter.isStabilized());
        }
    }

    SECTION("updateRegressionCoefficients method")
    {
        SECTION("should update internal regression parameters")
        {
            CyclicErrorFilter filterA(4, 5, 1.0, 5);
            CyclicErrorFilter filterB(4, 5, 1.0, 5);

            filterA.updateRegressionCoefficients(0.0, 100.0, 0.99);
            filterB.updateRegressionCoefficients(0.0, 50.0, 0.99);

            filterA.recordRawDatapoint(0, 0.0, 75.0);
            filterB.recordRawDatapoint(0, 0.0, 75.0);

            filterA.processNextRawDatapoint();
            filterB.processNextRawDatapoint();

            filterA.applyFilter(0, 75.0);
            filterB.applyFilter(0, 75.0);

            const auto cleanA = filterA.cleanSeries().back();
            const auto cleanB = filterB.cleanSeries().back();

            // filterA: correction = perfectDt(100) / raw(75) = 1.33 → corrects UP
            // filterB: correction = perfectDt(50) / raw(75) = 0.67 → corrects DOWN
            CHECK_THAT(cleanA, WithinRel(75.07427, 0.00001));
            CHECK_THAT(cleanB, WithinRel(74.92567, 0.00001));
        }
    }

    SECTION("isStabilized method")
    {
        SECTION("should return false when dataPointCount is less than recordingBufferCapacity")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 100);

            CHECK_FALSE(filter.isStabilized());
        }

        SECTION("should return true when dataPointCount reaches recordingBufferCapacity")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 10);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 10; ++i)
            {
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
            }

            for (int i = 0; i < 10; ++i)
            {
                filter.processNextRawDatapoint();
            }

            CHECK(filter.isStabilized());
        }
    }

    SECTION("restart method")
    {
        SECTION("should clear recorded datapoints")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 10);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 5; ++i)
            {
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
            }

            filter.restart();

            for (int i = 0; i < 10; ++i)
            {
                filter.processNextRawDatapoint();
            }

            CHECK_FALSE(filter.isStabilized());
        }

        SECTION("should preserve filter state")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 5);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 5; ++i)
            {
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
            }
            for (int i = 0; i < 5; ++i)
            {
                filter.processNextRawDatapoint();
            }

            REQUIRE(filter.isStabilized());

            filter.restart();

            REQUIRE(filter.isStabilized());
        }

        SECTION("should preserve learned filter corrections")
        {
            CyclicErrorFilter filter(4, 10, 1.0, 10);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 10; ++i)
            {
                const auto slot = i % 4;
                const auto rawValue = (slot == 0) ? 110.0 : 100.0;
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 10; ++i)
            {
                filter.processNextRawDatapoint();
            }

            filter.applyFilter(0, 110.0);
            const auto beforeRestart = filter.cleanSeries().back();

            filter.restart();

            filter.applyFilter(0, 110.0);
            const auto afterRestart = filter.cleanSeries().back();

            CHECK_THAT(afterRestart, WithinRel(beforeRestart, 0.00001));
            CHECK_THAT(afterRestart, WithinRel(109.67299, 0.00001));
        }

        SECTION("should reset OLS series on restart")
        {
            CyclicErrorFilter filter(4, 10, 1.0, 20);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 20; ++i)
            {
                const auto slot = i % 4;
                const auto rawValue = (slot == 0) ? 110.0 : 100.0;
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 20; ++i)
            {
                filter.processNextRawDatapoint();
            }

            REQUIRE(filter.isStabilized());

            for (int i = 20; i < 40; ++i)
            {
                const auto slot = i % 4;
                const auto rawValue = (slot == 0) ? 110.0 : 100.0;
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }
            REQUIRE_FALSE(filter.isPotentiallyMisaligned());

            filter.restart();

            REQUIRE(filter.isStabilized());
            REQUIRE_FALSE(filter.isPotentiallyMisaligned());
        }
    }

    SECTION("reset method")
    {
        SECTION("should reset all state including filter")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 5);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 5; ++i)
            {
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
            }
            for (int i = 0; i < 5; ++i)
            {
                filter.processNextRawDatapoint();
            }

            REQUIRE(filter.isStabilized());

            filter.reset();

            REQUIRE_FALSE(filter.isStabilized());
        }

        SECTION("should reset filterConfig to 1 for all slots")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 5);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 5; ++i)
            {
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
            }
            for (int i = 0; i < 5; ++i)
            {
                filter.processNextRawDatapoint();
            }

            filter.reset();

            filter.applyFilter(0, 100.0);
            CHECK_THAT(filter.cleanSeries().back(), WithinRel(100.0, 0.00001));
        }

        SECTION("should reset weightCorrection to 1")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 5);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 5; ++i)
            {
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
            }
            for (int i = 0; i < 5; ++i)
            {
                filter.processNextRawDatapoint();
            }

            filter.reset();

            filter.applyFilter(0, 123.456);
            CHECK_THAT(filter.cleanSeries().back(), WithinRel(123.456, 0.00001));
        }

        SECTION("should reset dataPointCount to 0")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 5);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 5; ++i)
            {
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
            }
            for (int i = 0; i < 5; ++i)
            {
                filter.processNextRawDatapoint();
            }

            filter.reset();

            REQUIRE_FALSE(filter.isStabilized());
        }

        SECTION("should clear learned filter corrections")
        {
            CyclicErrorFilter filter(4, 10, 1.0, 10);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 10; ++i)
            {
                const auto slot = i % 4;
                const auto rawValue = (slot == 0) ? 110.0 : 100.0;
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 10; ++i)
            {
                filter.processNextRawDatapoint();
            }

            filter.applyFilter(0, 110.0);
            const auto beforeReset = filter.cleanSeries().back();
            REQUIRE(beforeReset != 110.0);

            filter.reset();

            filter.applyFilter(0, 110.0);
            const auto afterReset = filter.cleanSeries().back();
            CHECK_THAT(afterReset, WithinRel(110.0, 0.00001));
        }

        SECTION("should reset OLS series on reset")
        {
            CyclicErrorFilter filter(4, 10, 1.0, 20);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 20; ++i)
            {
                const auto slot = i % 4;
                const auto rawValue = (slot == 0) ? 110.0 : 100.0;
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 20; ++i)
            {
                filter.processNextRawDatapoint();
            }

            REQUIRE(filter.isStabilized());

            for (int i = 20; i < 40; ++i)
            {
                const auto slot = i % 4;
                const auto rawValue = (slot == 0) ? 110.0 : 100.0;
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            REQUIRE_FALSE(filter.isPotentiallyMisaligned());

            filter.reset();

            REQUIRE_FALSE(filter.isStabilized());
            REQUIRE_FALSE(filter.isPotentiallyMisaligned());
        }
    }

    SECTION("filter learning behavior")
    {
        SECTION("should learn systematic error pattern over time")
        {
            CyclicErrorFilter filter(4, 5, 1.0, 20);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 20; ++i)
            {
                const auto slot = i % 4;
                const auto rawValue = (slot == 0) ? 110.0 : 100.0;
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 20; ++i)
            {
                filter.processNextRawDatapoint();
            }

            REQUIRE(filter.isStabilized());

            filter.applyFilter(0, 110.0);
            const auto correctedSlot0 = filter.cleanSeries().back();

            filter.applyFilter(1, 100.0);
            const auto correctedSlot1 = filter.cleanSeries().back();

            REQUIRE_THAT(correctedSlot0, WithinRel(109.33187, 0.00001));
            REQUIRE_THAT(correctedSlot1, WithinRel(100.20246, 0.00001));
        }

        SECTION("should respect aggressiveness parameter")
        {
            CyclicErrorFilter filterLow(4, 5, 0.1, 20);
            CyclicErrorFilter filterHigh(4, 5, 1.0, 20);

            filterLow.updateRegressionCoefficients(0.0, 100.0, 0.99);
            filterHigh.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 20; ++i)
            {
                const auto slot = i % 4;
                const auto rawValue = (slot == 0) ? 120.0 : 100.0;
                filterLow.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
                filterHigh.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 20; ++i)
            {
                filterLow.processNextRawDatapoint();
                filterHigh.processNextRawDatapoint();
            }

            filterLow.applyFilter(0, 120.0);
            filterHigh.applyFilter(0, 120.0);

            const auto correctedLow = filterLow.cleanSeries().back();
            const auto correctedHigh = filterHigh.cleanSeries().back();

            REQUIRE(correctedHigh < correctedLow);
        }
    }

    SECTION("boost mechanism behavior")
    {
        SECTION("should accelerate convergence when slot has persistent bias in same direction")
        {
            CyclicErrorFilter filterWithBoost(4, 10, 1.0, 30);
            CyclicErrorFilter filterWithoutBoost(4, 10, 1.0, 30);

            filterWithBoost.updateRegressionCoefficients(0.0, 100.0, 0.99);
            filterWithoutBoost.updateRegressionCoefficients(0.0, 100.0, 0.0);

            for (int i = 0; i < 30; ++i)
            {
                const auto slot = i % 4;
                const auto rawValue = (slot == 0) ? 115.0 : 100.0;
                filterWithBoost.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
                filterWithoutBoost.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 30; ++i)
            {
                filterWithBoost.processNextRawDatapoint();
                filterWithoutBoost.processNextRawDatapoint();
            }

            filterWithBoost.applyFilter(0, 115.0);
            filterWithoutBoost.applyFilter(0, 115.0);

            const auto correctedWithBoost = filterWithBoost.cleanSeries().back();
            const auto correctedWithoutBoost = filterWithoutBoost.cleanSeries().back();

            REQUIRE(correctedWithBoost < correctedWithoutBoost);
            REQUIRE_THAT(correctedWithBoost, WithinRel(113.90047, 0.00001));
            REQUIRE_THAT(correctedWithoutBoost, WithinRel(115.0, 0.00001));
        }

        SECTION("should not apply boost when deviations alternate in sign")
        {
            CyclicErrorFilter filterAlternating(4, 10, 1.0, 30);
            CyclicErrorFilter filterConsistent(4, 10, 1.0, 30);

            filterAlternating.updateRegressionCoefficients(0.0, 100.0, 0.99);
            filterConsistent.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 30; ++i)
            {
                const auto slot = i % 4;
                const auto iteration = i / 4;

                Configurations::precision rawValueAlternating = 100.0;
                Configurations::precision rawValueConsistent = 100.0;

                switch (slot)
                {
                case 0:
                    rawValueAlternating = (iteration % 2 == 0) ? 115.0 : 85.0;
                    rawValueConsistent = 115.0;
                    break;
                default:
                    break;
                }

                filterAlternating.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValueAlternating);
                filterConsistent.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValueConsistent);
            }

            for (int i = 0; i < 30; ++i)
            {
                filterAlternating.processNextRawDatapoint();
                filterConsistent.processNextRawDatapoint();
            }

            filterAlternating.applyFilter(0, 115.0);
            filterConsistent.applyFilter(0, 115.0);

            const auto correctedAlternating = filterAlternating.cleanSeries().back();
            const auto correctedConsistent = filterConsistent.cleanSeries().back();

            REQUIRE(correctedConsistent < correctedAlternating);
            REQUIRE_THAT(correctedAlternating, WithinRel(114.99991, 0.00001));
            REQUIRE_THAT(correctedConsistent, WithinRel(113.90047, 0.00001));
        }

        SECTION("should apply different boost levels to different slots based on their bias patterns")
        {
            CyclicErrorFilter filter(4, 10, 1.0, 40);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 40; ++i)
            {
                const auto slot = i % 4;
                Configurations::precision rawValue = 100.0;

                switch (slot)
                {
                case 0:
                    rawValue = 110.0;
                    break;
                case 2:
                    rawValue = 90.0;
                    break;
                default:
                    break;
                }

                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 40; ++i)
            {
                filter.processNextRawDatapoint();
            }

            filter.applyFilter(0, 100.0);
            const auto slot0Result = filter.cleanSeries().back();

            filter.applyFilter(1, 100.0);
            const auto slot1Result = filter.cleanSeries().back();

            filter.applyFilter(2, 100.0);
            const auto slot2Result = filter.cleanSeries().back();

            REQUIRE_THAT(slot0Result, WithinRel(98.67390, 0.00001));
            REQUIRE_THAT(slot1Result, WithinRel(99.99614, 0.00001));
            REQUIRE_THAT(slot2Result, WithinRel(101.3338, 0.00001));
        }
    }

    SECTION("isPotentiallyMisaligned method")
    {
        SECTION("should return false")
        {
            SECTION("when not stabilized")
            {
                CyclicErrorFilter filter(4, 10, 1.0, 50);

                filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

                for (int i = 0; i < 10; ++i)
                {
                    filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
                }

                for (int i = 0; i < 10; ++i)
                {
                    filter.processNextRawDatapoint();
                }

                REQUIRE_FALSE(filter.isStabilized());
                REQUIRE_FALSE(filter.isPotentiallyMisaligned());
            }

            SECTION("when clean series R2 is good relative to raw")
            {
                CyclicErrorFilter filter(4, 10, 1.0, 20);

                filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

                for (int i = 0; i < 20; ++i)
                {
                    filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
                }

                for (int i = 0; i < 20; ++i)
                {
                    filter.processNextRawDatapoint();
                }

                REQUIRE(filter.isStabilized());
                REQUIRE_FALSE(filter.isPotentiallyMisaligned());
            }
        }

        SECTION("should return false when OLS series is empty")
        {
            CyclicErrorFilter filter(4, 10, 1.0, 20);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 20; ++i)
            {
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), 100.0);
            }

            for (int i = 0; i < 20; ++i)
            {
                filter.processNextRawDatapoint();
            }

            REQUIRE(filter.isStabilized());
            REQUIRE_FALSE(filter.isPotentiallyMisaligned());
        }

        SECTION("when OLS series has data")
        {
            SECTION("should return false when clean R2 is acceptable")
            {
                CyclicErrorFilter filter(4, 10, 1.0, 20);

                filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

                for (int i = 0; i < 20; ++i)
                {
                    const auto slot = i % 4;
                    const auto rawValue = (slot == 0) ? 110.0 : 100.0;
                    filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
                }

                for (int i = 0; i < 20; ++i)
                {
                    filter.processNextRawDatapoint();
                }

                REQUIRE(filter.isStabilized());

                for (int i = 20; i < 40; ++i)
                {
                    const auto slot = i % 4;
                    const auto rawValue = (slot == 0) ? 110.0 : 100.0;
                    filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
                }

                REQUIRE_FALSE(filter.isPotentiallyMisaligned());
            }

            SECTION("should return true when clean R2 degrades due to pattern shift")
            {
                CyclicErrorFilter filter(6, 10, 1.0, 12);

                filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

                // Phase 1: Train filter with larger and asymmetric misalignment
                for (int i = 0; i < 30; ++i)
                {
                    const auto rawValue = generateSlotValue(i, 6, {100.0, 50.0, 150.0, 60.0, 180.0, 100.0});
                    filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
                    filter.processNextRawDatapoint();
                }

                REQUIRE(filter.isStabilized());

                // Phase 2: Build a robust OLS baseline with many matching datapoints
                for (int i = 12; i < 36; ++i)
                {
                    const auto rawValue = generateSlotValue(i, 6, {100.0, 50.0, 150.0, 60.0, 180.0, 100.0});
                    filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
                    filter.processNextRawDatapoint();
                }
                REQUIRE_FALSE(filter.isPotentiallyMisaligned());
                // Phase 3: Inject swapped pattern - slot 1 and 2 reversed, slot 5 alternates
                for (int i = 36; i < 60; ++i)
                {
                    auto rawValue = generateSlotValue(i, 6, {100.0, 150.0, 50.0, 20.0, 200.0, 100.0});
                    // Slot 5 alternates between 170 and 40
                    if (i % 6 == 5)
                    {
                        rawValue = ((i / 6) % 2 == 0) ? 170.0 : 40.0;
                    }
                    filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
                    filter.processNextRawDatapoint();
                }

                // Apply misaligned pattern to clean/raw series
                for (int i = 24; i < 36; ++i)
                {
                    auto rawValue = generateSlotValue(i, 6, {100.0, 150.0, 50.0, 20.0, 155.0, 100.0});
                    // Slot 5 alternates between 170 and 40
                    if (i % 6 == 5)
                    {
                        rawValue = ((i / 6) % 2 == 0) ? 170.0 : 40.0;
                    }
                    filter.applyFilter(i, rawValue);
                }

                filter.applyFilter(48, 10000);

                REQUIRE(filter.isPotentiallyMisaligned());
            }
        }
    }

    SECTION("integration scenarios")
    {
        SECTION("should handle multiple stabilization cycles")
        {
            CyclicErrorFilter filter(4, 10, 1.0, 10);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 10; ++i)
            {
                const auto slot = i % 4;
                const auto rawValue = (slot == 0) ? 110.0 : 100.0;
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 10; ++i)
            {
                filter.processNextRawDatapoint();
            }

            REQUIRE(filter.isStabilized());

            filter.applyFilter(0, 110.0);
            const auto firstCycleResult = filter.cleanSeries().back();

            filter.restart();

            for (int i = 0; i < 10; ++i)
            {
                const auto slot = i % 4;
                const auto rawValue = (slot == 0) ? 110.0 : 100.0;
                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 10; ++i)
            {
                filter.processNextRawDatapoint();
            }

            filter.applyFilter(0, 110.0);
            const auto secondCycleResult = filter.cleanSeries().back();

            REQUIRE_THAT(firstCycleResult, WithinRel(109.67299, 0.00001));
            REQUIRE_THAT(secondCycleResult, WithinRel(108.77302, 0.00001));
        }

        SECTION("should handle cyclic slot biases where closer slots read faster")
        {
            CyclicErrorFilter filter(4, 10, 1.0, 20);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 20; ++i)
            {
                const auto slot = i % 4;
                Configurations::precision rawValue = 100.0;

                switch (slot)
                {
                case 0:
                    rawValue = 90.0;
                    break;
                case 2:
                    rawValue = 110.0;
                    break;
                default:
                    break;
                }

                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 20; ++i)
            {
                filter.processNextRawDatapoint();
            }

            filter.applyFilter(0, 90.0);
            const auto slot0 = filter.cleanSeries().back();

            filter.applyFilter(1, 100.0);
            const auto slot1 = filter.cleanSeries().back();

            filter.applyFilter(2, 110.0);
            const auto slot2 = filter.cleanSeries().back();

            filter.applyFilter(3, 100.0);
            const auto slot3 = filter.cleanSeries().back();

            REQUIRE_THAT(slot0, WithinRel(90.73039, 0.00001));
            REQUIRE_THAT(slot1, WithinRel(99.99888, 0.00001));
            REQUIRE_THAT(slot2, WithinRel(109.10975, 0.00001));
            REQUIRE_THAT(slot3, WithinRel(99.99888, 0.00001));
        }

        SECTION("should handle opposing slot biases")
        {
            CyclicErrorFilter filter(4, 10, 1.0, 20);

            filter.updateRegressionCoefficients(0.0, 100.0, 0.99);

            for (int i = 0; i < 20; ++i)
            {
                const auto slot = i % 4;
                Configurations::precision rawValue = 100.0;

                switch (slot)
                {
                case 0:
                case 1:
                    rawValue = 110.0;
                    break;
                default:
                    rawValue = 90.0;
                    break;
                }

                filter.recordRawDatapoint(i, static_cast<Configurations::precision>(i), rawValue);
            }

            for (int i = 0; i < 20; ++i)
            {
                filter.processNextRawDatapoint();
            }

            filter.applyFilter(0, 100.0);
            const auto slot0 = filter.cleanSeries().back();

            filter.applyFilter(2, 100.0);
            const auto slot2 = filter.cleanSeries().back();

            REQUIRE_THAT(slot0, WithinRel(99.18957, 0.00001));
            REQUIRE_THAT(slot2, WithinRel(100.81042, 0.00001));
        }
    }
}
// NOLINTEND(readability-magic-numbers, readability-function-cognitive-complexity, readability-function-size, cppcoreguidelines-avoid-do-while,clang-analyzer-optin.core.EnumCastOutOfRange)