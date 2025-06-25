#pragma once

#include "NimBLEDevice.h"

class IServerCallbacks : public NimBLEServerCallbacks
{
protected:
    ~IServerCallbacks() = default;

public:
    virtual unsigned char getConnectionCount() const = 0;
};