#pragma once

#include <concepts>

#include "fakeit.hpp"

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

std::string generateSerial();

consteval bool isOdd(unsigned long number)
{
    return number % 2 != 0;
};

template <typename T>
    requires std::is_arithmetic_v<T>
constexpr bool
isInBounds(const T &value, const T &lower, const T &upper)
{
    return value >= lower && value <= upper;
}