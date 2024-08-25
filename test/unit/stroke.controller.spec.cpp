
#include <numeric>
#include <vector>

#include "./include/catch_amalgamated.hpp"
#include "./include/fakeit.hpp"

#include "../../src/rower/stroke.controller.h"

using namespace fakeit;

const std::vector<unsigned long> deltaTimes = {
    Configurations::rotationDebounceTimeMin + 2000,
    Configurations::rotationDebounceTimeMin + 1000,
    Configurations::rotationDebounceTimeMin + 500,
    Configurations::rotationDebounceTimeMin + 100,
};

TEST_CASE("StrokeController", "[rower]")
{
    SECTION("begin method should setup FlywheelService")
    {
        Mock<IStrokeService> mockStrokeService;
        Mock<IFlywheelService> mockFlywheelService;
        Fake(Method(mockFlywheelService, setup));

        StrokeController strokeController(mockStrokeService.get(), mockFlywheelService.get());
        strokeController.begin();

        Verify(Method(mockFlywheelService, setup)).Once();
    }

    SECTION("update method")
    {
        SECTION("should do nothing if data has not changed")
        {
            Mock<IStrokeService> mockStrokeService;
            Mock<IFlywheelService> mockFlywheelService;
            When(Method(mockFlywheelService, hasDataChanged)).Return(false);

            StrokeController strokeController(mockStrokeService.get(), mockFlywheelService.get());
            strokeController.update();

            Verify(Method(mockFlywheelService, hasDataChanged)).Once();
            VerifyNoOtherInvocations(mockFlywheelService);
            VerifyNoOtherInvocations(mockStrokeService);
        }

        SECTION("should skip processing if impulse count did not increased")
        {
            Mock<IStrokeService> mockStrokeService;
            Mock<IFlywheelService> mockFlywheelService;
            When(Method(mockFlywheelService, hasDataChanged)).Return(true);
            When(Method(mockFlywheelService, getData)).Return({});

            StrokeController strokeController(mockStrokeService.get(), mockFlywheelService.get());
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
            When(Method(mockFlywheelService, hasDataChanged)).Return(true);
            When(Method(mockFlywheelService, getData)).Return({
                .rawImpulseCount = 1,
            });

            Fake(Method(mockStrokeService, processData));
            When(Method(mockStrokeService, getData)).Return({});

            StrokeController strokeController(mockStrokeService.get(), mockFlywheelService.get());
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
