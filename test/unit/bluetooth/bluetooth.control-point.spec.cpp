// NOLINTBEGIN(readability-magic-numbers, readability-function-cognitive-complexity)
#include <array>
#include <bit>
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
    Fake(Method(mockEEPROMService, setMachineSettings));
    Fake(Method(mockEEPROMService, setSensorSignalSettings));
    Fake(Method(mockEEPROMService, setDragFactorSettings));
    Fake(Method(mockEEPROMService, setStrokePhaseDetectionSettings));

    Fake(Method(mockSettingsBleService, broadcastSettings));
    Fake(Method(mockSettingsBleService, broadcastStrokeDetectionSettings));

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

        SECTION("and when log level settings is valid")
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

        SECTION("and when BLE service setting is valid")
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

        SECTION("and when SD Card logging settings is valid")
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

        SECTION("and when Delta Time logging setting is valid")
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

    SECTION("handle RestartDevice request")
    {
        std::array<unsigned char, 3U> successResponse = {
            std::to_underlying(SettingsOpCodes::ResponseCode),
            std::to_underlying(SettingsOpCodes::RestartDevice),
            std::to_underlying(ResponseOpCodes::Successful)};

        When(Method(mockControlPointCharacteristic, getValue)).Return({std::to_underlying(SettingsOpCodes::RestartDevice)});
        Fake(Method(mockGlobals, restartWithDelay));

        controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

        SECTION("and indicate Success response")
        {
            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(successResponse)))
                .Once();
        }

        SECTION("restart device")
        {
            Verify(Method(mockGlobals, restartWithDelay)).Once();
        }
    }

    SECTION("handle SetMachineSettings request")
    {
#if ENABLE_RUNTIME_SETTINGS
        SECTION("and when settings payload size is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetMachineSettings),
                std::to_underlying(ResponseOpCodes::InvalidParameter),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({
                std::to_underlying(SettingsOpCodes::SetMachineSettings),
                10,
                10,
            });

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when settings values are invalid return OperationFailed response")
        {
            std::array<unsigned char, 3U> operationsFailedResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetMachineSettings),
                std::to_underlying(ResponseOpCodes::OperationFailed),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({
                std::to_underlying(SettingsOpCodes::SetMachineSettings),
                0,
                0,
                128,
                191,
                98,
                12,
                32,
                10,
            });

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(operationsFailedResponse)))
                .Once();
        }

        SECTION("and when MachineSettings is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetMachineSettings),
                std::to_underlying(ResponseOpCodes::Successful),
            };

            const auto expectedFlywheelInertia = 1.0F;
            const auto expectedMagicNumber = 1.0F;
            const auto expectedImpulsesPerRevolution = 1U;
            const auto expectedSprocketRadius = 0.01F;

            const auto flywheelInertia = std::bit_cast<unsigned int>(expectedFlywheelInertia);
            const auto magicNumber = roundf(expectedMagicNumber * ISettingsBleService::magicNumberScale);
            const auto mToCm = 100.0F;
            const auto sprocketRadius = static_cast<unsigned short>(roundf(expectedSprocketRadius * ISettingsBleService::sprocketRadiusScale * mToCm));

            const NimBLEAttValue payload = {
                std::to_underlying(SettingsOpCodes::SetMachineSettings),
                static_cast<unsigned char>(flywheelInertia),
                static_cast<unsigned char>(flywheelInertia >> 8),
                static_cast<unsigned char>(flywheelInertia >> 16),
                static_cast<unsigned char>(flywheelInertia >> 24),
                static_cast<unsigned char>(magicNumber),
                expectedImpulsesPerRevolution,
                static_cast<unsigned char>(sprocketRadius),
                static_cast<unsigned char>(sprocketRadius >> 8),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return(payload);

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            SECTION("save new machine settings to EEPROM")
            {
                Verify(Method(mockEEPROMService, setMachineSettings).Matching([&expectedFlywheelInertia, &expectedMagicNumber, &expectedImpulsesPerRevolution, &expectedSprocketRadius](const RowerProfile::MachineSettings newSettings)
                                                                              {
                        REQUIRE(newSettings.flywheelInertia == expectedFlywheelInertia);
                        REQUIRE(newSettings.concept2MagicNumber == expectedMagicNumber);
                        REQUIRE(newSettings.impulsesPerRevolution == expectedImpulsesPerRevolution);
                        REQUIRE(newSettings.sprocketRadius == expectedSprocketRadius);

                        return true; }))
                    .Once();
            }

            SECTION("notify new settings")
            {
                Verify(Method(mockSettingsBleService, broadcastSettings)).Once();
            }

            SECTION("indicate Success response")
            {
                Verify(Method(mockControlPointCharacteristic, getValue)).Once();
                Verify(Method(mockControlPointCharacteristic, indicate)).Once();
                Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                           .Using(Eq(successResponse)))
                    .Once();
            }
        }
