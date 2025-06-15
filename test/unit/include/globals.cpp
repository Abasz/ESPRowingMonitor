#include "esp_mac.h"

#include "./globals.h"

fakeit::Mock<Globals> mockGlobals;

void attachRotationInterrupt()
{
    mockGlobals.get().attachRotationInterrupt();
}

void detachRotationInterrupt()
{
    mockGlobals.get().detachRotationInterrupt();
}

void restartWithDelay(const unsigned long millis)
{
    mockGlobals.get().restartWithDelay(millis);
}

std::string generateSerial()
{
    const unsigned char macAddressLength = 6U;
    std::array<unsigned char, macAddressLength> mac{};
    esp_read_mac(mac.data(), ESP_MAC_BT);

    const unsigned char addressStringSize = 6;
    std::string serial;
    serial.resize(addressStringSize);

    // Most efficient way to format without memory size issue with <format> header
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    snprintf(serial.data(), addressStringSize + 1,
             "%02X%02X%02X",
             mac[3], mac[4], mac[5]);

    return serial;
}