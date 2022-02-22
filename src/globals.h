#pragma once

#include "bluetooth.controller.h"
#include "stroke.controller.h"
#include "power-manager.controller.h"

static unsigned long const MSECS_PER_SEC = 1000000;
static unsigned long const SECS_PER_MIN = 60;
static unsigned long const SECS_PER_HOUR = 3600;
static unsigned long const SECS_PER_DAY = 86400;

extern BluetoothService bleService;
extern BluetoothController bleController;
extern StrokeService strokeService;
extern StrokeController strokeController;
extern PowerManagerController powerManagerController;

IRAM_ATTR void rotationInterrupt();

void attachRotationInterrupt();
void detachRotationInterrupt();

void printPrefix(Print *_logOutput, int logLevel);
void printTimestamp(Print *_logOutput);
void printLogLevel(Print *_logOutput, int logLevel);