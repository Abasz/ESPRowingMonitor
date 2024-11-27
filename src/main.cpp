#include "esp_ota_ops.h"

#include "Arduino.h"
#include "ArduinoLog.h"

#include "globals.h"

#include "./test.array.h"

void setup()
{
    Serial.begin(static_cast<int>(Configurations::baudRate));

#if ARDUINO_USB_CDC_ON_BOOT != 1
    while (!Serial && !(bool)Serial.available())
    {
    }
#endif

    Log.begin(static_cast<int>(Configurations::defaultLogLevel), &Serial, false);
    Log.setPrefix(printPrefix);

    eepromService.setup();
    Log.setLevel(static_cast<int>(eepromService.getLogLevel()));

    peripheralController.begin();
    powerManagerController.begin();
    strokeController.begin();

    if constexpr (Configurations::batteryPinNumber != GPIO_NUM_NC)
    {
        peripheralController.notifyBattery(powerManagerController.getBatteryLevel());
    }

    const esp_partition_t *partitionNext = esp_ota_get_next_update_partition(nullptr);

    Log.traceln("address: %d; label: %s; size: %d, subtype: %d, type: %d", partitionNext->address, partitionNext->label, partitionNext->size, partitionNext->subtype, partitionNext->type);

    const esp_partition_t *partitionCurrent = esp_ota_get_running_partition();
    Log.traceln("address: %d; label: %s; size: %d, subtype: %d, type: %d", partitionCurrent->address,
                partitionCurrent->label, partitionCurrent->size, partitionCurrent->subtype, partitionCurrent->type);

#if defined(SIMULATE_FILE)
    setupFileSimulation();
#endif
}

void loop()
{
#if defined(SIMULATE_ROTATION)
    simulateRotation();
#endif

    if (otaService.isUpdating())
    {
        return;
    }

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