// NOLINTBEGIN
#pragma once

#include "Esp32-typedefs.h"

#define OUTPUT 0x03
#define INPUT_PULLUP 0x05
#define LOW 0x00
#define HIGH 0x01
#define PI 3.1415926535897932384626433832795
#define LED_BUILTIN GPIO_NUM_2

inline void pinMode(unsigned char pin, unsigned char mode) {}
inline void digitalWrite(unsigned char pin, unsigned char val) {}
inline unsigned long micros()
{
    return 0;
}
inline int digitalRead(unsigned char pin)
{
    return 0;
}
inline void gpio_hold_en(gpio_num_t gpio_num) {}
inline unsigned short analogRead(unsigned char pin) { return 0; }
inline void delay(unsigned int) {}
inline esp_sleep_wakeup_cause_t esp_sleep_get_wakeup_cause()
{
    return ESP_SLEEP_WAKEUP_EXT1;
}

class HardwareSerial
{
public:
    inline void begin(unsigned long baud, unsigned int config, char rxPin = -1, char txPin = -1, bool invert = false, unsigned long timeout_ms = 20000UL, unsigned char rxfifo_full_thrhd = 112){};
    inline void end(bool fullyTerminate = true){};
    inline void updateBaudRate(unsigned long baud){};
    inline int available(void) { return 1; };
    inline int availableForWrite(void) { return 1; };
    inline void flush(void){};
};
// NOLINTEND