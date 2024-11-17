#pragma once

#include "NimBLEDevice.h"

#include "../../../utils/EEPROM/EEPROM.service.interface.h"
#include "../../../utils/enums.h"
#include "../bluetooth.controller.interface.h"
#include "../callbacks/control-point.callbacks.h"

class IBaseMetricsBleService
{
protected:
    ~IBaseMetricsBleService() = default;

public:
    virtual NimBLEService *setup(NimBLEServer *server, BleServiceFlag bleServiceFlag) = 0;

    virtual void broadcastBaseMetrics(unsigned short revTime, unsigned int revCount, unsigned short strokeTime, unsigned short strokeCount, short avgStrokePower) = 0;

    virtual bool isSubscribed() = 0;
};