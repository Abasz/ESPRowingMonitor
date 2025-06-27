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
    Configurations::rotationDebounceTimeMin + 2000,
    Configurations::rotationDebounceTimeMin + 1000,
    Configurations::rotationDebounceTimeMin + 500,
    Configurations::rotationDebounceTimeMin + 100,
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

        flywheelService.setup(RowerProfile::MachineSettings{});

        Verify(Method(mockGlobals, attachRotationInterrupt)).Once();
        Verify(Method(mockArduino, pinMode).Using(Configurations::sensorPinNumber, INPUT_PULLUP)).Once();
    }

    SECTION("processRotation method")
    {
        SECTION("should indicate data change via hasDataChanged method on new measurement")
        {
            FlywheelService flywheelService;

            flywheelService.processRotation(Configurations::rotationDebounceTimeMin + 1000);

            REQUIRE(flywheelService.hasDataChanged());
        }

        SECTION("should ignore deltaTimes below the debounce time")
        {
            FlywheelService flywheelService;

            flywheelService.processRotation(Configurations::rotationDebounceTimeMin - 1000);

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
                .totalAngularDisplacement = Configurations::angularDisplacementPerImpulse * (double)deltaTimes.size(),
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

            flywheelService.processRotation(Configurations::rotationDebounceTimeMin + 1000);

            REQUIRE(flywheelService.hasDataChanged());

            flywheelService.getData();

            REQUIRE(flywheelService.hasDataChanged() == false);
        }
    }
}
// NOLINTEND(readability-magic-numbers)