#else
        SECTION("and when runtime settings are disabled return UnsupportedOpCode response")
        {
            std::array<unsigned char, 3U> unsupportedParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetMachineSettings),
                std::to_underlying(ResponseOpCodes::UnsupportedOpCode),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({
                std::to_underlying(SettingsOpCodes::SetMachineSettings),
            });

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(unsupportedParameterResponse)))
                .Once();
        }
#endif
    }

    SECTION("handle SetSensorSignalSettings request")
    {
#if ENABLE_RUNTIME_SETTINGS
        SECTION("and when settings payload size is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetSensorSignalSettings),
                std::to_underlying(ResponseOpCodes::InvalidParameter),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({
                std::to_underlying(SettingsOpCodes::SetSensorSignalSettings),
                10,
            });

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when settings values are invalid return OperationFailed response")
        {
            std::array<unsigned char, 3U> operationsFailedResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetSensorSignalSettings),
                std::to_underlying(ResponseOpCodes::OperationFailed),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({
                std::to_underlying(SettingsOpCodes::SetSensorSignalSettings),
                0,
                3,
            });

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(operationsFailedResponse)))
                .Once();
        }

        SECTION("and when SensorSignalSettings is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetSensorSignalSettings),
                std::to_underlying(ResponseOpCodes::Successful),
            };

            const unsigned char expectedRotationDebounce = 1U;
            const unsigned char expectedRowingStoppedThresholdPeriod = 4U;

            const NimBLEAttValue payload = {
                std::to_underlying(SettingsOpCodes::SetSensorSignalSettings),
                expectedRotationDebounce,
                expectedRowingStoppedThresholdPeriod,
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return(payload);

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            SECTION("save new sensor signal settings to EEPROM")
            {
                Verify(Method(mockEEPROMService, setSensorSignalSettings).Matching([](const RowerProfile::SensorSignalSettings newSettings)
                                                                                   {
                        REQUIRE(newSettings.rotationDebounceTimeMin == static_cast<unsigned short>(expectedRotationDebounce * ISettingsBleService::debounceTimeScale));
                        REQUIRE(newSettings.rowingStoppedThresholdPeriod == expectedRowingStoppedThresholdPeriod * ISettingsBleService::rowingStoppedThresholdScale);

                        return true; }))
                    .Once();
            }

            SECTION("notify new settings")
            {
                Verify(Method(mockSettingsBleService, broadcastSettings)).Once();
            }

            SECTION("indicate Success response")
            {
                Verify(Method(mockControlPointCharacteristic, getValue)).Once();
                Verify(Method(mockControlPointCharacteristic, indicate)).Once();
                Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                           .Using(Eq(successResponse)))
                    .Once();
            }
        }
#else
        SECTION("and when runtime settings are disabled return UnsupportedOpCode response")
        {
            std::array<unsigned char, 3U> unsupportedParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetMachineSettings),
                std::to_underlying(ResponseOpCodes::UnsupportedOpCode),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({
                std::to_underlying(SettingsOpCodes::SetMachineSettings),
            });

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(unsupportedParameterResponse)))
                .Once();
        }
