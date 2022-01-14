#include <Arduino.h>

#include "bluetooth.controller.h"
#include "stroke.controller.h"

BluetoothController bleController;
StrokeController strokeController;

IRAM_ATTR void rotationInterrupt()
{
    // execution time: 1-5
    strokeController.processRotation(micros());
}

void setup()
{
    Serial.begin(115200);

    bleController.begin();
    strokeController.begin();

    pinMode(GPIO_NUM_5, INPUT_PULLUP);
    Serial.println("Attach interrupt");
    attachInterrupt(digitalPinToInterrupt(GPIO_NUM_5), rotationInterrupt, RISING);
    bleController.setBattery(34);
    bleController.notifyCsc(0, 0, 0, 0);
}

auto lastStrokeCount = 0U;
// execution time
// - not connected 30 microsec
// - connected 1  1800-2350
// - connected 2 microsec 4000-4400
void loop()
{
    // for (auto deltaTime : testDeltaRotations)
    // {
    //     delay(deltaTime / 1000);
    //     rotationInterrupt();
    // auto start = micros();
    strokeController.readCscData();

    // execution time
    // - not connected 20-30 microsec
    // - connected 1900-2200 microsec
    // auto start = micros();
    if (strokeController.getLastRevTime() != strokeController.getLastRevReadTime())
    {
        if (strokeController.getStrokeCount() > lastStrokeCount)
        {

            Serial.print("dragFactor: ");
            Serial.println(strokeController.getDragCoefficient() * 1e6);
            // execution time
            // - not connected: 73
            // - connected: 2000-2600
            // auto start = micros();
            bleController.notifyDragFactor(static_cast<unsigned char>(strokeController.getDragCoefficient() * 1e6));
            lastStrokeCount = strokeController.getStrokeCount();
            // auto stop = micros();
            // Serial.print("notifyDragFactor: ");
            // Serial.println(stop - start);
        }

        // execution time
        // - not connected: 5 microsec
        // - connected: 1800-2100 microsec
        // auto start = micros();
        bleController.notifyCsc(
            strokeController.getLastRevTime(),
            strokeController.getRevCount(),
            strokeController.getLastStrokeTime(),
            strokeController.getStrokeCount());
        // auto stop = micros();
        // Serial.print("notifyCsc: ");
        // Serial.println(stop - start);

        strokeController.setLastRevReadTime();
        // auto stop = micros();
        // Serial.print("Main loop if statement: ");
        // Serial.println(stop - start);
    }
    // auto stop = micros();
    // Serial.print("Main loop: ");
    // Serial.println(stop - start);
    // }
}