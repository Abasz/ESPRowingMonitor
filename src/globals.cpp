#include "Arduino.h"

#include "globals.h"

Preferences preferences;

EEPROMService eepromService(preferences);
SdCardService sdCardService;
OtaUpdaterService otaService;
BluetoothService bleService(eepromService, sdCardService, otaService);
FlywheelService flywheelService;
StrokeService strokeService;
PowerManagerService powerManagerService;

PeripheralsController peripheralController(bleService, sdCardService, eepromService);
StrokeController strokeController(strokeService, flywheelService);
PowerManagerController powerManagerController(powerManagerService);

IRAM_ATTR void rotationInterrupt()
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

    // NOLINTBEGIN
    char timestamp[20];
    sprintf(timestamp, "%02d:%02d:%02d.%06d ", hours, minutes, seconds, microSeconds);
    _logOutput->print(timestamp);
    // NOLINTEND
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