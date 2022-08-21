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
        Log.infoln("distance: %f", strokeController.getDistance() / 100.0);
        Log.infoln("revCount: %d", strokeController.getRevCount());
        strokeController.setPreviousRevCount();
    }

    if (strokeController.getStrokeCount() > strokeController.getPreviousStrokeCount())
    {
        Log.infoln("driveDuration: %f", strokeController.getDriveDuration());
        Log.infoln("dragFactor: %d", strokeController.getDragFactor());
        Log.infoln("power: %d", strokeController.getAvgStrokePower());
        strokeController.setPreviousStrokeCount();
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