#endif
    }

    SECTION("handle SetDragFactorSettings request")
    {
#if ENABLE_RUNTIME_SETTINGS
        When(Method(mockEEPROMService, getSensorSignalSettings)).AlwaysReturn(RowerProfile::SensorSignalSettings{});

        SECTION("and when settings payload size is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetDragFactorSettings),
                std::to_underlying(ResponseOpCodes::InvalidParameter),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({
                std::to_underlying(SettingsOpCodes::SetDragFactorSettings),
                10,
                10,
            });

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when settings values are invalid return OperationFailed response")
        {
            std::array<unsigned char, 3U> operationsFailedResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetDragFactorSettings),
                std::to_underlying(ResponseOpCodes::OperationFailed),
            };

            const auto invalidDragFactorRecoveryPeriod = 1'000 / (RowerProfile::Defaults::rotationDebounceTimeMin / 1'000) + 1;

            When(Method(mockControlPointCharacteristic, getValue)).Return({
                std::to_underlying(SettingsOpCodes::SetDragFactorSettings),
                0,
                invalidDragFactorRecoveryPeriod,
                191,
                98,
                12,
                32,
                10,
            });

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(operationsFailedResponse)))
                .Once();
        }

        SECTION("and when DragFactorSettings is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetDragFactorSettings),
                std::to_underlying(ResponseOpCodes::Successful),
            };

            constexpr auto expectedGoodnessOfFitThreshold = 0.968627453F;
            const auto dragFactorLowerThreshold = static_cast<unsigned short>(roundf(RowerProfile::Defaults::lowerDragFactorThreshold * ISettingsBleService::dragFactorThresholdScale));
            const auto dragFactorUpperThreshold = static_cast<unsigned short>(roundf(RowerProfile::Defaults::upperDragFactorThreshold * ISettingsBleService::dragFactorThresholdScale));

            const NimBLEAttValue payload = {
                std::to_underlying(SettingsOpCodes::SetDragFactorSettings),
                static_cast<unsigned char>(roundf(expectedGoodnessOfFitThreshold * ISettingsBleService::goodnessOfFitThresholdScale)),
                static_cast<unsigned char>(RowerProfile::Defaults::maxDragFactorRecoveryPeriod / ISettingsBleService::dragFactorRecoveryPeriodScale),
                static_cast<unsigned char>(dragFactorLowerThreshold),
                static_cast<unsigned char>(dragFactorLowerThreshold >> 8),
                static_cast<unsigned char>(dragFactorUpperThreshold),
                static_cast<unsigned char>(dragFactorUpperThreshold >> 8),
                RowerProfile::Defaults::dragCoefficientsArrayLength,
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return(payload);

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            SECTION("save new machine settings to EEPROM")
            {
                Verify(Method(mockEEPROMService, setDragFactorSettings).Matching([](const RowerProfile::DragFactorSettings newSettings)
                                                                                 {
                        REQUIRE(newSettings.goodnessOfFitThreshold == expectedGoodnessOfFitThreshold);
                        REQUIRE(newSettings.maxDragFactorRecoveryPeriod  == RowerProfile::Defaults::maxDragFactorRecoveryPeriod);
                        REQUIRE(newSettings.lowerDragFactorThreshold == RowerProfile::Defaults::lowerDragFactorThreshold);
                        REQUIRE(newSettings.upperDragFactorThreshold  == RowerProfile::Defaults::upperDragFactorThreshold);
                        REQUIRE(newSettings.dragCoefficientsArrayLength  == RowerProfile::Defaults::dragCoefficientsArrayLength);

                        return true; }))
                    .Once();
            }

            SECTION("notify new settings")
            {
                Verify(Method(mockSettingsBleService, broadcastSettings)).Once();
            }

            SECTION("indicate Success response")
            {
                Verify(Method(mockControlPointCharacteristic, getValue)).Once();
                Verify(Method(mockControlPointCharacteristic, indicate)).Once();
                Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                           .Using(Eq(successResponse)))
                    .Once();
            }
        }
#else
        SECTION("and when runtime settings are disabled return UnsupportedOpCode response")
        {
            std::array<unsigned char, 3U> unsupportedParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetDragFactorSettings),
                std::to_underlying(ResponseOpCodes::UnsupportedOpCode),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({
                std::to_underlying(SettingsOpCodes::SetDragFactorSettings),
            });

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(unsupportedParameterResponse)))
                .Once();
        }
