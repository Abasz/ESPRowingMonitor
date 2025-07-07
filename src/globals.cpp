#include <array>
#include <string>

#include "esp_mac.h"

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
ConnectionManagerCallbacks connectionManagerCallbacks;

BluetoothController bleController(eepromService, otaService, settingsBleService, batteryBleService, deviceInfoBleService, otaBleService, baseMetricsBleService, extendedMetricsBleService, connectionManagerCallbacks);

PeripheralsController peripheralController(bleController, sdCardService, eepromService);
StrokeController strokeController(strokeService, flywheelService, eepromService);
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

std::string generateSerial()
{
    const unsigned char macAddressLength = 6U;
    std::array<unsigned char, macAddressLength> mac{};
    esp_read_mac(mac.data(), ESP_MAC_BT);

    const unsigned char addressStringSize = 6;
    std::string serial;
    serial.resize(addressStringSize);

    // Most efficient way to format without memory size issue with <format> header
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    snprintf(serial.data(), addressStringSize + 1,
             "%02X%02X%02X",
             mac[3], mac[4], mac[5]);

    return serial;
}