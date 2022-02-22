#include <Arduino.h>

#include "globals.h"

BluetoothService bleService;
StrokeService strokeService;
PowerManagerService powerManagerService;

BluetoothController bleController(bleService);
StrokeController strokeController(strokeService);
PowerManagerController powerManagerController(powerManagerService);

IRAM_ATTR void rotationInterrupt()
{
    // execution time general: 1-5, max: 520 and a few 120
    // auto start = micros();
    strokeService.processRotation(micros());
    // auto stop = micros();

    // Serial.print("rotationInterrupt: ");
    // Serial.println(stop - start);
}

IRAM_ATTR void batteryMeasurementInterrupt()
{
    powerManagerService.measureBattery();
}

void attachRotationInterrupt()
{
    attachInterrupt(digitalPinToInterrupt(GPIO_NUM_26), rotationInterrupt, RISING);
}

void detachRotationInterrupt()
{
    detachInterrupt(digitalPinToInterrupt(GPIO_NUM_26));
}

void printPrefix(Print *_logOutput, int logLevel)
{
    printTimestamp(_logOutput);
    printLogLevel(_logOutput, logLevel);
}

void printTimestamp(Print *_logOutput)
{
    unsigned long const msecs = micros();
    unsigned long const secs = msecs / MSECS_PER_SEC;
    unsigned long const microSeconds = msecs % MSECS_PER_SEC;
    unsigned long const seconds = secs % SECS_PER_MIN;
    unsigned long const minutes = (secs / SECS_PER_MIN) % SECS_PER_MIN;
    unsigned long const hours = (secs % SECS_PER_DAY) / SECS_PER_HOUR;

    char timestamp[20];
    sprintf(timestamp, "%02d:%02d:%02d.%06d ", hours, minutes, seconds, microSeconds);
    _logOutput->print(timestamp);
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