#endif
    }

    SECTION("handle SetStrokeDetectionSettings request")
    {
#if ENABLE_RUNTIME_SETTINGS
        SECTION("and when settings payload size is invalid return InvalidParameter response")
        {
            std::array<unsigned char, 3U> invalidParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetStrokeDetectionSettings),
                std::to_underlying(ResponseOpCodes::InvalidParameter),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({
                std::to_underlying(SettingsOpCodes::SetStrokeDetectionSettings),
                10,
                10,
            });

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(invalidParameterResponse)))
                .Once();
        }

        SECTION("and when settings values are invalid return OperationFailed response")
        {
            std::array<unsigned char, 3U> operationsFailedResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetStrokeDetectionSettings),
                std::to_underlying(ResponseOpCodes::OperationFailed),
            };

            const NimBLEAttValue invalidPayload = {
                std::to_underlying(SettingsOpCodes::SetStrokeDetectionSettings),
                0x03,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                100,
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return(invalidPayload);

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(operationsFailedResponse)))
                .Once();
        }

        SECTION("and when StrokeDetectionSettings is valid")
        {
            std::array<unsigned char, 3U> successResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetStrokeDetectionSettings),
                std::to_underlying(ResponseOpCodes::Successful),
            };

            const auto impulseAndDetection = (std::to_underlying(RowerProfile::Defaults::strokeDetectionType) & 0x03) | 
                                            ((RowerProfile::Defaults::impulseDataArrayLength & 0x1F) << 2U) |
                                            (static_cast<unsigned char>(std::is_same_v<Configurations::precision, double>) << 7U);
            const auto poweredTorque = static_cast<short>(roundf(RowerProfile::Defaults::minimumPoweredTorque * ISettingsBleService::poweredTorqueScale));
            const auto dragTorque = static_cast<short>(roundf(RowerProfile::Defaults::minimumDragTorque * ISettingsBleService::dragTorqueScale));
            const auto recoverySlopeMarginBits = std::bit_cast<unsigned int>(RowerProfile::Defaults::minimumRecoverySlopeMargin * ISettingsBleService::recoverySlopeMarginPayloadScale);
            const auto recoverySlope = static_cast<short>(roundf(RowerProfile::Defaults::minimumRecoverySlope * ISettingsBleService::recoverySlopeScale));
            const auto strokeTimes = (RowerProfile::Defaults::minimumRecoveryTime / ISettingsBleService::minimumStrokeTimesScale) | ((RowerProfile::Defaults::minimumDriveTime / ISettingsBleService::minimumStrokeTimesScale) << 12U);

            const NimBLEAttValue payload = {
                std::to_underlying(SettingsOpCodes::SetStrokeDetectionSettings),
                impulseAndDetection,
                static_cast<unsigned char>(poweredTorque),
                static_cast<unsigned char>(poweredTorque >> 8),
                static_cast<unsigned char>(dragTorque),
                static_cast<unsigned char>(dragTorque >> 8),
                static_cast<unsigned char>(recoverySlopeMarginBits),
                static_cast<unsigned char>(recoverySlopeMarginBits >> 8),
                static_cast<unsigned char>(recoverySlopeMarginBits >> 16),
                static_cast<unsigned char>(recoverySlopeMarginBits >> 24),
                static_cast<unsigned char>(recoverySlope),
                static_cast<unsigned char>(recoverySlope >> 8),
                static_cast<unsigned char>(strokeTimes),
                static_cast<unsigned char>(strokeTimes >> 8),
                static_cast<unsigned char>(strokeTimes >> 16),
                RowerProfile::Defaults::driveHandleForcesMaxCapacity,
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return(payload);

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            SECTION("save new stroke detection settings to EEPROM")
            {
                Verify(Method(mockEEPROMService, setStrokePhaseDetectionSettings).Matching([&](const RowerProfile::StrokePhaseDetectionSettings newSettings)
                                                                                           {
                        REQUIRE(newSettings.strokeDetectionType == RowerProfile::Defaults::strokeDetectionType);
                        REQUIRE(newSettings.impulseDataArrayLength == RowerProfile::Defaults::impulseDataArrayLength);
                        REQUIRE(newSettings.minimumPoweredTorque == RowerProfile::Defaults::minimumPoweredTorque);
                        REQUIRE(newSettings.minimumDragTorque == RowerProfile::Defaults::minimumDragTorque);
                        REQUIRE(newSettings.minimumRecoverySlopeMargin == RowerProfile::Defaults::minimumRecoverySlopeMargin);
                        REQUIRE(newSettings.minimumRecoverySlope == RowerProfile::Defaults::minimumRecoverySlope);
                        REQUIRE(newSettings.minimumRecoveryTime == RowerProfile::Defaults::minimumRecoveryTime);
                        REQUIRE(newSettings.minimumDriveTime == RowerProfile::Defaults::minimumDriveTime);
                        REQUIRE(newSettings.driveHandleForcesMaxCapacity == RowerProfile::Defaults::driveHandleForcesMaxCapacity);

                        return true; }))
                    .Once();
            }

            SECTION("notify new stroke detection settings")
            {
                Verify(Method(mockSettingsBleService, broadcastStrokeDetectionSettings)).Once();
            }

            SECTION("indicate Success response")
            {
                Verify(Method(mockControlPointCharacteristic, getValue)).Once();
                Verify(Method(mockControlPointCharacteristic, indicate)).Once();
                Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                           .Using(Eq(successResponse)))
                    .Once();
            }
        }
#else
        SECTION("and when runtime settings are disabled return UnsupportedOpCode response")
        {
            std::array<unsigned char, 3U> unsupportedParameterResponse = {
                std::to_underlying(SettingsOpCodes::ResponseCode),
                std::to_underlying(SettingsOpCodes::SetStrokeDetectionSettings),
                std::to_underlying(ResponseOpCodes::UnsupportedOpCode),
            };

            When(Method(mockControlPointCharacteristic, getValue)).Return({
                std::to_underlying(SettingsOpCodes::SetStrokeDetectionSettings),
            });

            controlPointCallback.onWrite(&mockControlPointCharacteristic.get(), mockConnectionInfo.get());

            Verify(Method(mockControlPointCharacteristic, getValue)).Once();
            Verify(Method(mockControlPointCharacteristic, indicate)).Once();
            Verify(OverloadedMethod(mockControlPointCharacteristic, setValue, void(const std::array<unsigned char, 3U>))
                       .Using(Eq(unsupportedParameterResponse)))
                .Once();
        }
#endif
    }
}
// NOLINTEND(readability-magic-numbers, readability-function-cognitive-complexity)