#pragma once

#include "../../src/rower/flywheel.service.h"
#include "../../src/rower/stroke.controller.h"
#include "../../src/rower/stroke.service.h"

extern FlywheelService flywheelService;
extern StrokeService strokeService;
extern StrokeController strokeController;

void attachRotationInterrupt();
void detachRotationInterrupt();

constexpr bool isOdd(unsigned long number)
{
    return number % 2 != 0;
};