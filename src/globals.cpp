#include <array>

#include "Arduino.h"

#include "globals.h"

Preferences preferences;

EEPROMService eepromService(preferences);
OtaUpdaterService otaService;
PowerManagerService powerManagerService;

FlywheelService flywheelService;
StrokeService strokeService;

SdCardService sdCardService;
BatteryBleService batteryBleService;
DeviceInfoBleService deviceInfoBleService;
OtaBleService otaBleService(otaService);
SettingsBleService settingsBleService(sdCardService, eepromService);
BaseMetricsBleService baseMetricsBleService(settingsBleService, eepromService);
ExtendedMetricBleService extendedMetricsBleService;

BluetoothController bleController(eepromService, otaService, settingsBleService, batteryBleService, deviceInfoBleService, otaBleService, baseMetricsBleService, extendedMetricsBleService);

PeripheralsController peripheralController(bleController, sdCardService, eepromService);
StrokeController strokeController(strokeService, flywheelService);
PowerManagerController powerManagerController(powerManagerService);

void IRAM_ATTR rotationInterrupt()
{
    flywheelService.processRotation(micros());
}

void attachRotationInterrupt()
{
    attachInterrupt(digitalPinToInterrupt(Configurations::sensorPinNumber), rotationInterrupt, FALLING);
}

void detachRotationInterrupt()
{
    detachInterrupt(digitalPinToInterrupt(Configurations::sensorPinNumber));
}

void restartWithDelay(const unsigned long millis)
{
    auto *const timer = timerBegin(1e6);
    timerAttachInterrupt(timer, []() IRAM_ATTR
                         { esp_restart(); });
    timerAlarm(timer, millis * 1'000, false, 0);
}

void printPrefix(Print *const _logOutput, const int logLevel)
{
    printTimestamp(_logOutput);
    printLogLevel(_logOutput, logLevel);
}

void printTimestamp(Print *const _logOutput)
{
    const unsigned long msecs = micros();
    const unsigned long secs = msecs / msecsPerSec;
    const unsigned long microSeconds = msecs % msecsPerSec;
    const unsigned long seconds = secs % secsPerMin;
    const unsigned long minutes = (secs / secsPerMin) % secsPerMin;
    const unsigned long hours = (secs % secsPerDay) / secsPerHour;

    std::array<char, 20> timestamp{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    snprintf(timestamp.data(), timestamp.size(), "%02lu:%02lu:%02lu.%06lu ", hours, minutes, seconds, microSeconds);
    _logOutput->print(timestamp.data());
}

void printLogLevel(Print *const _logOutput, const int logLevel)
{
    switch (logLevel)
    {
    default:
    case 0:
        _logOutput->print(" SILENT - ");
        break;
    case 1:
        _logOutput->print("  FATAL - ");
        break;
    case 2:
        _logOutput->print("  ERROR - ");
        break;
    case 3:
        _logOutput->print("WARNING - ");
        break;
    case 4:
        _logOutput->print("   INFO - ");
        break;
    case 5:
        _logOutput->print("  TRACE - ");
        break;
    case 6:
        _logOutput->print("VERBOSE - ");
        break;
    }
}