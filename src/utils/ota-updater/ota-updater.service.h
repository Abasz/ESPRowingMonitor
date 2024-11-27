#pragma once

#include <deque>
#include <span>

#include "NimBLEDevice.h"

#include "../enums.h"
#include "./ota-updater.service.interface.h"

using std::span;

class OtaUpdaterService final : public IOtaUpdaterService
{
    NimBLECharacteristic *otaTxCharacteristic = nullptr;

    std::deque<unsigned char> buffer;
    unsigned short mtu = 512;
    const unsigned char blePackageHeaderSize = 3;
    const unsigned char bufferCapacity = 40;

    void handleBegin(const span<const unsigned char> &payload);
    void handlePackage(const span<const unsigned char> &payload);
    void handleEnd(const span<const unsigned char> &payload);
    void handleInstall();
    void handleError(OtaResponseOpCodes errorCode);
    void send(unsigned char head);
    void send(const unsigned char *data, size_t messageLength);
    void setMtu(unsigned short newMtu);
    void terminateUpload();
    void flushBuffer();

public:
    OtaUpdaterService();

    bool isUpdating() const override;
    void onData(const NimBLEAttValue &data, unsigned short newMtu) override;
    void begin(NimBLECharacteristic *otaTxCharacteristic) override;
};