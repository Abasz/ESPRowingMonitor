#pragma once

#include "Preferences.h"

#include "peripherals/peripherals.controller.h"
#include "rower/stroke.controller.h"
#include "utils/EEPROM.service.h"
#include "utils/power-manager.controller.h"

static unsigned long lastUpdateTime = 0UL;

static unsigned long const msecsPerSec = 1000000;
static unsigned long const secsPerMin = 60;
static unsigned long const secsPerHour = 3600;
static unsigned long const secsPerDay = 86400;

extern Preferences preferences;
extern EEPROMService eepromService;
extern BluetoothService bleService;
extern NetworkService networkService;
extern PeripheralsController bleController;
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