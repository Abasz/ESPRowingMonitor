#pragma once

typedef enum gpio_num_t
{
    GPIO_NUM_NC = -1,
    GPIO_NUM_0 = 0,
    GPIO_NUM_1 = 1,
    GPIO_NUM_2 = 2,
    GPIO_NUM_3 = 3,
    GPIO_NUM_4 = 4,
    GPIO_NUM_5 = 5,
    GPIO_NUM_6 = 6,
    GPIO_NUM_7 = 7,
    GPIO_NUM_8 = 8,
    GPIO_NUM_9 = 9,
    GPIO_NUM_10 = 10,
    GPIO_NUM_11 = 11,
    GPIO_NUM_12 = 12,
    GPIO_NUM_13 = 13,
    GPIO_NUM_14 = 14,
    GPIO_NUM_15 = 15,
    GPIO_NUM_16 = 16,
    GPIO_NUM_17 = 17,
    GPIO_NUM_18 = 18,
    GPIO_NUM_19 = 19,
    GPIO_NUM_20 = 20,
    GPIO_NUM_21 = 21,
    GPIO_NUM_22 = 22,
    GPIO_NUM_23 = 23,
    GPIO_NUM_25 = 25,
    GPIO_NUM_26 = 26,
    GPIO_NUM_27 = 27,
    GPIO_NUM_28 = 28,
    GPIO_NUM_29 = 29,
    GPIO_NUM_30 = 30,
    GPIO_NUM_31 = 31,
    GPIO_NUM_32 = 32,
    GPIO_NUM_33 = 33,
    GPIO_NUM_34 = 34,
    GPIO_NUM_35 = 35,
    GPIO_NUM_36 = 36,
    GPIO_NUM_37 = 37,
    GPIO_NUM_38 = 38,
    GPIO_NUM_39 = 39,
    GPIO_NUM_MAX,
} gpio_num_t;

typedef enum
{
    ESP_SLEEP_WAKEUP_UNDEFINED,       //!< In case of deep sleep, reset was not caused by exit from deep sleep
    ESP_SLEEP_WAKEUP_ALL,             //!< Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source
    ESP_SLEEP_WAKEUP_EXT0,            //!< Wakeup caused by external signal using RTC_IO
    ESP_SLEEP_WAKEUP_EXT1,            //!< Wakeup caused by external signal using RTC_CNTL
    ESP_SLEEP_WAKEUP_TIMER,           //!< Wakeup caused by timer
    ESP_SLEEP_WAKEUP_TOUCHPAD,        //!< Wakeup caused by touchpad
    ESP_SLEEP_WAKEUP_ULP,             //!< Wakeup caused by ULP program
    ESP_SLEEP_WAKEUP_GPIO,            //!< Wakeup caused by GPIO (light sleep only on ESP32, S2 and S3)
    ESP_SLEEP_WAKEUP_UART,            //!< Wakeup caused by UART (light sleep only)
    ESP_SLEEP_WAKEUP_WIFI,            //!< Wakeup caused by WIFI (light sleep only)
    ESP_SLEEP_WAKEUP_COCPU,           //!< Wakeup caused by COCPU int
    ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG, //!< Wakeup caused by COCPU crash
    ESP_SLEEP_WAKEUP_BT,              //!< Wakeup caused by BT (light sleep only)
} esp_sleep_source_t;

typedef esp_sleep_source_t esp_sleep_wakeup_cause_t;

inline void esp_sleep_enable_ext0_wakeup(gpio_num_t gpio_num, int level) {}
inline void esp_deep_sleep_start() {}