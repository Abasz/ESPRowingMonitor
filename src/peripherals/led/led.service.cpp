#include <utility>

#include "soc/gpio_num.h"

#include "Arduino.h"
#include "led_strip.h"

#include "./led.service.h"

#include "../../utils/configuration.h"
#include "../../utils/enums.h"

LedService::LedService()
{
    if constexpr (Configurations::ledPin == GPIO_NUM_NC)
    {
        return;
    }

    if constexpr (Configurations::isRgb)
    {
        const led_strip_config_t stripConfig = {
            .strip_gpio_num = static_cast<int>(Configurations::ledPin),
            .max_leds = 1,
            .led_model = LED_MODEL_WS2812,
            .color_component_format = getColorFormat(),
            .flags = {
                .invert_out = false,
            },
        };

        const led_strip_rmt_config_t rmtConfig = {
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .mem_block_symbols = 0,
            .flags = {
                .with_dma = Configurations::rmtDMAsupported,
            },
        };

        led_strip_new_rmt_device(&stripConfig, &rmtConfig, &ledStripHandle);
    }
    else
    {
        pinMode(Configurations::ledPin, OUTPUT);
    }
}

void LedService::setColor(const LedColor color)
{
    currentColor = color;

    if constexpr (!Configurations::isRgb || Configurations::ledPin == GPIO_NUM_NC)
    {
        return;
    }

    const auto colorValue = static_cast<unsigned int>(color);
    const auto red = (colorValue >> 16U) & 0xFFU;
    const auto green = (colorValue >> 8U) & 0xFFU;
    const auto blue = colorValue & 0xFFU;

    led_strip_set_pixel(ledStripHandle, 0, red, green, blue);
}

LedColor LedService::getColor() const
{
    return currentColor;
}

void LedService::refresh()
{
    if constexpr (Configurations::ledPin == GPIO_NUM_NC)
    {
        return;
    }

    if constexpr (Configurations::isRgb)
    {
        led_strip_refresh(ledStripHandle);
    }
    else
    {
        digitalWrite(Configurations::ledPin, currentColor == LedColor::Black ? LOW : HIGH);
    }
}

void LedService::clear()
{
    if constexpr (Configurations::ledPin == GPIO_NUM_NC)
    {
        return;
    }

    if constexpr (Configurations::isRgb)
    {
        led_strip_clear(ledStripHandle);
    }
    else
    {
        digitalWrite(Configurations::ledPin, LOW);
    }
}

// Map FastLED EOrder to led_strip color format
// The octal digits represent positions: hundreds=R, tens=G, ones=B
constexpr led_color_component_format_t LedService::getColorFormat()
{
    constexpr auto order = std::to_underlying(Configurations::ledColorChannelOrder);

    // Extract positions from EOrder octal encoding
    constexpr unsigned char rPos = (order >> 6U) & 0x3U;
    constexpr unsigned char gPos = (order >> 3U) & 0x3U;
    constexpr unsigned char bPos = order & 0x3U;

    return led_color_component_format_t{
        .format = {
            .r_pos = rPos,
            .g_pos = gPos,
            .b_pos = bPos,
            .w_pos = 3,
            .reserved = 0,
            .num_components = 3,
        },
    };
}
