#pragma once

#include "./fakeit.hpp"

class Globals
{
protected:
    ~Globals() = default;

public:
    virtual void attachRotationInterrupt() = 0;
    virtual void detachRotationInterrupt() = 0;
    virtual void restartWithDelay(unsigned long millis = 0) = 0;
};

extern fakeit::Mock<Globals> mockGlobals;

void attachRotationInterrupt();
void detachRotationInterrupt();
void restartWithDelay(unsigned long millis = 0);

consteval bool isOdd(unsigned long number)
{
    return number % 2 != 0;
};