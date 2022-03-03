#include <numeric>

#include "globals.h"
#include "test.array.h"

using std::accumulate;

void simulatRotation()
{
    strokeService.processRotation(accumulate(testDeltaTimes, testDeltaTimes + i, 0));
    i++;
}