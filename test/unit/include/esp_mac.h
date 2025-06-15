// NOLINTBEGIN
#pragma once

#include "esp_err.h"

typedef enum
{
    ESP_MAC_WIFI_STA,      /**< MAC for WiFi Station (6 bytes) */
    ESP_MAC_WIFI_SOFTAP,   /**< MAC for WiFi Soft-AP (6 bytes) */
    ESP_MAC_BT,            /**< MAC for Bluetooth (6 bytes) */
    ESP_MAC_ETH,           /**< MAC for Ethernet (6 bytes) */
    ESP_MAC_IEEE802154,    /**< if CONFIG_SOC_IEEE802154_SUPPORTED=y, MAC for IEEE802154 (8 bytes) */
    ESP_MAC_BASE,          /**< Base MAC for that used for other MAC types (6 bytes) */
    ESP_MAC_EFUSE_FACTORY, /**< MAC_FACTORY eFuse which was burned by Espressif in production (6 bytes) */
    ESP_MAC_EFUSE_CUSTOM,  /**< MAC_CUSTOM eFuse which was can be burned by customer (6 bytes) */
    ESP_MAC_EFUSE_EXT,     /**< if CONFIG_SOC_IEEE802154_SUPPORTED=y, MAC_EXT eFuse which is used as an extender for IEEE802154 MAC (2 bytes) */
} esp_mac_type_t;

esp_err_t esp_read_mac(uint8_t *mac, esp_mac_type_t type)
{
    const unsigned char macAddressLength = 6U;
    std::array<unsigned char, macAddressLength> base_mac_addr{0x01, 0x11, 0x10, 0x09, 0x08, 0x07};
    memcpy(mac, base_mac_addr.data(), macAddressLength);

    return ESP_OK;
}
// NOLINTEND