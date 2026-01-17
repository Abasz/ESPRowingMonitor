#pragma once

#include "../../utils/enums.h"

class ILedService
{
protected:
    ~ILedService() = default;

public:
    virtual void setColor(LedColor color) = 0;
    [[nodiscard]] virtual LedColor getColor() const = 0;
    virtual void refresh() = 0;
    virtual void clear() = 0;
};
