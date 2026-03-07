#pragma once

class NimBLEAttValue;
class NimBLECharacteristic;

class IOtaUpdaterService
{
protected:
    ~IOtaUpdaterService() = default;

public:
    virtual void begin(NimBLECharacteristic *newOtaTxCharacteristic) = 0;
    [[nodiscard]] virtual bool isUpdating() const = 0;
    virtual void onData(const NimBLEAttValue &data, unsigned short mtu) = 0;
};