#include "ArduinoLog.h"

#include "globals.h"

#include "test.array.h"

void setup()
{
    Serial.begin(1500000);
    while (!Serial && !Serial.available())
    {
    }

    Log.begin(static_cast<int>(Settings::defaultLogLevel), &Serial, false);
    Log.setPrefix(printPrefix);

    strokeController.begin();

    //     bleController.notifyBattery(powerManagerController.getBatteryLevel());
    // #ifndef POWERMETER
    //     bleController.notifyCsc(0, 0, 0, 0);
    // #else
    //     bleController.notifyPsc(0, 0, 0, 0, 0);
    // #endif
}

// execution time
// - not connected 30 microsec
// - connected  microsec 2000-4900
void loop()
{
    strokeController.update();
}