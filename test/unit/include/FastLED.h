// NOLINTBEGIN
#pragma once

#include "fakeit.hpp"

/// RGB color channel orderings, used when instantiating controllers to determine
/// what order the controller should send data out in. The default ordering
/// is RGB.
/// Within this enum, the red channel is 0, the green channel is 1, and the
/// blue chanel is 2.
enum EOrder
{
    RGB = 0012, ///< Red,   Green, Blue  (0012)
    RBG = 0021, ///< Red,   Blue,  Green (0021)
    GRB = 0102, ///< Green, Red,   Blue  (0102)
    GBR = 0120, ///< Green, Blue,  Red   (0120)
    BRG = 0201, ///< Blue,  Red,   Green (0201)
    BGR = 0210  ///< Blue,  Green, Red   (0210)
};

class CLEDController
{
};

struct CRGB
{
    typedef enum
    {
        Black = 0x000000,
        Blue = 0x0000FF,
        Green = 0x008000,
        Red = 0xFF0000
    } HTMLColorCode;

    // NOLINTNEXTLINE
    void operator=(const HTMLColorCode rhs);
};

class MockCFastLED
{
public:
    virtual CLEDController &addLeds(struct CRGB *data, int nLedsOrOffset, int nLedsIfOffset = 0) = 0;
    virtual void clear(bool writeData = false) = 0;
    virtual void show() = 0;

    virtual void mockHelperSetColor(const CRGB::HTMLColorCode rhs) = 0;
};
extern fakeit::Mock<MockCFastLED> mockFastLED;

class CFastLED
{
public:
    template <template <unsigned char DATA_PIN, EOrder RGB_ORDER> class CHIPSET, unsigned char DATA_PIN, EOrder RGB_ORDER>
    CLEDController &addLeds(struct CRGB *data, int nLedsOrOffset, int nLedsIfOffset = 0)
    {
        return mockFastLED.get().addLeds(data, nLedsOrOffset, nLedsIfOffset);
    }

    void clear(bool writeData = false)
    {
        mockFastLED.get().clear(writeData);
    }

    void show()
    {
        mockFastLED.get().show();
    }
};

template <unsigned char DATA_PIN, EOrder RGB_ORDER>
class WS2812
{
};

extern CFastLED FastLED;
// NOLINTEND