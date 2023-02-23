#pragma once

#include "stroke.controller.h"

static unsigned long const msecsPerSec = 1000000;
static unsigned long const secsPerMin = 60;
static unsigned long const secsPerHour = 3600;
static unsigned long const secsPerDay = 86400;

extern LinearRegressorService regressorService;
extern StrokeService strokeService;
extern StrokeController strokeController;

IRAM_ATTR void rotationInterrupt();

void attachRotationInterrupt();
void detachRotationInterrupt();

void printPrefix(Print *_logOutput, int logLevel);
void printTimestamp(Print *_logOutput);
void printLogLevel(Print *_logOutput, int logLevel);