#include <Arduino.h>

#include "power-manager.controller.h"

PowerManagerController::PowerManagerController()
{
}

void PowerManagerController::begin() const
{
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_33, ESP_EXT1_WAKEUP_ALL_LOW);
    gpio_hold_en(GPIO_NUM_33);
    gpio_deep_sleep_hold_en();
}

void PowerManagerController::checkSleep(unsigned long lastRevTime, bool isDeviceConnected) const
{
    if (!isDeviceConnected && micros() - lastRevTime > DEEP_SLEEP_TIMEOUT)
    {
        Serial.println("Going to sleep mode");
        esp_deep_sleep_start();
    }
}