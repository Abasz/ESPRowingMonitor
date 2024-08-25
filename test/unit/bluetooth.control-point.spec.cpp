#include <array>
#include <string>

#include "catch_amalgamated.hpp"
#include "fakeit.hpp"

#include "Arduino.h"
#include "NimBLEDevice.h"

#include "../../src/peripherals/bluetooth.service.h"
#include "../../src/peripherals/sd-card.service.interface.h"
#include "../../src/utils/EEPROM.service.interface.h"
#include "../../src/utils/configuration.h"
#include "../../src/utils/enums.h"

using namespace fakeit;

TEST_CASE("BluetoothServer ControlPointCallbacks onWrite method should", "[callbacks]")
{
    Mock<IEEPROMService> mockEEPROMService;
    Mock<ISdCardService> mockSdCardService;
    Mock<NimBLECharacteristic> mockControlPointCharacteristic;

    mockArduino.Reset();
    mockNimBLEServer.Reset();
    mockNimBLEAdvertising.Reset();
    mockNimBLEService.Reset();
    mockNimBLECharacteristic.Reset();

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string))).AlwaysReturn(&mockNimBLEService.get());
    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const unsigned short))).AlwaysReturn(&mockNimBLEService.get());
    Fake(Method(mockNimBLEServer, createServer));
    Fake(Method(mockNimBLEServer, init));
    Fake(Method(mockNimBLEServer, setPower));
    Fake(Method(mockNimBLEServer, start));

    When(OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int))).AlwaysReturn(&mockNimBLECharacteristic.get());
    When(OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))).AlwaysReturn(&mockNimBLECharacteristic.get());
    When(Method(mockNimBLEService, getServer)).AlwaysReturn(&mockNimBLEServer.get());
    Fake(Method(mockNimBLEService, start));

    When(Method(mockNimBLECharacteristic, getSubscribedCount)).AlwaysReturn(0);
    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 1U>)));
    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const unsigned short)));
    Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::string)));
    Fake(Method(mockNimBLECharacteristic, notify));

    Fake(Method(mockNimBLEAdvertising, start));
    Fake(Method(mockNimBLEAdvertising, setAppearance));
    Fake(Method(mockNimBLEAdvertising, addServiceUUID));

    When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);
    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(true);
    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(true);
    When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(ArduinoLogLevel::LogLevelSilent);

    When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(true);

    // Test specific mocks
    When(
        OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
            .Using(CommonBleFlags::settingsControlPointUuid, Any()))
        .AlwaysReturn(&mockControlPointCharacteristic.get());

    Fake(Method(mockControlPointCharacteristic, notify));
    Fake(Method(mockControlPointCharacteristic, indicate));
    Fake(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>)));

    Fake(Method(mockEEPROMService, setLogLevel));
    Fake(Method(mockEEPROMService, setBleServiceFlag));
    Fake(Method(mockEEPROMService, setLogToSdCard));
    Fake(Method(mockEEPROMService, setLogToBluetooth));

    BluetoothService bluetoothService(mockEEPROMService.get(), mockSdCardService.get());
    bluetoothService.setup();
    mockNimBLECharacteristic.ClearInvocationHistory();
    NimBLECharacteristicCallbacks *controlPointCallback = std::move(mockControlPointCharacteristic.get().callbacks);

    SECTION("indicate OperationFailed response when request is empty")
    {
        std::array<unsigned char, 3U> operationFailedResponse = {
            static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
            static_cast<unsigned char>(0),
            static_cast<unsigned char>(ResponseOpCodes::OperationFailed)};

        When(Method(mockControlPointCharacteristic, getValue)).Return({});

        controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

        Verify(Method(mockControlPointCharacteristic, getValue)).Once();
        Verify(Method(mockControlPointCharacteristic, indicate)).Once();
        Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                   .Using(Eq(operationFailedResponse)))
            .Once();
    }

    SECTION("indicate UnsupportedOpCode when unkown OpCode is sent")
    {
        const auto unsupportedOpCode = 60;

        std::array<unsigned char, 3U> unsupportedOperationResponse = {
            static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
            static_cast<unsigned char>(unsupportedOpCode),
            static_cast<unsigned char>(ResponseOpCodes::UnsupportedOpCode)};

        When(Method(mockControlPointCharacteristic, getValue)).Return({unsupportedOpCode});

        controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

        Verify(Method(mockControlPointCharacteristic, getValue)).Once();
        Verify(Method(mockControlPointCharacteristic, indicate)).Once();
        Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                   .Using(Eq(unsupportedOperationResponse)))
            .Once();
    }

    SECTION("handle SetLogLevel request")
    {
        SECTION("and when log level settings is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::SetLogLevel),
                static_cast<unsigned char>(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetLogLevel), 10});

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when log level settings is missing return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::SetLogLevel),
                static_cast<unsigned char>(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetLogLevel)});

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when log level settings is is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::SetLogLevel),
                static_cast<unsigned char>(ResponseOpCodes::Successful)};
            const auto expectedLogLevel = ArduinoLogLevel::LogLevelInfo;

            const auto logToBluetooth = true;
            const auto logToSdCard = true;
            const auto logLevel = expectedLogLevel;
            const auto logFileOpen = false;
            const unsigned char expectedSettings =
                ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
                ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned>(logToSdCard) + 1 : 0) << 2U) |
                (static_cast<unsigned char>(logLevel) << 4U);

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetLogLevel), static_cast<unsigned char>(expectedLogLevel)});
            When(Method(mockNimBLECharacteristic, getSubscribedCount)).AlwaysReturn(1);
            When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetooth);
            When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCard);
            When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevel);
            When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(logFileOpen);

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            SECTION("indicate Success response")
            {
                Verify(Method(mockControlPointCharacteristic, getValue)).Once();
                Verify(Method(mockControlPointCharacteristic, indicate)).Once();
                Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                           .Using(Eq(successResponse)))
                    .Once();
            }

            SECTION("save new log level")
            {
                Verify(Method(mockEEPROMService, setLogLevel).Using(Eq(expectedLogLevel))).Once();
            }

            SECTION("notify new settings")
            {
                Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                           .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})))
                    .Once();
                Verify(Method(mockNimBLECharacteristic, notify)).Once();
            }
        }
    }

    SECTION("handle ChangeBleService request")
    {
        SECTION("and when BLE service settings is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::ChangeBleService),
                static_cast<unsigned char>(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::ChangeBleService), 10});

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when BLE service settings is missing return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::ChangeBleService),
                static_cast<unsigned char>(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::ChangeBleService)});

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when BLE service setting is is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::ChangeBleService),
                static_cast<unsigned char>(ResponseOpCodes::Successful)};
            const auto expectedBleService = BleServiceFlag::CscService;

            const auto logToBluetooth = true;
            const auto logToSdCard = true;
            const auto logLevel = ArduinoLogLevel::LogLevelSilent;
            const auto logFileOpen = false;
            const unsigned char expectedSettings =
                ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
                ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned>(logToSdCard) + 1 : 0) << 2U) |
                (static_cast<unsigned char>(logLevel) << 4U);

            When(Method(mockNimBLECharacteristic, getSubscribedCount)).AlwaysReturn(1);
            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::ChangeBleService), static_cast<unsigned char>(expectedBleService)});
            When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetooth);
            When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCard);
            When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevel);
            Fake(Method(mockEEPROMService, setLogLevel));
            When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(logFileOpen);
            Fake(Method(mockArduino, esp_restart));
            Fake(Method(mockArduino, delay));

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            SECTION("indicate Success response")
            {
                Verify(Method(mockControlPointCharacteristic, getValue)).Once();
                Verify(Method(mockControlPointCharacteristic, indicate)).Once();
                Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                           .Using(Eq(successResponse)))
                    .Once();
            }

            SECTION("save new BLE service flag")
            {
                Verify(Method(mockEEPROMService, setBleServiceFlag).Using(Eq(expectedBleService))).Once();
            }

            SECTION("notify new settings")
            {
                Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                           .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})))
                    .Once();
                Verify(Method(mockNimBLECharacteristic, notify)).Once();
            }

            SECTION("restart device")
            {
                Verify(Method(mockArduino, esp_restart)).Once();
            }
        }
    }

    SECTION("handle SetSdCardLogging request")
    {
        SECTION("and when Delta Time logging setting is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::SetSdCardLogging),
                static_cast<unsigned char>(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetSdCardLogging), 10});

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when Delta Time logging setting is missing return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::SetSdCardLogging),
                static_cast<unsigned char>(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetSdCardLogging)});

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when SD Card logging settings is is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::SetSdCardLogging),
                static_cast<unsigned char>(ResponseOpCodes::Successful)};
            const auto expectedSdCardLogging = true;

            const auto logToBluetooth = true;
            const auto logToSdCard = expectedSdCardLogging;
            const auto logLevel = ArduinoLogLevel::LogLevelSilent;
            const auto logFileOpen = true;
            const unsigned char expectedSettings =
                ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
                ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned>(logToSdCard) + 1 : 0) << 2U) |
                (static_cast<unsigned char>(logLevel) << 4U);

            When(Method(mockNimBLECharacteristic, getSubscribedCount)).AlwaysReturn(1);
            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetSdCardLogging), static_cast<unsigned char>(expectedSdCardLogging)});
            When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetooth);
            When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCard);
            When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevel);
            Fake(Method(mockEEPROMService, setLogLevel));
            When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(logFileOpen);

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            SECTION("indicate Success response")
            {
                Verify(Method(mockControlPointCharacteristic, getValue)).Once();
                Verify(Method(mockControlPointCharacteristic, indicate)).Once();
                Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                           .Using(Eq(successResponse)))
                    .Once();
            }

            SECTION("save new SD Card logging setting")
            {
                Verify(Method(mockEEPROMService, setLogToSdCard).Using(expectedSdCardLogging)).Once();
            }

            SECTION("notify new settings")
            {
                Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                           .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})))
                    .Once();
                Verify(Method(mockNimBLECharacteristic, notify)).Once();
            }
        }
    }

    SECTION("handle SetDeltaTimeLogging request")
    {
        SECTION("and when Delta Time logging setting is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::SetDeltaTimeLogging),
                static_cast<unsigned char>(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetDeltaTimeLogging), 10});

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when Delta Time logging setting is missing return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::SetDeltaTimeLogging),
                static_cast<unsigned char>(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetDeltaTimeLogging), 10});

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when Delta Time logging setting is is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
                static_cast<unsigned char>(SettingsOpCodes::SetDeltaTimeLogging),
                static_cast<unsigned char>(ResponseOpCodes::Successful)};
            const auto expectedDeltaTimeLogging = true;

            const auto logToBluetooth = expectedDeltaTimeLogging;
            const auto logToSdCard = true;
            const auto logLevel = ArduinoLogLevel::LogLevelSilent;
            const auto logFileOpen = false;
            const unsigned char expectedSettings =
                ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
                ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned>(logToSdCard) + 1 : 0) << 2U) |
                (static_cast<unsigned char>(logLevel) << 4U);

            When(Method(mockNimBLECharacteristic, getSubscribedCount)).AlwaysReturn(1);
            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetDeltaTimeLogging), static_cast<unsigned char>(expectedDeltaTimeLogging)});
            When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetooth);
            When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCard);
            When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevel);
            Fake(Method(mockEEPROMService, setLogLevel));
            When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(logFileOpen);

            controlPointCallback->onWrite(&mockControlPointCharacteristic.get());

            SECTION("indicate Success response")
            {
                Verify(Method(mockControlPointCharacteristic, getValue)).Once();
                Verify(Method(mockControlPointCharacteristic, indicate)).Once();
                Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                           .Using(Eq(successResponse)))
                    .Once();
            }

            SECTION("save new Delta Time logging setting")
            {
                Verify(Method(mockEEPROMService, setLogToBluetooth).Using(expectedDeltaTimeLogging)).Once();
            }

            SECTION("notify new settings")
            {
                Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                           .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})))
                    .Once();
                Verify(Method(mockNimBLECharacteristic, notify)).Once();
            }
        }
    }
}