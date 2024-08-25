#include "./fakeit.hpp"

#include "./FastLED.h"

fakeit::Mock<MockCFastLED> mockFastLED;

// NOLINTNEXTLINE
void CRGB::operator=(const HTMLColorCode rhs)
{
    mockFastLED.get().mockHelperSetColor(rhs);
}