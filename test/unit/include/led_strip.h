// NOLINTBEGIN
#pragma once

#include <cstddef>

#include "fakeit.hpp"

#include "esp_err.h"

using led_strip_handle_t = struct led_strip_t *;

struct led_strip_rmt_config_t
{
    int clk_src;
    size_t mem_block_symbols;
    struct
    {
        bool with_dma : 1;
    } flags;
};

enum led_model_t
{
    LED_MODEL_WS2812,
    LED_MODEL_SK6812,
    LED_MODEL_WS2811,
    LED_MODEL_INVALID
};

struct format_layout
{
    unsigned int r_pos : 2;
    unsigned int g_pos : 2;
    unsigned int b_pos : 2;
    unsigned int w_pos : 2;
    unsigned int reserved : 21;
    unsigned int num_components : 3;
};

union led_color_component_format_t
{
    struct format_layout format;
    unsigned int format_id;
};

struct led_strip_config_t
{
    int strip_gpio_num;
    unsigned int max_leds;
    led_model_t led_model;
    led_color_component_format_t color_component_format;
    struct
    {
        bool invert_out : 1;
    } flags;
};

#define RMT_CLK_SRC_DEFAULT 0

class MockLedStrip
{
protected:
    ~MockLedStrip() = default;

public:
    virtual esp_err_t led_strip_new_rmt_device(
        const led_strip_config_t *led_config,
        const led_strip_rmt_config_t *rmt_config,
        led_strip_handle_t *ret_strip) = 0;

    virtual esp_err_t led_strip_set_pixel(
        led_strip_handle_t strip,
        unsigned int index,
        unsigned int red,
        unsigned int green,
        unsigned int blue) = 0;

    virtual esp_err_t led_strip_refresh(led_strip_handle_t strip) = 0;

    virtual esp_err_t led_strip_clear(led_strip_handle_t strip) = 0;
};

extern fakeit::Mock<MockLedStrip> mockLedStrip;

inline esp_err_t led_strip_new_rmt_device(
    const led_strip_config_t *led_config,
    const led_strip_rmt_config_t *rmt_config,
    led_strip_handle_t *ret_strip)
{
    return mockLedStrip.get().led_strip_new_rmt_device(led_config, rmt_config, ret_strip);
}

inline esp_err_t led_strip_set_pixel(
    led_strip_handle_t strip,
    unsigned int index,
    unsigned int red,
    unsigned int green,
    unsigned int blue)
{
    return mockLedStrip.get().led_strip_set_pixel(strip, index, red, green, blue);
}

inline esp_err_t led_strip_refresh(led_strip_handle_t strip)
{
    return mockLedStrip.get().led_strip_refresh(strip);
}

inline esp_err_t led_strip_clear(led_strip_handle_t strip)
{
    return mockLedStrip.get().led_strip_clear(strip);
}

// NOLINTEND
