// NOLINTBEGIN(readability-magic-numbers)
#include <numeric>
#include <vector>

#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "../include/Arduino.h"

#include "../include/globals.h"

#include "../../../src/rower/flywheel.service.h"

using namespace fakeit;

const std::vector<unsigned long> deltaTimes = {
    RowerProfile::Defaults::rotationDebounceTimeMin + 2000,
    RowerProfile::Defaults::rotationDebounceTimeMin + 1000,
    RowerProfile::Defaults::rotationDebounceTimeMin + 500,
    RowerProfile::Defaults::rotationDebounceTimeMin + 100,
};

TEST_CASE("FlywheelService", "[rower]")
{
    mockGlobals.Reset();
    mockArduino.Reset();

    Fake(Method(mockGlobals, detachRotationInterrupt));
    Fake(Method(mockGlobals, attachRotationInterrupt));

    SECTION("setup method should setup interrupts")
    {
        Fake(Method(mockArduino, pinMode));

        FlywheelService flywheelService;

        flywheelService.setup(RowerProfile::MachineSettings{}, RowerProfile::SensorSignalSettings{});

        Verify(Method(mockGlobals, attachRotationInterrupt)).Once();
        Verify(Method(mockArduino, pinMode).Using(Configurations::sensorPinNumber, INPUT_PULLUP)).Once();
    }

    SECTION("processRotation method")
    {
        SECTION("should indicate data change via hasDataChanged method on new measurement")
        {
            FlywheelService flywheelService;

            flywheelService.processRotation(RowerProfile::Defaults::rotationDebounceTimeMin + 1000);

            REQUIRE(flywheelService.hasDataChanged());
        }

        SECTION("should ignore deltaTimes below the debounce time")
        {
            FlywheelService flywheelService;

            flywheelService.processRotation(RowerProfile::Defaults::rotationDebounceTimeMin - 1000);

            REQUIRE(flywheelService.hasDataChanged() == false);
        }

        SECTION("should disable and then reenable interrupts when reading data")
        {
            FlywheelService flywheelService;

            flywheelService.getData();

            Verify(Method(mockGlobals, detachRotationInterrupt)).Once();
            Verify(Method(mockGlobals, attachRotationInterrupt)).Once();
        }

        SECTION("should update data based on new valid measurement")
        {
            RowingDataModels::FlywheelData expected{
                .rawImpulseCount = static_cast<unsigned long>(deltaTimes.size()),
                .deltaTime = deltaTimes.back(),
                .totalTime = std::accumulate(cbegin(deltaTimes), cend(deltaTimes), 0UL),
                .totalAngularDisplacement = (2 * PI) / RowerProfile::Defaults::impulsesPerRevolution * (double)deltaTimes.size(),
                .cleanImpulseTime = std::accumulate(cbegin(deltaTimes), cend(deltaTimes), 0UL),
                .rawImpulseTime = std::accumulate(cbegin(deltaTimes), cend(deltaTimes), 0UL),
            };

            FlywheelService flywheelService;

            auto now = 0UL;

            for (const auto &testCase : deltaTimes)
            {
                now += testCase;
                flywheelService.processRotation(now);
            }

            const auto result = flywheelService.getData();

            REQUIRE(result.rawImpulseCount == expected.rawImpulseCount);
            REQUIRE(result.deltaTime == expected.deltaTime);
            REQUIRE(result.totalTime == expected.totalTime);
            REQUIRE(result.totalAngularDisplacement == expected.totalAngularDisplacement);
            REQUIRE(result.cleanImpulseTime == expected.cleanImpulseTime);
            REQUIRE(result.rawImpulseTime == expected.rawImpulseTime);
        }

        SECTION("should reset data changed indicator after reading the new data")
        {
            FlywheelService flywheelService;

            flywheelService.processRotation(RowerProfile::Defaults::rotationDebounceTimeMin + 1000);

            REQUIRE(flywheelService.hasDataChanged());

            flywheelService.getData();

            REQUIRE(flywheelService.hasDataChanged() == false);
        }

        SECTION("debounce filter")
        {
            const auto dtLarge1 = RowerProfile::Defaults::rotationDebounceTimeMin + 100000;
            const auto dtLarge2 = RowerProfile::Defaults::rotationDebounceTimeMin + 95000;
            const auto dtMed1 = RowerProfile::Defaults::rotationDebounceTimeMin + 50000;
            const auto dtMed2 = RowerProfile::Defaults::rotationDebounceTimeMin + 52000;
            const auto dtMed3 = RowerProfile::Defaults::rotationDebounceTimeMin + 54000;
            const auto dtSmall = RowerProfile::Defaults::rotationDebounceTimeMin + 1000;

            SECTION("should accept first impulse")
            {
                FlywheelService flywheelService;

                flywheelService.processRotation(dtLarge1);

                REQUIRE(flywheelService.hasDataChanged());
                const auto result = flywheelService.getData();
                REQUIRE(result.rawImpulseCount == 1);
                REQUIRE(result.deltaTime == dtLarge1);
            }

            SECTION("should accept second impulse when delta difference is small enough")
            {
                FlywheelService flywheelService;

                auto currentTime = dtLarge1;
                flywheelService.processRotation(currentTime);
                // diff = |102000 - 107000| = 5000 <= 102000 ✓
                currentTime += dtLarge2;
                flywheelService.processRotation(currentTime);

                const auto result = flywheelService.getData();
                REQUIRE(result.rawImpulseCount == 2);
                REQUIRE(result.deltaTime == dtLarge2);
            }

            SECTION("should reject impulse when delta drops significantly")
            {
                FlywheelService flywheelService;

                auto currentTime = dtLarge1;
                flywheelService.processRotation(currentTime);

                // diff = |8000 - 107000| = 99000 > 8000 ✗
                currentTime += dtSmall;
                flywheelService.processRotation(currentTime);

                const auto result = flywheelService.getData();
                REQUIRE(result.rawImpulseCount == 1);
                REQUIRE(result.deltaTime == dtLarge1);
            }

            SECTION("should handle gradually changing deltas")
            {
                FlywheelService flywheelService;

                auto currentTime = dtMed1;
                flywheelService.processRotation(currentTime);

                // diff = |52000 - 50000| = 2000 <= 52000 ✓
                currentTime += dtMed2;
                flywheelService.processRotation(currentTime);
                // diff = |54000 - 52000| = 2000 <= 54000 ✓
                currentTime += dtMed3;
                flywheelService.processRotation(currentTime);

                const auto result = flywheelService.getData();
                REQUIRE(result.rawImpulseCount == 3);
                REQUIRE(result.deltaTime == dtMed3);
            }

            SECTION("should demonstrate filter rejection with big delta jump")
            {
                FlywheelService flywheelService;

                auto currentTime = dtLarge1;
                flywheelService.processRotation(currentTime);

                // diff = |102000 - 107000| = 5000 <= 102000 ✓
                currentTime += dtLarge2;
                flywheelService.processRotation(currentTime);

                const auto validResult = flywheelService.getData();
                REQUIRE(validResult.rawImpulseCount == 2);
                REQUIRE(validResult.deltaTime == dtLarge2);

                // diff = |8000 - 102000| = 94000 > 8000 ✗
                currentTime += dtSmall;
                flywheelService.processRotation(currentTime);

                const auto finalResult = flywheelService.getData();
                REQUIRE(finalResult.rawImpulseCount == 2);
                REQUIRE(finalResult.deltaTime == dtLarge2);
            }
        }
    }
}
// NOLINTEND(readability-magic-numbers)