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
    attachInterrupt(digitalPinToInterrupt(GPIO_NUM_33), rotationInterrupt, RISING);
}

void detachRotationInterrupt()
{
    detachInterrupt(digitalPinToInterrupt(GPIO_NUM_33));
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
