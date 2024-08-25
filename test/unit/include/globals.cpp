#include "Arduino.h"
#include "FastLED.h"

#include "globals.h"

HardwareSerial Serial;
CFastLED FastLED;

fakeit::Mock<Globals> mockGlobals;

void attachRotationInterrupt()
{
    mockGlobals.get().attachRotationInterrupt();
}

void detachRotationInterrupt()
{
    mockGlobals.get().detachRotationInterrupt();
}