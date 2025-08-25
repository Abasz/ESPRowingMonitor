#include "../esp_err.h"

#include "../Arduino.h"

esp_err_t rtc_gpio_pullup_en(gpio_num_t gpio_num)
{
    return mockArduino.get().rtc_gpio_pullup_en(gpio_num);
}