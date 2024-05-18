#include "ArduinoLog.h"

#include "globals.h"

#include "./test.array.h"

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

    const auto now = millis();
    const auto minUpdateInterval = 4'000;
    if (strokeController.getStrokeCount() != strokeController.getPreviousStrokeCount() || now - lastUpdateTime > minUpdateInterval)
    {
        peripheralController.updateData(strokeController.getAllData());
        lastUpdateTime = now;
    }

    if (strokeController.getStrokeCount() != strokeController.getPreviousStrokeCount())
    {
        Log.traceln("driveDuration: %D", strokeController.getDriveDuration());
        Log.traceln("recoveryDuration: %D", strokeController.getRecoveryDuration());
        Log.traceln("dragFactor: %d", strokeController.getDragFactor());
        Log.traceln("power: %d", strokeController.getAvgStrokePower());
        Log.traceln("distance: %D", strokeController.getDistance() / 100.0);

        peripheralController.notifyDragFactor(strokeController.getDragFactor());
        strokeController.setPreviousStrokeCount();
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