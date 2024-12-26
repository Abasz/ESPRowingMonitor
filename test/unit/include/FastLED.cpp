// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access, readability-magic-numbers)
#include "./fakeit.hpp"

#include "./FastLED.h"
#include "FastLED.h"

fakeit::Mock<MockCFastLED> mockFastLED;
CFastLED FastLED;

CRGB &CRGB::operator=(const unsigned int rhs)
{
    mockFastLED.get().mockHelperSetColor(rhs);

    r = (rhs >> 16) & 0xFF;
    g = (rhs >> 8) & 0xFF;
    b = (rhs >> 0) & 0xFF;

    return *this;
}

bool operator==(const CRGB &lhs, const CRGB &rhs)
{
    return (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b);
}
// NOLINTEND(cppcoreguidelines-pro-type-union-access, readability-magic-numbers)