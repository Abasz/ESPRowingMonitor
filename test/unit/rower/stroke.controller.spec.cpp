#include <vector>

#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "../../../src/rower/stroke.controller.h"

using namespace fakeit;

const std::vector<unsigned long> deltaTimes = {
    RowerProfile::Defaults::rotationDebounceTimeMin + 2000,
    RowerProfile::Defaults::rotationDebounceTimeMin + 1000,
    RowerProfile::Defaults::rotationDebounceTimeMin + 500,
    RowerProfile::Defaults::rotationDebounceTimeMin + 100,
};

TEST_CASE("StrokeController", "[rower]")
{
    SECTION("begin method should setup FlywheelService and StrokeService")
    {
        Mock<IStrokeService> mockStrokeService;
        Mock<IFlywheelService> mockFlywheelService;
        Mock<IEEPROMService> mockEEPROMService;

        Fake(Method(mockFlywheelService, setup));
        When(Method(mockEEPROMService, getMachineSettings)).Return(RowerProfile::MachineSettings{});
        When(Method(mockEEPROMService, getSensorSignalSettings)).Return(RowerProfile::SensorSignalSettings{});
#if ENABLE_RUNTIME_SETTINGS
        Fake(Method(mockStrokeService, setup));
#endif

        StrokeController strokeController(mockStrokeService.get(), mockFlywheelService.get(), mockEEPROMService.get());
        strokeController.begin();

        Verify(Method(mockEEPROMService, getMachineSettings)).Once();
        Verify(Method(mockEEPROMService, getSensorSignalSettings)).Once();
        Verify(Method(mockFlywheelService, setup)).Once();
#if ENABLE_RUNTIME_SETTINGS
        Verify(Method(mockStrokeService, setup)).Once();
#endif
    }

    SECTION("update method")
    {
        SECTION("should do nothing if data has not changed")
        {
            Mock<IStrokeService> mockStrokeService;
            Mock<IFlywheelService> mockFlywheelService;
            Mock<IEEPROMService> mockEEPROMService;
            When(Method(mockFlywheelService, hasDataChanged)).Return(false);

            StrokeController strokeController(mockStrokeService.get(), mockFlywheelService.get(), mockEEPROMService.get());
            strokeController.update();

            Verify(Method(mockFlywheelService, hasDataChanged)).Once();
            VerifyNoOtherInvocations(mockFlywheelService);
            VerifyNoOtherInvocations(mockStrokeService);
        }

        SECTION("should skip processing if impulse count did not increased")
        {
            Mock<IStrokeService> mockStrokeService;
            Mock<IFlywheelService> mockFlywheelService;
            Mock<IEEPROMService> mockEEPROMService;
            When(Method(mockFlywheelService, hasDataChanged)).Return(true);
            When(Method(mockFlywheelService, getData)).Return({});

            StrokeController strokeController(mockStrokeService.get(), mockFlywheelService.get(), mockEEPROMService.get());
            strokeController.update();

            Verify(Method(mockFlywheelService, hasDataChanged)).Once();
            Verify(Method(mockFlywheelService, getData)).Once();
            Verify(Method(mockStrokeService, processData)).Exactly(0);
            Verify(Method(mockStrokeService, getData)).Exactly(0);
            VerifyNoOtherInvocations(mockFlywheelService);
            VerifyNoOtherInvocations(mockStrokeService);
        }

        SECTION("if new data is available")
        {
            Mock<IStrokeService> mockStrokeService;
            Mock<IFlywheelService> mockFlywheelService;
            Mock<IEEPROMService> mockEEPROMService;
            When(Method(mockFlywheelService, hasDataChanged)).Return(true);
            When(Method(mockFlywheelService, getData)).Return({
                .rawImpulseCount = 1,
            });

            Fake(Method(mockStrokeService, processData));
            When(Method(mockStrokeService, getData)).Return({});

            StrokeController strokeController(mockStrokeService.get(), mockFlywheelService.get(), mockEEPROMService.get());
            strokeController.update();

            Verify(Method(mockFlywheelService, hasDataChanged)).Once();

            SECTION("should get flywheel data")
            {
                Verify(Method(mockFlywheelService, getData)).Once();
            }

            SECTION("should process new flywheel data")
            {
                Verify(Method(mockStrokeService, processData)).Once();
            }

            SECTION("should get new process metrics")
            {
                Verify(Method(mockStrokeService, getData)).Once();
            }
        }
    }
}
