// NOLINTBEGIN(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while, clang-analyzer-cplusplus.NewDeleteLeaks, clang-analyzer-cplusplus.NewDelete, clang-analyzer-core.CallAndMessage)
#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "./include/Arduino.h"
#include "./include/led_strip.h"

#include "../../src/peripherals/led/led.service.h"
#include "../../src/utils/configuration.h"

using namespace fakeit;

TEST_CASE("LedService", "[peripheral]")
{
    mockArduino.Reset();
    mockLedStrip.Reset();

    SECTION("constructor should")
    {
        SECTION("create RMT LED strip device with correct configuration")
        {
            led_strip_config_t capturedConfig{};
            led_strip_rmt_config_t capturedRmtConfig{};
            bool constructorCalled = false;

            When(Method(mockLedStrip, led_strip_new_rmt_device))
                .AlwaysDo([&capturedConfig, &capturedRmtConfig, &constructorCalled](const led_strip_config_t *config, const led_strip_rmt_config_t *rmtConfig, led_strip_handle_t *)
                          {
                              capturedConfig = *config;
                              capturedRmtConfig = *rmtConfig;
                              constructorCalled = true;
                              return ESP_OK; });

            LedService ledService;

            REQUIRE(constructorCalled);
            REQUIRE(capturedConfig.strip_gpio_num == static_cast<int>(Configurations::ledPin));
            REQUIRE(capturedConfig.max_leds == 1);
            REQUIRE(capturedConfig.led_model == LED_MODEL_WS2812);
            REQUIRE(capturedConfig.flags.invert_out == false);

            REQUIRE(capturedRmtConfig.clk_src == RMT_CLK_SRC_DEFAULT);
            REQUIRE(capturedRmtConfig.flags.with_dma == true);
        }

        SECTION("configure color component format based on EOrder")
        {
            led_strip_config_t capturedConfig{};

            When(Method(mockLedStrip, led_strip_new_rmt_device))
                .AlwaysDo([&capturedConfig](const led_strip_config_t *config, const led_strip_rmt_config_t *, led_strip_handle_t *)
                          {
                              capturedConfig = *config;
                              return ESP_OK; });

            LedService ledService;

            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            const auto format = capturedConfig.color_component_format.format;
            REQUIRE(format.num_components == 3);
            REQUIRE(format.w_pos == 3);

            // Verify exact positions match the configuration
            const auto orderValue = std::to_underlying(Configurations::ledColorChannelOrder);
            const unsigned char expectedRPos = (orderValue >> 6U) & 0x3U;
            const unsigned char expectedGPos = (orderValue >> 3U) & 0x3U;
            const unsigned char expectedBPos = orderValue & 0x3U;

            REQUIRE(format.r_pos == expectedRPos);
            REQUIRE(format.g_pos == expectedGPos);
            REQUIRE(format.b_pos == expectedBPos);
        }
    }

    SECTION("setColor method should")
    {
        unsigned int capturedIndex = 999;
        unsigned int capturedRed = 0;
        unsigned int capturedGreen = 0;
        unsigned int capturedBlue = 0;

        When(Method(mockLedStrip, led_strip_new_rmt_device)).AlwaysReturn(ESP_OK);
        When(Method(mockLedStrip, led_strip_set_pixel))
            .AlwaysDo([&capturedIndex, &capturedRed, &capturedGreen, &capturedBlue](led_strip_handle_t, unsigned int index, unsigned int red, unsigned int green, unsigned int blue)
                      {
                          capturedIndex = index;
                          capturedRed = red;
                          capturedGreen = green;
                          capturedBlue = blue;
                          return ESP_OK; });
        LedService ledService;

        SECTION("set pixel with correct RGB values for Green")
        {
            ledService.setColor(LedColor::Green);

            REQUIRE(capturedIndex == 0);
            // Green = 0x008000 -> R=0, G=128, B=0
            REQUIRE(capturedRed == 0);
            REQUIRE(capturedGreen == 128);
            REQUIRE(capturedBlue == 0);
        }

        SECTION("set pixel with correct RGB values for Red")
        {
            ledService.setColor(LedColor::Red);

            REQUIRE(capturedIndex == 0);
            // Red = 0xFF0000 -> R=255, G=0, B=0
            REQUIRE(capturedRed == 255);
            REQUIRE(capturedGreen == 0);
            REQUIRE(capturedBlue == 0);
        }

        SECTION("set pixel with correct RGB values for Blue")
        {
            ledService.setColor(LedColor::Blue);

            REQUIRE(capturedIndex == 0);
            // Blue = 0x0000FF -> R=0, G=0, B=255
            REQUIRE(capturedRed == 0);
            REQUIRE(capturedGreen == 0);
            REQUIRE(capturedBlue == 255);
        }

        SECTION("set pixel with correct RGB values for Black")
        {
            ledService.setColor(LedColor::Black);

            REQUIRE(capturedIndex == 0);
            // Black = 0x000000 -> R=0, G=0, B=0
            REQUIRE(capturedRed == 0);
            REQUIRE(capturedGreen == 0);
            REQUIRE(capturedBlue == 0);
        }
    }

    SECTION("getColor method should")
    {
        SECTION("return initial color as Black")
        {
            When(Method(mockLedStrip, led_strip_new_rmt_device)).AlwaysReturn(ESP_OK);
            LedService ledService;
            REQUIRE(ledService.getColor() == LedColor::Black);
        }

        SECTION("return the last set color")
        {
            When(Method(mockLedStrip, led_strip_new_rmt_device)).AlwaysReturn(ESP_OK);
            When(Method(mockLedStrip, led_strip_set_pixel)).AlwaysReturn(ESP_OK);
            LedService ledService;

            ledService.setColor(LedColor::Green);
            REQUIRE(ledService.getColor() == LedColor::Green);

            ledService.setColor(LedColor::Red);
            REQUIRE(ledService.getColor() == LedColor::Red);
        }
    }

    SECTION("refresh method should")
    {
        bool refreshCalled = false;

        When(Method(mockLedStrip, led_strip_new_rmt_device)).AlwaysReturn(ESP_OK);
        When(Method(mockLedStrip, led_strip_refresh))
            .AlwaysDo([&refreshCalled](led_strip_handle_t)
                      {
                          refreshCalled = true;
                          return ESP_OK; });
        LedService ledService;

        SECTION("call led_strip_refresh")
        {
            ledService.refresh();

            REQUIRE(refreshCalled);
        }
    }

    SECTION("clear method should")
    {
        bool clearCalled = false;

        When(Method(mockLedStrip, led_strip_new_rmt_device)).AlwaysReturn(ESP_OK);
        When(Method(mockLedStrip, led_strip_clear))
            .AlwaysDo([&clearCalled](led_strip_handle_t)
                      {
                          clearCalled = true;
                          return ESP_OK; });
        LedService ledService;

        SECTION("call led_strip_clear")
        {
            ledService.clear();

            REQUIRE(clearCalled);
        }
    }
}
// NOLINTEND(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while, clang-analyzer-cplusplus.NewDeleteLeaks, clang-analyzer-cplusplus.NewDelete, clang-analyzer-core.CallAndMessage)