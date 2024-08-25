#include <numeric>

#include "./globals.h"

void simulateRotation(const unsigned long now)
{
    flywheelService.processRotation(now);
}