#pragma once

void attachRotationInterrupt();
void detachRotationInterrupt();

constexpr bool isOdd(unsigned long number)
{
    return number % 2 != 0;
};