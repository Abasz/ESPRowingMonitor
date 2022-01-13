#include <Arduino.h>

#include "bluetooth.controller.h"
#include "stroke.controller.h"

BluetoothController bleController;
StrokeController strokeController;

IRAM_ATTR void rotationInterrupt()
{
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
// - not connected 10 microsec
// - connected microsec 1100-1500 microsec
void loop()
{
    // auto start = micros();

    auto lastRevTime = strokeController.getLastRevTime();

    // execution time
    // - not connected 20-30 microsec
    // - connected 1200-1700 microsec
    // auto start = micros();
    if (lastRevTime != strokeController.getLastRevReadTime())
    {
        auto data = strokeController.getCscData();

        if (data.strokeCount > lastStrokeCount)
        {
            Serial.print("dragFactor: ");
            Serial.println(data.dragFactor);
            bleController.notifyDragFactor(static_cast<unsigned char>(data.dragFactor));
            lastStrokeCount = data.strokeCount;
        }
        // Serial.print("deltaTime: ");
        Serial.println(data.deltaTime);
        // Serial.print("deltaTimeDiff: ");
        // Serial.println(data.deltaTimeDiff);

        // execution time
        // - not connected: 5 microsec
        // - connected: 1200-1700 microsec
        // auto start = micros();
        bleController.notifyCsc(
            data.lastRevTime,
            data.revCount,
            data.lastStrokeTime,
            data.strokeCount);
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
}
