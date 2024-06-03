#pragma once

#include "ArduinoLog.h"
#include "Preferences.h"

#include "./EEPROM.service.interface.h"
#include "./configuration.h"
#include "./enums.h"

class EEPROMService final : public IEEPROMService
{
    Preferences &preferences;

    static constexpr const char *logLevelAddress = "logLevel";
    static constexpr const char *bluetoothDeltaTimeLoggingAddress = "bleLogging";
    static constexpr const char *sdCardLoggingAddress = "sdCardLogging";
    static constexpr const char *bleServiceFlagAddress = "bleService";

    ArduinoLogLevel logLevel = Configurations::defaultLogLevel;
    bool logToBluetooth = Configurations::enableBluetoothDeltaTimeLogging;
    bool logToSdCard = false;
    BleServiceFlag bleServiceFlag = Configurations::defaultBleServiceFlag;

public:
    explicit EEPROMService(Preferences &_preferences);

    void setup() override;

    void setLogLevel(ArduinoLogLevel newLogLevel) override;
    void setLogToBluetooth(bool shouldLogToBluetooth) override;
    void setLogToSdCard(bool shouldLogToSdCard) override;
    void setBleServiceFlag(BleServiceFlag newServiceFlag) override;

    BleServiceFlag getBleServiceFlag() const override;
    ArduinoLogLevel getLogLevel() const override;
    bool getLogToBluetooth() const override;
    bool getLogToSdCard() const override;
};