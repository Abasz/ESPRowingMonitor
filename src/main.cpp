#include "ArduinoLog.h"

#include "globals.h"

#include "test.array.h"

void setup()
{
    Serial.begin(115200);
    while (!Serial && !Serial.available())
    {
    }

    Log.begin(static_cast<int>(Settings::defaultLogLevel), &Serial, false);
    Log.setPrefix(printPrefix);

    eepromService.setup();
    Log.setLevel(static_cast<int>(eepromService.getLogLevel()));

    bleController.begin();
    strokeController.begin();
    powerManagerController.begin();

    bleController.notifyBattery(powerManagerController.getBatteryLevel());
    bleController.notify(0, 0, 0, 0, 0);
}

// execution time
// - not connected 30 microsec
// - connected  microsec 2000-4900
void loop()
{
    // simulateRotation();

    strokeController.update();
    bleController.update();
    powerManagerController.update(strokeController.getRawImpulseTime(), bleController.isAnyDeviceConnected());

    // auto start = micros();

    // execution time
    // - not connected 20-30 microsec
    // - connected 1900-2200 microsec
    // auto start = micros();
    if (strokeController.getRevCount() != strokeController.getPreviousRevCount())
    {
        // execution time
        // - not connected: 5 microsec
        // - connected: 1800-2100 microsec
        // auto start = micros();
        bleController.notify(
            strokeController.getLastRevTime(),
            strokeController.getRevCount(),
            strokeController.getLastStrokeTime(),
            strokeController.getStrokeCount(),
            strokeController.getAvgStrokePower());
        // auto stop = micros();
        // Serial.print("notifyCsc: ");
        // Serial.println(stop - start);

        strokeController.setPreviousRevCount();
        // auto stop = micros();
        // Serial.print("Main loop if statement: ");
        // Serial.println(stop - start);
    }

    if (strokeController.getStrokeCount() != strokeController.getPreviousStrokeCount())
    {
        Log.infoln("driveDuration: %D", strokeController.getDriveDuration());
        Log.infoln("recoveryDuration: %D", strokeController.getRecoveryDuration());
        Log.infoln("dragFactor: %d", strokeController.getDragFactor());
        Log.infoln("power: %d", strokeController.getAvgStrokePower());
        Log.infoln("distance: %D", strokeController.getDistance() / 100.0);

        // execution time
        // - not connected: 173-200
        // - connected: 900-2700
        // auto start = micros();
        bleController.notifyDragFactor(strokeController.getDragFactor());
        strokeController.setPreviousStrokeCount();
        // auto stop = micros();
        // Serial.print("notifyDragFactor: ");
        // Serial.println(stop - start);
    }

    auto battLevel = powerManagerController.getBatteryLevel();
    if (battLevel != powerManagerController.getPreviousBatteryLevel())
    {
        Log.infoln("batteryLevel: %d", battLevel);
        bleController.notifyBattery(battLevel);
        powerManagerController.setPreviousBatteryLevel();
    }
    // auto stop = micros();
    // if (stop - start > 10)
    // {
    //     Serial.print("Main loop: ");
    //     Serial.println(stop - start);
    // }
}