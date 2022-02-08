#include <numeric>

#include "globals.h"
#include "test.array.h"

using std::accumulate;

void simulatRotation()
{
    i++;
    strokeService.processRotation(accumulate(testDeltaTimes, testDeltaTimes + i + 1, 0));
}