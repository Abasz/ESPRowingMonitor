#pragma once

#include "NimBLEDevice.h"

class IConnectionManagerCallbacks : public NimBLEServerCallbacks
{
protected:
    ~IConnectionManagerCallbacks() = default;

public:
    virtual unsigned char getConnectionCount() const = 0;
};