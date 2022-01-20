#include <Arduino.h>

#include "globals.h"

BluetoothController bleController;
StrokeController strokeController;
PowerManagerController powerManagerController;

IRAM_ATTR void rotationInterrupt()
{
    // execution time: 1-5
    strokeController.processRotation(micros());
}

IRAM_ATTR void connectionLedIndicatorInterrupt()
{
    bleController.checkConnectedDevices();
}

IRAM_ATTR void batteryMeasurementInterrupt()
{
    powerManagerController.measureBattery();
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

    // Total time
    unsigned long const msecs = micros();
    unsigned long const secs = msecs / MSECS_PER_SEC;

    // Time in components
    unsigned long const microSeconds = msecs % MSECS_PER_SEC;
    unsigned long const seconds = secs % SECS_PER_MIN;
    unsigned long const minutes = (secs / SECS_PER_MIN) % SECS_PER_MIN;
    unsigned long const hours = (secs % SECS_PER_DAY) / SECS_PER_HOUR;

    // Time as string
    char timestamp[20];
    sprintf(timestamp, "%02d:%02d:%02d.%06d ", hours, minutes, seconds, microSeconds);
    _logOutput->print(timestamp);
}

void printLogLevel(Print *_logOutput, int logLevel)
{
    /// Show log description based on log level
    switch (logLevel)
    {
    default:
    case 0:
        _logOutput->print("SILENT - ");
        break;
    case 1:
        _logOutput->print("FATAL - ");
        break;
    case 2:
        _logOutput->print("ERROR - ");
        break;
    case 3:
        _logOutput->print("WARNING - ");
        break;
    case 4:
        _logOutput->print("INFO - ");
        break;
    case 5:
        _logOutput->print("TRACE - ");
        break;
    case 6:
        _logOutput->print("VERBOSE - ");
        break;
    }
}

// void DEBUG_PrintDeltaTimesData()
// {
//     for (auto i : [])
//     {
//         Serial.print(i);
//         Serial.print(",");
//     }
//     Serial.println();
// }

// void DEBUG_CalculateRPMData()
// {
// if (testI == sizeof(testData) / sizeof(testData[0]))
// {
//     Serial.println("Resetting test data");
//     Serial.print("Final Revcount: ");
//     Serial.println(strokeController.getRevCount());
//     Serial.print("Final StrokeCount: ");
//     Serial.println(strokeController.getStrokeCount());
//     testI = 0;
// }
// double currentRPM = (revCount - lastRev) / ((lastRevTime - lastRevReadTime()) / 1000.0) * 60;

//     if (lastStrokeTime != lastStrokeReadTime)
//     {
//         strokeRate = (strokeCount - strokeSinceLastRead) / ((lastStrokeTime - lastStrokeReadTime) / 1000.0) * 60;
//         lastStrokeReadTime = lastStrokeTime;
//         strokeSinceLastRead = strokeCount;
//     }
//     Serial.print("count: ");
//     Serial.println(revCount);
//     Serial.print("stroke rate: ");
//     Serial.println(strokeRate);
//     Serial.print("RPM: ");
//     Serial.println((revCount - revSinceLastRead) / ((lastRevTime - lastRevReadTime) / 1000.0) * 60);
//     Serial.print("StrokeCount: ");
//     Serial.println(strokeCount);
//     lastRevReadTime = lastRevTime;
//     revSinceLastRead = revCount;

// if (now - strokeController.getLastRevTime() > testData[testI] * 1000)
// {
//     if (testI != sizeof(testData) / sizeof(testData[0]))
//     {
//         run();
//         testI++;
//     }
// }
// }
