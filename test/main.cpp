#include <numeric>

#include "ArduinoLog.h"

#include "globals.h"
#include "test.array.h"

using std::size;

void loop()
{
    simulatRotation();
    strokeController.update();
    if (strokeController.getRevCount() != strokeController.getPreviousRevCount())
    {
        strokeController.setPreviousRevCount();
    }

    if (strokeController.getStrokeCount() > strokeController.getPreviousStrokeCount())
    {
        Log.infoln("driveDuration: %f", strokeController.getDriveDuration());
        Log.infoln("dragFactor: %d", strokeController.getDragFactor());
        strokeController.setPreviousStrokeCount();
    }

    if (strokeController.getAvgStrokePower() != strokeController.getPreviousAvgStrokePower())
    {
        Log.infoln("power: %d", strokeController.getAvgStrokePower());
        strokeController.setPreviousAvgStrokePower();
    }
}

int main()
{
    while (i < size(testDeltaTimes))
    {
        loop();
    }
    return 0;
}