#include "./globals.h"

FlywheelService flywheelService;
StrokeService strokeService;
StrokeController strokeController(strokeService, flywheelService);

void attachRotationInterrupt()
{
}

void detachRotationInterrupt()
{
}