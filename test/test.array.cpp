#include <numeric>

#include "globals.h"
#include "test.array.h"

using std::accumulate;

void simulateRotation()
{
    strokeService.processRotation(accumulate(testDeltaTimes, testDeltaTimes + i, 0));
    i++;
}