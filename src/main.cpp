#include "ArduinoLog.h"

#include "globals.h"

#include "test.array.h"

void setup()
{
    Serial.begin(static_cast<int>(Configurations::baudRate));
    while (!Serial && !(bool)Serial.available())
    {
    }

    Log.begin(static_cast<int>(Configurations::defaultLogLevel), &Serial, false);
    Log.setPrefix(printPrefix);

    eepromService.setup();
    Log.setLevel(static_cast<int>(eepromService.getLogLevel()));

    peripheralController.begin();
    powerManagerController.begin();
    StrokeController::begin();

    if constexpr (Configurations::batteryPinNumber != GPIO_NUM_NC)
    {
        peripheralController.notifyBattery(powerManagerController.getBatteryLevel());
    }

#if defined(SIMULATE_FILE)
    setupFileSimulation();
#endif
}

// execution time
// - not connected 30 microsec
// - connected  microsec 2000-4900
void loop()
{
#if defined(SIMULATE_ROTATION)
    simulateRotation();
#endif

    strokeController.update();
    peripheralController.update(powerManagerController.getBatteryLevel());
    powerManagerController.update(strokeController.getLastRevTime(), peripheralController.isAnyDeviceConnected());

    if (strokeController.getRawImpulseCount() != strokeController.getPreviousRawImpulseCount())
    {
        peripheralController.updateDeltaTime(strokeController.getDeltaTime());
        strokeController.setPreviousRawImpulseCount();
    }

    // auto start = micros();

    // execution time
    // - not connected 20-30 microsec
    // - connected 1900-2200 microsec
    // auto start = micros();
    const auto now = millis();
    const auto minUpdateInterval = 4000;
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

    if constexpr (Configurations::batteryPinNumber != GPIO_NUM_NC)
    {
        if (powerManagerController.getBatteryLevel() != powerManagerController.getPreviousBatteryLevel())
        {
            Log.infoln("batteryLevel: %d", powerManagerController.getBatteryLevel());
            peripheralController.notifyBattery(powerManagerController.getBatteryLevel());
            powerManagerController.setPreviousBatteryLevel();
        }
    }
    // auto stop = micros();
    // if (stop - start > 10)
    // {
    //     Serial.print("Main loop: ");
    //     Serial.println(stop - start);
    // }
}