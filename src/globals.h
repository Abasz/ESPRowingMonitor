#pragma once

#include <Arduino.h>

#include "bluetooth.controller.h"
#include "stroke.controller.h"
#include "power-manager.controller.h"

// Division constants
static unsigned long const MSECS_PER_SEC = 1000000;
static unsigned long const SECS_PER_MIN = 60;
static unsigned long const SECS_PER_HOUR = 3600;
static unsigned long const SECS_PER_DAY = 86400;

extern BluetoothService bleService;
extern BluetoothController bleController;
extern StrokeService strokeService;
extern StrokeController strokeController;
extern PowerManagerController powerManagerController;

IRAM_ATTR void rotationInterrupt();

void attachRotationInterrupt();
void detachRotationInterrupt();

void printPrefix(Print *_logOutput, int logLevel);
void printTimestamp(Print *_logOutput);
void printLogLevel(Print *_logOutput, int logLevel);

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
