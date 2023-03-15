#include <numeric>

#include "globals.h"

void simulateRotation(unsigned long now)
{
    flywheelService.processRotation(now);
}