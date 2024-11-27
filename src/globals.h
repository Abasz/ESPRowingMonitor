#pragma once

#include "Arduino.h"
#include "Preferences.h"

#include "./peripherals/bluetooth/ble-services/base-metrics.service.h"
#include "./peripherals/bluetooth/ble-services/battery.service.h"
#include "./peripherals/bluetooth/ble-services/device-info.service.h"
#include "./peripherals/bluetooth/ble-services/extended-metrics.service.h"
#include "./peripherals/bluetooth/ble-services/ota.service.h"
#include "./peripherals/bluetooth/ble-services/settings.service.h"
#include "./peripherals/bluetooth/bluetooth.controller.h"
#include "./peripherals/peripherals.controller.h"
#include "./peripherals/sd-card/sd-card.service.h"
#include "./rower/flywheel.service.h"
#include "./rower/stroke.controller.h"
#include "./rower/stroke.service.h"
#include "./utils/EEPROM/EEPROM.service.h"
#include "./utils/ota-updater/ota-updater.service.h"
#include "./utils/power-manager/power-manager.controller.h"
#include "./utils/power-manager/power-manager.service.h"

static unsigned long lastUpdateTime = 0UL;

static const unsigned long msecsPerSec = 1'000'000;
static const unsigned long secsPerMin = 60;
static const unsigned long secsPerHour = 3'600;
static const unsigned long secsPerDay = 86'400;

extern Preferences preferences;
extern EEPROMService eepromService;
extern OtaUpdaterService otaService;
extern PowerManagerService powerManagerService;
extern PowerManagerController powerManagerController;

extern FlywheelService flywheelService;
extern StrokeService strokeService;
extern StrokeController strokeController;

extern SettingsBleService settingsBleService;
extern BatteryBleService batteryBleService;
extern DeviceInfoBleService deviceInfoBleService;
extern OtaBleService otaBleService;
extern BaseMetricsBleService baseMetricsBleService;
extern ExtendedMetricBleService extendedMetricsBleService;
extern SdCardService sdCardService;

extern BluetoothController bleController;

extern PeripheralsController peripheralController;

void rotationInterrupt();

void attachRotationInterrupt();
void detachRotationInterrupt();

void printPrefix(Print *_logOutput, int logLevel);
void printTimestamp(Print *_logOutput);
void printLogLevel(Print *_logOutput, int logLevel);

consteval bool isOdd(unsigned long number)
{
    return number % 2 != 0;
};