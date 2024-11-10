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