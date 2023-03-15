// NOLINTBEGIN(cppcoreguidelines-init-variables,readability-function-cognitive-complexity)
#include <fstream>
#include <vector>

#include "../Arduino.h"
#include "catch_amalgamated.hpp"

#include "../../src/rower/stroke.service.h"

using std::ifstream;
using std::stod;
using std::string;
using std::vector;

TEST_CASE("StrokeService")
{
    SECTION("should have correct settings for test")
    {
        CHECK(Settings::impulsesPerRevolution == 3);
        CHECK(Settings::impulseDataArrayLength == 7);
        CHECK(Settings::flywheelInertia == 0.073);
        CHECK(Settings::dragCoefficientsArrayLength == 1);
        CHECK(Settings::goodnessOfFitThreshold == 0.97);
        CHECK(Settings::rotationDebounceTimeMin == 7000);
        CHECK(Settings::sprocketRadius == 1.5);
        CHECK(Settings::strokeDebounceTime == 200000);
    }

    ifstream deltaTimesStream("test/unit/stroke.service.spec.deltaTimes.txt");
    REQUIRE(deltaTimesStream.good());

    ifstream slopeStream("test/unit/stroke.service.spec.slope.txt");
    REQUIRE(slopeStream.good());

    ifstream torqueStream("test/unit/stroke.service.spec.torque.txt");
    REQUIRE(torqueStream.good());

    ifstream forceCurveStream("test/unit/stroke.service.spec.forceCurves.txt");
    REQUIRE(forceCurveStream.good());

    vector<unsigned long> deltaTimes;
    const auto arraySize = 1764;
    deltaTimes.reserve(arraySize);
    vector<double> slopes;
    slopes.reserve(arraySize - 1);
    vector<double> torques;
    torques.reserve(arraySize - 1);
    vector<vector<double>> forceCurves;
    forceCurves.reserve(10);

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

    string forceCurve = "";
    while (forceCurveStream >> forceCurve)
    {
        size_t pos_start = 0;
        size_t pos_end = 0;
        string token = "";
        vector<double> res;

        while ((pos_end = forceCurve.find(",", pos_start)) != string::npos)
        {
            token = forceCurve.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + 1;
            res.push_back(stod(token));
        }

        res.push_back(stod(forceCurve.substr(pos_start)));

        forceCurves.push_back(res);
    }

    REQUIRE(deltaTimes.size() > 0);
    REQUIRE(slopes.size() > 0);
    REQUIRE(torques.size() > 0);
    REQUIRE(forceCurves.size() > 0);

    SECTION("processData method should correctly determine")
    {
        StrokeService strokeService;
        auto angularDisplacementPerImpulse = (2 * PI) / 3;
        auto rawImpulseCount = 0UL;
        auto totalTime = 0UL;
        auto totalAngularDisplacement = 0.0;
        RowingDataModels::RowingMetrics rowingMetrics;
        for (auto &deltaTime : deltaTimes)
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
                .rawImpulseTime = totalTime};

            strokeService.processData(data);
            auto prevStrokeCount = rowingMetrics.strokeCount;
            rowingMetrics = strokeService.getData();

            if (rowingMetrics.strokeCount > prevStrokeCount)
            {
                SECTION("force curves on new strokes")
                {
                    INFO("deltaTime: " << deltaTime);
                    REQUIRE_THAT(rowingMetrics.driveHandleForces, Catch::Matchers::RangeEquals(forceCurves[rowingMetrics.strokeCount - 1], [](double a, double b)
                                                                                               { return std::abs(a - b) < 0.0000001; }));
                }
            }
        }

        SECTION("total rowing metrics")
        {
            REQUIRE(rowingMetrics.strokeCount == 10);
            REQUIRE(rowingMetrics.lastStrokeTime == 32679715);
            const auto expectedDistance = 9307.2041350242;
            REQUIRE_THAT(rowingMetrics.distance, Catch::Matchers::WithinRel(expectedDistance, 0.0000001));
            REQUIRE(rowingMetrics.lastRevTime == 39707220);
        }
    }
}
// NOLINTEND(cppcoreguidelines-init-variables,readability-function-cognitive-complexity)