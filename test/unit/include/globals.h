#pragma once

#include "./fakeit.hpp"

class Globals
{
protected:
    ~Globals() = default;

public:
    virtual void attachRotationInterrupt() = 0;
    virtual void detachRotationInterrupt() = 0;
    virtual void abort(int errorCode) = 0;
};

extern fakeit::Mock<Globals> mockGlobals;

void attachRotationInterrupt();
void detachRotationInterrupt();

consteval bool isOdd(unsigned long number)
{
    return number % 2 != 0;
};