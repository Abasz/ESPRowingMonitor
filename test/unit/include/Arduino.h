// NOLINTBEGIN
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

#include "fakeit.hpp"

#include "./soc/gpio_num.h"

using BaseType_t = int;
using UBaseType_t = unsigned int;
using TaskFunction_t = void (*)(void *);
using TaskHandle_t = void *;
using voidFuncPtr = void (*)(void);

struct hw_timer_t;

#define FALLING 0x02
#define INPUT 0x01
#define OUTPUT 0x03
#define INPUT_PULLUP 0x05
#define LOW 0x00
#define HIGH 0x01
#define PI 3.1415926535897932384626433832795
#define LED_BUILTIN GPIO_NUM_2
#define IRAM_ATTR

class Print
{
private:
public:
    Print() {}

    size_t print(const char[])
    {
        return 0;
    };
};

class MockArduino
{
protected:
    ~MockArduino() = default;

public:
    virtual void abort(int errorCode) = 0;
    virtual unsigned long analogReadMilliVolts(unsigned char pin) = 0;
    virtual void pinMode(unsigned char pin, unsigned char mode) = 0;
    virtual void digitalWrite(unsigned char pin, unsigned char val) = 0;
    virtual unsigned long micros() = 0;
    virtual unsigned long millis() = 0;
    virtual int digitalRead(unsigned char pin) = 0;
    virtual void gpio_hold_en(gpio_num_t gpio_num) = 0;
    virtual void gpio_deep_sleep_hold_en() = 0;
    virtual int rtc_gpio_pullup_en(gpio_num_t gpio_num) = 0;
    virtual unsigned short analogRead(unsigned char pin) = 0;
    virtual void delay(unsigned int delay) = 0;
    virtual esp_sleep_wakeup_cause_t esp_sleep_get_wakeup_cause() = 0;
    virtual void esp_sleep_enable_ext0_wakeup(gpio_num_t gpio_num, int level) = 0;
    virtual void esp_deep_sleep_start() = 0;
    virtual BaseType_t xTaskCreatePinnedToCore(TaskFunction_t pvTaskCode,
                                               const char *const pcName,
                                               const unsigned int usStackDepth,
                                               void *const pvParameters,
                                               UBaseType_t uxPriority,
                                               TaskHandle_t *const pvCreatedTask,
                                               const BaseType_t xCoreID) = 0;
    virtual void vTaskDelete(TaskHandle_t xTaskToDelete) = 0;
};

extern fakeit::Mock<MockArduino> mockArduino;

inline unsigned char digitalPinToInterrupt(gpio_num_t pin)
{
    return static_cast<unsigned char>(pin);
}
inline void attachInterrupt(unsigned char pin, std::function<void(void)> intRoutine, int mode) {}
inline void detachInterrupt(unsigned char pin) {};

inline unsigned long analogReadMilliVolts(unsigned char pin)
{
    return mockArduino.get().analogReadMilliVolts(pin);
};
inline void pinMode(unsigned char pin, unsigned char mode)
{
    mockArduino.get().pinMode(pin, mode);
}
inline void digitalWrite(unsigned char pin, unsigned char val)
{
    mockArduino.get().digitalWrite(pin, val);
}
inline unsigned long micros()
{
    return mockArduino.get().micros();
}
inline unsigned long millis()
{
    return mockArduino.get().millis();
}
inline int digitalRead(unsigned char pin)
{
    return mockArduino.get().digitalRead(pin);
}
inline void gpio_hold_en(gpio_num_t gpio_num)
{
    mockArduino.get().gpio_hold_en(gpio_num);
}
inline unsigned short analogRead(unsigned char pin)
{
    return mockArduino.get().analogRead(pin);
}
inline void delay(unsigned int delay)
{
    mockArduino.get().delay(delay);
}
inline esp_sleep_wakeup_cause_t esp_sleep_get_wakeup_cause()
{
    return mockArduino.get().esp_sleep_get_wakeup_cause();
}

inline void esp_sleep_enable_ext0_wakeup(gpio_num_t gpio_num, int level)
{
    mockArduino.get().esp_sleep_enable_ext0_wakeup(gpio_num, level);
}
inline void esp_deep_sleep_start()
{
    mockArduino.get().esp_deep_sleep_start();
}

inline void vTaskDelete(TaskHandle_t xTaskToDelete)
{
    mockArduino.get().vTaskDelete(xTaskToDelete);
}

inline BaseType_t xTaskCreatePinnedToCore(TaskFunction_t pvTaskCode,
                                          const char *const pcName,
                                          const uint32_t usStackDepth,
                                          void *const pvParameters,
                                          UBaseType_t uxPriority,
                                          TaskHandle_t *const pvCreatedTask,
                                          const BaseType_t xCoreID)
{
    pvTaskCode(pvParameters);
    return mockArduino.get().xTaskCreatePinnedToCore(
        pvTaskCode,
        pcName,
        usStackDepth,
        pvParameters,
        uxPriority,
        pvCreatedTask,
        xCoreID);
}

inline void timerAttachInterrupt(hw_timer_t *timer, voidFuncPtr userFunc) {}
inline void timerAlarm(hw_timer_t *timer, unsigned long long alarm_value, bool autoreload, unsigned long long reload_count) {}
inline hw_timer_t *timerBegin(uint32_t frequency)
{
    return nullptr;
}

inline void esp_restart() {}

class HardwareSerial : public Print
{
protected:
    ~HardwareSerial() = default;

public:
    virtual inline void begin(unsigned long baud, unsigned int config = 134217756, char rxPin = -1, char txPin = -1, bool invert = false, unsigned long timeout_ms = 20'000UL, unsigned char rxfifo_full_thrhd = 112) {}
    virtual inline void end(bool fullyTerminate = true) {}
    virtual inline void updateBaudRate(unsigned long baud) {}
    virtual inline int available() { return 0; }
    virtual inline int availableForWrite() { return 0; }
    inline void printf(const std::string __fmt, ...) {};
    virtual inline void flush() {}
    inline operator bool()
    {
        return static_cast<bool>(available());
    }
};
extern fakeit::Mock<HardwareSerial> mockSerial;
extern HardwareSerial &Serial;

#pragma GCC diagnostic pop
// NOLINTEND