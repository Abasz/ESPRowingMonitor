#include "./fakeit.hpp"

#include "./FastLED.h"

fakeit::Mock<MockCFastLED> mockFastLED;
CFastLED FastLED;

// NOLINTNEXTLINE
void CRGB::operator=(const HTMLColorCode rhs)
{
    mockFastLED.get().mockHelperSetColor(rhs);
}