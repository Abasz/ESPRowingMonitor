#include "ArduinoLog.h"

#include "globals.h"

#include "test.array.h"

void setup()
{
    auto const baudRate = 115200;
    Serial.begin(baudRate);
    while (!Serial && !(bool)Serial.available())
    {
    }

    Log.begin(static_cast<int>(Configurations::defaultLogLevel), &Serial, false);
    Log.setPrefix(printPrefix);

    eepromService.setup();
    Log.setLevel(static_cast<int>(eepromService.getLogLevel()));

    peripheralController.begin();
    StrokeController::begin();
    powerManagerController.begin();

    peripheralController.notifyBattery(powerManagerController.getBatteryLevel());
}

// execution time
// - not connected 30 microsec
// - connected  microsec 2000-4900
void loop()
{
    // simulateRotation();

    strokeController.update();
    peripheralController.update(powerManagerController.getBatteryLevel());
    powerManagerController.update(strokeController.getRawImpulseTime(), peripheralController.isAnyDeviceConnected());

    // auto start = micros();

    // execution time
    // - not connected 20-30 microsec
    // - connected 1900-2200 microsec
    // auto start = micros();
    auto const now = millis();
    auto const minUpdateInterval = 4000;
    if (strokeController.getStrokeCount() != strokeController.getPreviousStrokeCount() || now - lastUpdateTime > minUpdateInterval)
    {
        peripheralController.updateData(strokeController.getAllData());
        lastUpdateTime = now;
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
        peripheralController.notifyDragFactor(strokeController.getDragFactor());
        strokeController.setPreviousStrokeCount();
        // auto stop = micros();
        // Serial.print("notifyDragFactor: ");
        // Serial.println(stop - start);
    }

    if (powerManagerController.getBatteryLevel() != powerManagerController.getPreviousBatteryLevel())
    {
        Log.infoln("batteryLevel: %d", powerManagerController.getBatteryLevel());
        peripheralController.notifyBattery(powerManagerController.getBatteryLevel());
        powerManagerController.setPreviousBatteryLevel();
    }
    // auto stop = micros();
    // if (stop - start > 10)
    // {
    //     Serial.print("Main loop: ");
    //     Serial.println(stop - start);
    // }
}