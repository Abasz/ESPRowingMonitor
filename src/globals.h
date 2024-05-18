#pragma once

#include "Preferences.h"

#include "peripherals/peripherals.controller.h"
#include "rower/stroke.controller.h"
#include "utils/EEPROM.service.h"
#include "utils/power-manager.controller.h"

static unsigned long lastUpdateTime = 0UL;

static const unsigned long msecsPerSec = 1'000'000;
static const unsigned long secsPerMin = 60;
static const unsigned long secsPerHour = 3'600;
static const unsigned long secsPerDay = 86'400;

extern Preferences preferences;
extern EEPROMService eepromService;
extern BluetoothService bleService;
extern NetworkService networkService;
extern SdCardService sdCardService;
extern PeripheralsController peripheralController;
extern FlywheelService flywheelService;
extern StrokeService strokeService;
extern StrokeController strokeController;
extern PowerManagerController powerManagerController;

IRAM_ATTR void rotationInterrupt();

void attachRotationInterrupt();
void detachRotationInterrupt();

void printPrefix(Print *_logOutput, int logLevel);
void printTimestamp(Print *_logOutput);
void printLogLevel(Print *_logOutput, int logLevel);

constexpr bool isOdd(unsigned long number)
{
    return number % 2 != 0;
};