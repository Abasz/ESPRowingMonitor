#include "Arduino.h"

#include "globals.h"

Preferences preferences;

EEPROMService eepromService(preferences);
BluetoothService bleService(eepromService);
NetworkService networkService(eepromService);
FlywheelService flywheelService;
StrokeService strokeService;
PowerManagerService powerManagerService;

BluetoothController bleController(bleService, networkService, eepromService);
StrokeController strokeController(strokeService, flywheelService);
PowerManagerController powerManagerController(powerManagerService);

IRAM_ATTR void rotationInterrupt()
{
    // execution time general: 1-5, max: 520 and a few 120
    // auto start = micros();
    flywheelService.processRotation(micros());
    // auto stop = micros();

    // Serial.print("rotationInterrupt: ");
    // Serial.println(stop - start);
}

void attachRotationInterrupt()
{
    attachInterrupt(digitalPinToInterrupt(Settings::sensorPinNumber), rotationInterrupt, FALLING);
}

void detachRotationInterrupt()
{
    detachInterrupt(digitalPinToInterrupt(Settings::sensorPinNumber));
}

void printPrefix(Print *_logOutput, int logLevel)
{
    printTimestamp(_logOutput);
    printLogLevel(_logOutput, logLevel);
}

void printTimestamp(Print *_logOutput)
{
    unsigned long const msecs = micros();
    unsigned long const secs = msecs / msecsPerSec;
    unsigned long const microSeconds = msecs % msecsPerSec;
    unsigned long const seconds = secs % secsPerMin;
    unsigned long const minutes = (secs / secsPerMin) % secsPerMin;
    unsigned long const hours = (secs % secsPerDay) / secsPerHour;

    // NOLINTBEGIN
    char timestamp[20];
    sprintf(timestamp, "%02d:%02d:%02d.%06d ", hours, minutes, seconds, microSeconds);
    _logOutput->print(timestamp);
    // NOLINTEND
}

void printLogLevel(Print *_logOutput, int logLevel)
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