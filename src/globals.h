#pragma once

#include <Preferences.h>

#include "EEPROM.service.h"
#include "bluetooth.controller.h"
#include "power-manager.controller.h"
#include "stroke.controller.h"

static unsigned long lastUpdateTime = 0UL;

static unsigned long const msecsPerSec = 1000000;
static unsigned long const secsPerMin = 60;
static unsigned long const secsPerHour = 3600;
static unsigned long const secsPerDay = 86400;

extern Preferences preferences;
extern EEPROMService eepromService;
extern BluetoothService bleService;
extern BluetoothController bleController;
extern LinearRegressorService regressorService;
extern StrokeService strokeService;
extern StrokeController strokeController;
extern PowerManagerController powerManagerController;

IRAM_ATTR void rotationInterrupt();

void attachRotationInterrupt();
void detachRotationInterrupt();

void printPrefix(Print *_logOutput, int logLevel);
void printTimestamp(Print *_logOutput);
void printLogLevel(Print *_logOutput, int logLevel);