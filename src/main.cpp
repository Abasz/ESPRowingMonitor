#include <Arduino.h>

#include "globals.h"

void setup()
{
    Serial.begin(115200);

    bleController.begin();
    strokeController.begin();
    powerManagerController.begin();

    bleController.notifyBattery(powerManagerController.getBatteryLevel());
    bleController.notifyCsc(0, 0, 0, 0);
}

auto lastStrokeCount = 0U;
byte lastBatteryLevel = 0;
// execution time
// - not connected 30 microsec
// - connected 1  1800-2350
// - connected 2 microsec 4000-4400
void loop()
{
    strokeController.readCscData();
    powerManagerController.checkSleep(strokeController.getLastRevTime(), bleController.isAnyDeviceConnected());

    // for (auto deltaTime : testDeltaRotations)
    // {
    //     delay(deltaTime / 1000);
    //     rotationInterrupt();
    // auto start = micros();

    // execution time
    // - not connected 20-30 microsec
    // - connected 1900-2200 microsec
    // auto start = micros();
    if (strokeController.getLastRevTime() != strokeController.getLastRevReadTime())
    {
        if (strokeController.getStrokeCount() > lastStrokeCount)
        {

            Serial.print("dragFactor: ");
            Serial.println(strokeController.getDragCoefficient() * 1e6);
            // execution time
            // - not connected: 73
            // - connected: 2000-2600
            // auto start = micros();
            bleController.notifyDragFactor(static_cast<byte>(strokeController.getDragCoefficient() * 1e6));
            lastStrokeCount = strokeController.getStrokeCount();
            // auto stop = micros();
            // Serial.print("notifyDragFactor: ");
            // Serial.println(stop - start);
        }

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

        strokeController.setLastRevReadTime();
        // auto stop = micros();
        // Serial.print("Main loop if statement: ");
        // Serial.println(stop - start);
    }
    auto battLevel = powerManagerController.getBatteryLevel();
    if (battLevel != lastBatteryLevel)
    {
        Serial.println(battLevel);
        bleController.notifyBattery(battLevel);
        lastBatteryLevel = battLevel;
    }
    // auto stop = micros();
    // Serial.print("Main loop: ");
    // Serial.println(stop - start);
    // }
}