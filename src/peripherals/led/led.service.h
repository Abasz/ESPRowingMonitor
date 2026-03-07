#pragma once

#include "led_strip.h"

#include "../../utils/enums.h"
#include "./led.service.interface.h"

class LedService final : public ILedService
{
    led_strip_handle_t ledStripHandle = nullptr;
    LedColor currentColor = LedColor::Black;

    [[nodiscard]] static constexpr led_color_component_format_t getColorFormat();

public:
    LedService();
    void setColor(LedColor color) override;
    [[nodiscard]] LedColor getColor() const override;
    void refresh() override;
    void clear() override;
};
