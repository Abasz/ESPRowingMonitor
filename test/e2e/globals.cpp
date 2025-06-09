#include "./globals.h"

#include "fakeit.hpp"

#include "../../src/utils/EEPROM/EEPROM.service.interface.h"

fakeit::Mock<IEEPROMService> mockEEPROMService;

FlywheelService flywheelService;
StrokeService strokeService;
StrokeController strokeController(strokeService, flywheelService, mockEEPROMService.get());

void attachRotationInterrupt()
{
}

void detachRotationInterrupt()
{
}