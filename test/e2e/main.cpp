#include <fstream>
#include <numeric>
#include <span>
#include <string>

#include "ArduinoLog.h"

#include "globals.h"

#include "./test.array.h"

void loop(const unsigned long now)
{
    simulateRotation(now);
    strokeController.update();
    if (strokeController.getRevCount() != strokeController.getPreviousRevCount())
    {
        Log.infoln("distance: %f", strokeController.getDistance() / 100.0);
        Log.infoln("revCount: %d", strokeController.getRevCount());
        strokeController.setPreviousRevCount();
    }

    if (strokeController.getStrokeCount() > strokeController.getPreviousStrokeCount())
    {
        Log.infoln("driveDuration: %f", strokeController.getDriveDuration());
        Log.infoln("recoveryDuration: %f", strokeController.getRecoveryDuration());
        Log.infoln("dragFactor: %d", strokeController.getDragFactor());
        Log.infoln("power: %d", strokeController.getAvgStrokePower());
        strokeController.setPreviousStrokeCount();
    }
}

int main(int argc, const char *argv[])
{
    const auto args = std::span(argv + 1, size_t(argc - 1));
    unsigned long now = 0;

    if (args.empty())
    {
        for (const auto &deltaTime : testDeltaTimes)
        {
            now += deltaTime;
            loop(now);
        }

        return 0;
    }

    if (args[0] == std::string("simulate"))
    {
        const auto minTimeThreshold = 40'000UL;
        const auto maxTimeThreshold = 45'000UL;
        unsigned long timeThreshold = maxTimeThreshold;
        const size_t impulseCount = 20'000;
        bool direction = false;
        for (size_t i = 0; i < impulseCount; i++)
        {
            now += timeThreshold;

            loop(now);

            if (direction)
            {
                timeThreshold += 1'000;
            }
            if (!direction)
            {
                timeThreshold -= 1'000;
            }
            if (timeThreshold == minTimeThreshold)
            {
                direction = true;
            }

            if (timeThreshold == maxTimeThreshold)
            {
                direction = false;
            }
        }

        return 0;
    }

    unsigned long deltaTime = 0;

    std::ifstream deltaTimeStream(args[0]);
    printf("Running external file: %s\n", args[0]);
    while (deltaTimeStream >> deltaTime)
    {
        now += deltaTime;
        loop(now);
    }

    return 0;
}
