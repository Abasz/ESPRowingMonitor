#include <array>
#include <string>

#include "../include/catch_amalgamated.hpp"
#include "../include/fakeit.hpp"

#include "../include/Arduino.h"
#include "../include/NimBLEDevice.h"

#include "../../../src/peripherals/bluetooth/bluetooth.controller.h"
#include "../../../src/peripherals/bluetooth/callbacks/control-point.callbacks.h"
#include "../../../src/peripherals/sd-card/sd-card.service.interface.h"
#include "../../../src/utils/EEPROM/EEPROM.service.interface.h"
#include "../../../src/utils/configuration.h"
#include "../../../src/utils/enums.h"
#include "../../../src/utils/ota-updater/ota-updater.service.interface.h"

using namespace fakeit;

TEST_CASE("ControlPointCallbacks onWrite method should", "[callbacks]")
{
    Mock<IEEPROMService> mockEEPROMService;
    Mock<IBluetoothController> mockBleController;
    Mock<NimBLECharacteristic> mockControlPointCharacteristic;

    mockArduino.Reset();
    mockNimBLEServer.Reset();
    mockNimBLEAdvertising.Reset();
    mockNimBLEService.Reset();
    mockNimBLECharacteristic.Reset();

    Fake(Method(mockNimBLEAdvertising, start));
    Fake(Method(mockNimBLEAdvertising, setAppearance));
    Fake(Method(mockNimBLEAdvertising, addServiceUUID));

    Fake(Method(mockControlPointCharacteristic, indicate));
    Fake(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>)));

    Fake(Method(mockEEPROMService, setLogLevel));
    Fake(Method(mockEEPROMService, setBleServiceFlag));
    Fake(Method(mockEEPROMService, setLogToSdCard));
    Fake(Method(mockEEPROMService, setLogToBluetooth));

    Fake(Method(mockBleController, notifySettings));

    ControlPointCallbacks controlPointCallback(mockBleController.get(), mockEEPROMService.get());

    SECTION("indicate OperationFailed response when request is empty")
    {
        std::array<unsigned char, 3U> operationFailedResponse = {
            static_cast<unsigned char>(SettingsOpCodes::ResponseCode),
            static_cast<unsigned char>(0),
            static_cast<unsigned char>(ResponseOpCodes::OperationFailed)};

        When(Method(mockControlPointCharacteristic, getValue)).Return({});

        controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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

        controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetLogLevel), static_cast<unsigned char>(expectedLogLevel)});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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
                Verify(Method(mockBleController, notifySettings)).Once();
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

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::ChangeBleService), static_cast<unsigned char>(expectedBleService)});
            Fake(Method(mockEEPROMService, setLogLevel));
            Fake(Method(mockArduino, esp_restart));
            Fake(Method(mockArduino, delay));

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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
                Verify(Method(mockBleController, notifySettings)).Once();
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

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetSdCardLogging), static_cast<unsigned char>(expectedSdCardLogging)});
            Fake(Method(mockEEPROMService, setLogLevel));

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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
                Verify(Method(mockBleController, notifySettings)).Once();
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

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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

            When(Method(mockControlPointCharacteristic, getValue)).Return({static_cast<unsigned char>(SettingsOpCodes::SetDeltaTimeLogging), static_cast<unsigned char>(expectedDeltaTimeLogging)});
            Fake(Method(mockEEPROMService, setLogLevel));

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get());

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
                Verify(Method(mockBleController, notifySettings)).Once();
            }
        }
    }
}