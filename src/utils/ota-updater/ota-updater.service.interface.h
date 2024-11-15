#pragma once

#include "NimBLEDevice.h"

class IOtaUpdaterService
{
protected:
    ~IOtaUpdaterService() = default;

public:
    virtual void begin(NimBLECharacteristic *newOtaTxCharacteristic) = 0;
    virtual bool isUpdating() const = 0;
    virtual void onData(const NimBLEAttValue &data, unsigned short mtu) = 0;
};