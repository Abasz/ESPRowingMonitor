#pragma once

#include "../../src/rower/stroke.controller.h"

extern FlywheelService flywheelService;
extern StrokeService strokeService;
extern StrokeController strokeController;

void attachRotationInterrupt();
void detachRotationInterrupt();

constexpr bool isOdd(unsigned long number)
{
    return number % 2 != 0;
};