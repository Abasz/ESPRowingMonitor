#include <numeric>

#include "globals.h"
#include "test.array.h"

using std::accumulate;

void simulatRotation()
{
    // strokeService.processRotation(testDeltaTimes[i]);
    strokeService.processRotation(accumulate(testDeltaTimes, testDeltaTimes + i, 0));
    i++;
}