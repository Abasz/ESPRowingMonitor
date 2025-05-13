#include <array>
#include <utility>

#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "../include/Arduino.h"
#include "../include/NimBLEDevice.h"

#include "../include/globals.h"

#include "../../../src/peripherals/bluetooth//ble-services/settings.service.interface.h"
#include "../../../src/peripherals/bluetooth/callbacks/control-point.callbacks.h"
#include "../../../src/peripherals/sd-card/sd-card.service.interface.h"
#include "../../../src/utils/EEPROM/EEPROM.service.interface.h"
#include "../../../src/utils/enums.h"
#include "../../../src/utils/ota-updater/ota-updater.service.interface.h"

using namespace fakeit;

TEST_CASE("ControlPointCallbacks onWrite method should", "[callbacks]")
{
    mockArduino.Reset();
    mockGlobals.Reset();

    Mock<IEEPROMService> mockEEPROMService;
    Mock<ISettingsBleService> mockSettingsBleService;
    Mock<NimBLECharacteristic> mockControlPointCharacteristic;
    Mock<NimBLEConnInfo> mockConnectionInfo;

    Fake(Method(mockControlPointCharacteristic, indicate));
    Fake(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>)));

    Fake(Method(mockEEPROMService, setLogLevel));
    Fake(Method(mockEEPROMService, setBleServiceFlag));
    Fake(Method(mockEEPROMService, setLogToSdCard));
    Fake(Method(mockEEPROMService, setLogToBluetooth));

    Fake(Method(mockSettingsBleService, broadcastSettings));

    ControlPointCallbacks controlPointCallback(mockSettingsBleService.get(), mockEEPROMService.get());

    SECTION("indicate OperationFailed response when request is empty")
    {
        std::array<unsigned char, 3U> operationFailedResponse = {
            std::to_underlying(SettingsOpCodes::ResponseCode),
            static_cast<unsigned char>(0),
            std::to_underlying(ResponseOpCodes::OperationFailed)};

        When(Method(mockControlPointCharacteristic, getValue)).Return({});

        controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

        Verify(Method(mockControlPointCharacteristic, getValue)).Once();
        Verify(Method(mockControlPointCharacteristic, indicate)).Once();
        Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                   .Using(Eq(operationFailedResponse)))
            .Once();
    }

    SECTION("when unkown OpCode is sent")
    {
        SECTION("and BleServiceFlag is not FtmsService indicate UnsupportedOpCode")
        {
            const auto unsupportedOpCode = 60;

            std::array<unsigned char, 3U> unsupportedOperationFtmsResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCodeFtms),
                static_cast<unsigned char>(unsupportedOpCode),
                std::to_underlying(ResponseOpCodes::ControlNotPermitted),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({unsupportedOpCode});
            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn({BleServiceFlag::FtmsService});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(unsupportedOperationFtmsResponse)))
                .Once();
        }

        SECTION("and BleServiceFlag is FtmsService indicate ControlNotPermitted")
        {
            const auto unsupportedOpCode = 60;

            std::array<unsigned char, 3U> unsupportedOperationFtmsResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCodeFtms),
                static_cast<unsigned char>(unsupportedOpCode),
                std::to_underlying(ResponseOpCodes::ControlNotPermitted),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({unsupportedOpCode});
            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn({BleServiceFlag::FtmsService});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(unsupportedOperationFtmsResponse)))
                .Once();
        }
    }

    SECTION("handle SetLogLevel request")
    {
        SECTION("and when log level settings is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetLogLevel),
                std::to_underlying(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::SetLogLevel), 10});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when log level settings is missing return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetLogLevel),
                std::to_underlying(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::SetLogLevel)});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when log level settings is is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetLogLevel),
                std::to_underlying(ResponseOpCodes::Successful)};
            const auto expectedLogLevel = ArduinoLogLevel::LogLevelInfo;

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::SetLogLevel), std::to_underlying(expectedLogLevel)});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

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
                Verify(Method(mockSettingsBleService, broadcastSettings)).Once();
            }
        }
    }

    SECTION("handle ChangeBleService request")
    {
        SECTION("and when BLE service settings is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::ChangeBleService),
                std::to_underlying(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::ChangeBleService), 3});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when BLE service settings is missing return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::ChangeBleService),
                std::to_underlying(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::ChangeBleService)});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when BLE service setting is is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::ChangeBleService),
                std::to_underlying(ResponseOpCodes::Successful)};
            const auto expectedBleService = BleServiceFlag::FtmsService;

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::ChangeBleService), std::to_underlying(expectedBleService)});
            Fake(Method(mockEEPROMService, setLogLevel));
            Fake(Method(mockGlobals, restartWithDelay));
            Fake(Method(mockArduino, delay));

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

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
                Verify(Method(mockSettingsBleService, broadcastSettings)).Once();
            }

            SECTION("restart device")
            {
                Verify(Method(mockGlobals, restartWithDelay)).Once();
            }
        }
    }

    SECTION("handle SetSdCardLogging request")
    {
        SECTION("and when Delta Time logging setting is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetSdCardLogging),
                std::to_underlying(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::SetSdCardLogging), 10});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when Delta Time logging setting is missing return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetSdCardLogging),
                std::to_underlying(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::SetSdCardLogging)});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when SD Card logging settings is is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetSdCardLogging),
                std::to_underlying(ResponseOpCodes::Successful)};
            const auto expectedSdCardLogging = true;

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::SetSdCardLogging), static_cast<unsigned char>(expectedSdCardLogging)});
            Fake(Method(mockEEPROMService, setLogLevel));

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

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
                Verify(Method(mockSettingsBleService, broadcastSettings)).Once();
            }
        }
    }

    SECTION("handle SetDeltaTimeLogging request")
    {
        SECTION("and when Delta Time logging setting is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetDeltaTimeLogging),
                std::to_underlying(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::SetDeltaTimeLogging), 10});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when Delta Time logging setting is missing return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetDeltaTimeLogging),
                std::to_underlying(ResponseOpCodes::InvalidParameter)};

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::SetDeltaTimeLogging), 10});

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when Delta Time logging setting is is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetDeltaTimeLogging),
                std::to_underlying(ResponseOpCodes::Successful)};
            const auto expectedDeltaTimeLogging = true;

            When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::SetDeltaTimeLogging), static_cast<unsigned char>(expectedDeltaTimeLogging)});
            Fake(Method(mockEEPROMService, setLogLevel));

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

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
                Verify(Method(mockSettingsBleService, broadcastSettings)).Once();
            }
        }
    }
}