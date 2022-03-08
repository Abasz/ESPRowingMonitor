#pragma once

#include "../src/stroke.controller.h"

extern LinearRegressorService regressorService;
extern StrokeService strokeService;
extern StrokeController strokeController;

void attachRotationInterrupt();
void detachRotationInterrupt();