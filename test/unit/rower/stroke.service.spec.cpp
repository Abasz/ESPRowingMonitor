#include <fstream>
#include <vector>

#include "../include/catch_amalgamated.hpp"

#include "../include/Arduino.h"

#include "../../../src/rower/stroke.service.h"
#include "../../../src/utils/configuration.h"

using std::ifstream;
using std::stof;
using std::string;
using std::vector;

TEST_CASE("StrokeService")
{
    SECTION("should have correct settings for test")
    {
        CHECK(Configurations::impulsesPerRevolution == 3);
        CHECK(Configurations::impulseDataArrayLength == 7);
        CHECK(Configurations::flywheelInertia == 0.073F);
        CHECK(Configurations::dragCoefficientsArrayLength == 1);
        CHECK(Configurations::goodnessOfFitThreshold == 0.97F);
        CHECK(Configurations::rotationDebounceTimeMin == 7'000);
        CHECK(Configurations::sprocketRadius == 1.5F / 100);
        CHECK(Configurations::minimumDriveTime == 300'000);
        CHECK(Configurations::minimumRecoveryTime == 300'000);
    }

    ifstream deltaTimesStream("test/unit/rower/test-data/stroke.service.spec.deltaTimes.txt");
    REQUIRE(deltaTimesStream.good());

    ifstream slopeStream("test/unit/rower/test-data/stroke.service.spec.slope.txt");
    REQUIRE(slopeStream.good());

    ifstream torqueStream("test/unit/rower/test-data/stroke.service.spec.torque.txt");
    REQUIRE(torqueStream.good());

    ifstream forceCurveStream("test/unit/rower/test-data/stroke.service.spec.forceCurves.txt");
    REQUIRE(forceCurveStream.good());

    ifstream dragFactorStream("test/unit/rower/test-data/stroke.service.spec.dragFactor.txt");
    REQUIRE(dragFactorStream.good());

    vector<unsigned long> deltaTimes;
    const auto arraySize = 1'764;
    deltaTimes.reserve(arraySize);
    vector<double> slopes;
    slopes.reserve(arraySize - 1);
    vector<double> torques;
    torques.reserve(arraySize - 1);
    vector<vector<float>> forceCurves;
    forceCurves.reserve(10);
    vector<double> dragFactors;
    dragFactors.reserve(10);

    unsigned long deltaTime = 0;
    while (deltaTimesStream >> deltaTime)
    {
        deltaTimes.push_back(deltaTime);
    }

    auto slope = 0.0;
    while (slopeStream >> slope)
    {
        slopes.push_back(slope);
    }

    auto torque = 0.0;
    while (torqueStream >> torque)
    {
        torques.push_back(torque);
    }

    auto dragFactor = 0.0;
    while (dragFactorStream >> dragFactor)
    {
        dragFactors.push_back(dragFactor);
    }

    string forceCurve;
    while (forceCurveStream >> forceCurve)
    {
        size_t pos_start = 0;
        size_t pos_end = 0;
        string token;
        vector<float> res;

        while ((pos_end = forceCurve.find(",", pos_start)) != string::npos)
        {
            token = forceCurve.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + 1;
            res.push_back(stof(token));
        }

        res.push_back(stof(forceCurve.substr(pos_start)));

        forceCurves.push_back(res);
    }

    REQUIRE(!deltaTimes.empty());
    REQUIRE(!slopes.empty());
    REQUIRE(!torques.empty());
    REQUIRE(!forceCurves.empty());

    SECTION("processData method should correctly determine")
    {
        StrokeService strokeService;
        const auto angularDisplacementPerImpulse = (2 * PI) / 3;
        auto rawImpulseCount = 0UL;
        auto totalTime = 0UL;
        Configurations::precision totalAngularDisplacement = 0.0;
        RowingDataModels::RowingMetrics rowingMetrics;
        for (const auto &deltaTime : deltaTimes)
        {
            totalAngularDisplacement += angularDisplacementPerImpulse;
            totalTime += deltaTime;
            rawImpulseCount++;
            RowingDataModels::FlywheelData data{
                .rawImpulseCount = rawImpulseCount,
                .deltaTime = deltaTime,
                .totalTime = totalTime,
                .totalAngularDisplacement = totalAngularDisplacement,
                .cleanImpulseTime = totalTime,
                .rawImpulseTime = totalTime,
            };

            strokeService.processData(data);
            const auto prevStrokeCount = rowingMetrics.strokeCount;
            rowingMetrics = strokeService.getData();

            if (rowingMetrics.strokeCount > prevStrokeCount)
            {
                SECTION("force curves on new stroke (" + std::to_string(rowingMetrics.strokeCount) + ")")
                {
                    INFO("deltaTime: " << deltaTime << ", stroke number: " << rowingMetrics.strokeCount);
                    REQUIRE_THAT(rowingMetrics.driveHandleForces, Catch::Matchers::RangeEquals(forceCurves[rowingMetrics.strokeCount - 1], [](float first, float second)
                                                                                               { return std::abs(first - second) < 0.00001F; }));
                    REQUIRE_THAT(rowingMetrics.dragCoefficient * 1e6, Catch::Matchers::WithinRel(dragFactors[rowingMetrics.strokeCount - 1], 0.0000001));
                }
            }
        }

        SECTION("total rowing metrics")
        {
            REQUIRE(rowingMetrics.strokeCount == 10);
            REQUIRE(rowingMetrics.lastStrokeTime == 32'573'215);
            const auto expectedDistance = 9'290.9570160674;
            REQUIRE_THAT(rowingMetrics.distance, Catch::Matchers::WithinRel(expectedDistance, 0.0000001));
            REQUIRE(rowingMetrics.lastRevTime == 39'577'207);
        }
    }
}