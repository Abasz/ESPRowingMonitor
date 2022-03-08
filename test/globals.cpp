#include "globals.h"

LinearRegressorService regressorService;
StrokeService strokeService(regressorService);
StrokeController strokeController(strokeService);

void attachRotationInterrupt()
{
}

void detachRotationInterrupt()
{
}