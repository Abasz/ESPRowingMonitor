#include <numeric>

#include "globals.h"
#include "test.array.h"

void simulateRotation()
{
    flywheelService.processRotation(std::accumulate(testDeltaTimes.begin(), testDeltaTimes.begin() + i, 0));
    i++;
}