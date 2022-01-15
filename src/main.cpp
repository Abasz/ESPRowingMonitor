#include <Arduino.h>

#include "bluetooth.controller.h"
#include "stroke.controller.h"

constexpr unsigned int DEEP_SLEEP_TIMEOUT = 4 * 60 * 1000;

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

    pinMode(GPIO_NUM_33, INPUT_PULLUP);
    pinMode(GPIO_NUM_2, OUTPUT);

    esp_sleep_enable_ext1_wakeup(GPIO_SEL_33, ESP_EXT1_WAKEUP_ALL_LOW);
    gpio_hold_en(GPIO_NUM_33);
    gpio_deep_sleep_hold_en();

    Serial.println("Attach interrupt");
    attachInterrupt(digitalPinToInterrupt(GPIO_NUM_33), rotationInterrupt, RISING);

    bleController.begin();
    strokeController.begin();

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
    strokeController.readCscData();
    auto now = millis();

    // for (auto deltaTime : testDeltaRotations)
    // {
    //     delay(deltaTime / 1000);
    //     rotationInterrupt();
    // auto start = micros();

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
    bleController.checkConnectedDevices();

    if (now - strokeController.getLastRevTime() > DEEP_SLEEP_TIMEOUT)
    {
        Serial.println("Going to sleep mode");
        esp_deep_sleep_start();
    }
}