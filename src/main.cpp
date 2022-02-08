#include <Arduino.h>

#include "ArduinoLog.h"

#include "globals.h"

void setup()
{
    Serial.begin(115200);
    while (!Serial && !Serial.available())
    {
    }

    Log.begin(static_cast<int>(ArduinoLog::Log_level_trace), &Serial, false);
    Log.setPrefix(printPrefix);

    bleController.begin();
    strokeController.begin();
    powerManagerController.begin();

    bleController.notifyBattery(powerManagerController.getBatteryLevel());
    bleController.notifyCsc(0, 0, 0, 0);
}

// TODO: these tracking values should be moved to the controllers like for lastRevReadTime
auto lastStrokeCount = 0U;
byte lastBatteryLevel = 0;
auto lastAvgStrokePower = 0U;
// execution time
// - not connected 30 microsec
// - connected 1  1800-2350
// - connected 2 microsec 4000-4400
void loop()
{
    strokeController.update();
    powerManagerController.update(strokeController.getLastRevTime(), bleController.isAnyDeviceConnected());

    // for (auto deltaTime : testDeltaRotations)
    // {
    //     delay(deltaTime / 1000);
    //     rotationInterrupt();
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
        bleController.notifyCsc(
            strokeController.getLastRevTime(),
            strokeController.getRevCount(),
            strokeController.getLastStrokeTime(),
            strokeController.getStrokeCount());
        // auto stop = micros();
        // Serial.print("notifyCsc: ");
        // Serial.println(stop - start);

        strokeController.setPreviousRevCount();
        // auto stop = micros();
        // Serial.print("Main loop if statement: ");
        // Serial.println(stop - start);
    }

        if (strokeController.getStrokeCount() > lastStrokeCount)
        {
        Log.infoln("driveDuration: %D", strokeController.getDriveDuration());
        Log.infoln("dragFactor: %d", strokeController.getDragFactor());
            // execution time
            // - not connected: 73
            // - connected: 2000-2600
            // auto start = micros();
        bleController.notifyDragFactor(strokeController.getDragFactor());
            lastStrokeCount = strokeController.getStrokeCount();
            // auto stop = micros();
            // Serial.print("notifyDragFactor: ");
            // Serial.println(stop - start);
        }

    if (strokeController.getAvgStrokePower() != lastAvgStrokePower)
    {
        Log.infoln("power: %d", strokeController.getAvgStrokePower());
        lastAvgStrokePower = strokeController.getAvgStrokePower();
        // execution time
        // - not connected: 73
        // - connected: 2000-2600
        // auto start = micros();
        // auto stop = micros();
        // Serial.print("notifyDragFactor: ");
        // Serial.println(stop - start);
    }

    auto battLevel = powerManagerController.getBatteryLevel();
    if (battLevel != lastBatteryLevel)
    {
        Log.infoln("batteryLevel: %d", battLevel);
        bleController.notifyBattery(battLevel);
        lastBatteryLevel = battLevel;
    }
    // auto stop = micros();
    // Serial.print("Main loop: ");
    // Serial.println(stop - start);
    // }
}