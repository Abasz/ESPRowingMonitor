#include "./globals.h"

fakeit::Mock<Globals> mockGlobals;

void attachRotationInterrupt()
{
    mockGlobals.get().attachRotationInterrupt();
}

void detachRotationInterrupt()
{
    mockGlobals.get().detachRotationInterrupt();
}

void restartWithDelay(const unsigned long millis)
{
    mockGlobals.get().restartWithDelay(millis);
}