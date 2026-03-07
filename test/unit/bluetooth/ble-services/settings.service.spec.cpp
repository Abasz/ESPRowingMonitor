// NOLINTBEGIN(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while, modernize-type-traits)
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <string>
#include <type_traits>
#include <utility>

#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "../../include/ArduinoLog.h"
#include "../../include/NimBLEDevice.h"

#include "../../../../src/peripherals/bluetooth/ble-services/settings.service.h"
#include "../../../../src/peripherals/bluetooth/ble-services/settings.service.interface.h"
#include "../../../../src/peripherals/bluetooth/ble.enums.h"
#include "../../../../src/peripherals/sd-card/sd-card.service.interface.h"
#include "../../../../src/utils/EEPROM/EEPROM.service.interface.h"
#include "../../../../src/utils/configuration.h"
#include "../../../../src/utils/enums.h"
#include "../../../../src/utils/settings.model.h"

using namespace fakeit;

TEST_CASE("SettingsBleService", "[ble-service]")
{
    mockNimBLEServer.Reset();

    Mock<IEEPROMService> mockEEPROMService;
    Mock<ISdCardService> mockSdCardService;
    Mock<NimBLECharacteristic> mockSettingsCharacteristic;
    Mock<NimBLECharacteristic> mockStrokeSettingsCharacteristic;
    Mock<NimBLEService> mockSettingsService;

    const auto logToBluetooth = true;
    const auto logToSdCard = true;
    const auto logLevel = ArduinoLogLevel::LogLevelVerbose;
    const auto logFileOpen = true;

    const unsigned char settingsTestData =
        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCard) + 1 : 0) << 2U) |
        (static_cast<unsigned char>(logLevel) << 4U) |
        (static_cast<unsigned char>(Configurations::isRuntimeSettingsEnabled) << 7U);

    const auto flywheelInertia = std::bit_cast<unsigned int>(RowerProfile::Defaults::flywheelInertia);
    constexpr auto mToCm = 100.0F;
    const auto sprocketRadius = static_cast<unsigned short>(std::roundf(RowerProfile::Defaults::sprocketRadius * ISettingsBleService::sprocketRadiusScale * mToCm));

    const auto rotationDebounceTimeMin = static_cast<unsigned char>(RowerProfile::Defaults::rotationDebounceTimeMin / ISettingsBleService::debounceTimeScale);
    const auto rowingStoppedThresholdPeriod = static_cast<unsigned char>(RowerProfile::Defaults::rowingStoppedThresholdPeriod / ISettingsBleService::rowingStoppedThresholdScale);

    const auto expectedGoodnessOfFitThreshold = 0.968627453F;
    const auto dragFactorLowerThreshold = static_cast<unsigned short>(roundf(RowerProfile::Defaults::lowerDragFactorThreshold * ISettingsBleService::dragFactorThresholdScale));
    const auto dragFactorUpperThreshold = static_cast<unsigned short>(roundf(RowerProfile::Defaults::upperDragFactorThreshold * ISettingsBleService::dragFactorThresholdScale));

    const std::array<unsigned char, ISettingsBleService::settingsPayloadSize> expectedInitialSettings = {
        settingsTestData,
        static_cast<unsigned char>(flywheelInertia),
        static_cast<unsigned char>(flywheelInertia >> 8),
        static_cast<unsigned char>(flywheelInertia >> 16),
        static_cast<unsigned char>(flywheelInertia >> 24),
        static_cast<unsigned char>(roundf(RowerProfile::Defaults::concept2MagicNumber * ISettingsBleService::magicNumberScale)),
        RowerProfile::Defaults::impulsesPerRevolution,
        static_cast<unsigned char>(sprocketRadius),
        static_cast<unsigned char>(sprocketRadius >> 8),
        rotationDebounceTimeMin,
        rowingStoppedThresholdPeriod,
        static_cast<unsigned char>(roundf(expectedGoodnessOfFitThreshold * ISettingsBleService::goodnessOfFitThresholdScale)),
        static_cast<unsigned char>(RowerProfile::Defaults::maxDragFactorRecoveryPeriod / ISettingsBleService::dragFactorRecoveryPeriodScale),
        static_cast<unsigned char>(dragFactorLowerThreshold),
        static_cast<unsigned char>(dragFactorLowerThreshold >> 8),
        static_cast<unsigned char>(dragFactorUpperThreshold),
        static_cast<unsigned char>(dragFactorUpperThreshold >> 8),
        RowerProfile::Defaults::dragCoefficientsArrayLength,
    };

    const auto strokeDetectionAndImpulseEncoded = (std::to_underlying(RowerProfile::Defaults::strokeDetectionType) & 0x03) |
                                                  ((RowerProfile::Defaults::impulseDataArrayLength & 0x1F) << 2U) |
                                                  (static_cast<unsigned char>(std::is_same_v<Configurations::precision, double>) << 7U);
    const auto minimumPoweredTorque = static_cast<short>(roundf(RowerProfile::Defaults::minimumPoweredTorque * ISettingsBleService::poweredTorqueScale));
    const auto minimumDragTorque = static_cast<short>(roundf(RowerProfile::Defaults::minimumDragTorque * ISettingsBleService::dragTorqueScale));
    const auto minimumRecoverySlope = static_cast<short>(roundf(RowerProfile::Defaults::minimumRecoverySlope * ISettingsBleService::recoverySlopeScale));
    const auto strokeTimesEncoded = (RowerProfile::Defaults::minimumRecoveryTime / ISettingsBleService::minimumStrokeTimesScale) | ((RowerProfile::Defaults::minimumDriveTime / ISettingsBleService::minimumStrokeTimesScale) << 12);

    const std::array<unsigned char, ISettingsBleService::strokeSettingsPayloadSize> expectedInitialStrokeSettings = {
        static_cast<unsigned char>(strokeDetectionAndImpulseEncoded),
        static_cast<unsigned char>(minimumPoweredTorque),
        static_cast<unsigned char>(minimumPoweredTorque >> 8),
        static_cast<unsigned char>(minimumDragTorque),
        static_cast<unsigned char>(minimumDragTorque >> 8),
        static_cast<unsigned char>(minimumRecoverySlope),
        static_cast<unsigned char>(minimumRecoverySlope >> 8),
        static_cast<unsigned char>(strokeTimesEncoded),
        static_cast<unsigned char>(strokeTimesEncoded >> 8),
        static_cast<unsigned char>(strokeTimesEncoded >> 16),
        RowerProfile::Defaults::driveHandleForcesMaxCapacity,
    };

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string))).AlwaysReturn(&mockSettingsService.get());

    When(OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int)).Using(CommonBleFlags::settingsUuid, Any())).AlwaysReturn(&mockSettingsCharacteristic.get());
    When(OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int)).Using(CommonBleFlags::strokeDetectionSettingsUuid, Any())).AlwaysReturn(&mockStrokeSettingsCharacteristic.get());
    When(OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int)).Using(CommonBleFlags::settingsControlPointUuid, Any())).AlwaysReturn(&mockSettingsCharacteristic.get());

    Fake(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::settingsPayloadSize>)));
    Fake(OverloadedMethod(mockStrokeSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::strokeSettingsPayloadSize>)));
    Fake(Method(mockSettingsCharacteristic, setCallbacks));

    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetooth);
    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCard);
    When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevel);
    When(Method(mockEEPROMService, getMachineSettings)).AlwaysReturn(RowerProfile::MachineSettings{});
    When(Method(mockEEPROMService, getSensorSignalSettings)).AlwaysReturn(RowerProfile::SensorSignalSettings{});
    When(Method(mockEEPROMService, getDragFactorSettings)).AlwaysReturn(RowerProfile::DragFactorSettings{
        .goodnessOfFitThreshold = expectedGoodnessOfFitThreshold,
    });
    When(Method(mockEEPROMService, getStrokePhaseDetectionSettings)).AlwaysReturn(RowerProfile::StrokePhaseDetectionSettings{});
    When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(logFileOpen);

    SettingsBleService settingsBleService(mockSdCardService.get(), mockEEPROMService.get());

    SECTION("setup method should")
    {
        const unsigned int expectedSettingsProperty = NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ;
        const unsigned int expectedControlPointProperty = WRITE_NR | NIMBLE_PROPERTY::INDICATE;

        SECTION("initialize settings BLE service with correct UUID")
        {
            settingsBleService.setup(&mockNimBLEServer.get());

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string)).Using(CommonBleFlags::settingsServiceUuid)).Once();
        }

        SECTION("start stroke detection settings BLE characteristic with correct UUID")
        {
            settingsBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::strokeDetectionSettingsUuid, expectedSettingsProperty))
                .Once();
        }

        SECTION("start settings BLE characteristic with correct UUID")
        {
            settingsBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::settingsUuid, expectedSettingsProperty))
                .Once();
        }

        SECTION("set initial stroke detection settings value")
        {
            settingsBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockStrokeSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::strokeSettingsPayloadSize>))
                    .Using(Eq(expectedInitialStrokeSettings)))
                .Once();
        }

        SECTION("set initial settings value")
        {
            settingsBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::settingsPayloadSize>))
                    .Using(Eq(std::array<unsigned char, ISettingsBleService::settingsPayloadSize>{expectedInitialSettings})))
                .Once();
        }

        SECTION("initialize control point with correct UUID and callbacks")
        {
            settingsBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::settingsControlPointUuid, expectedControlPointProperty))
                .Once();

            Verify(Method(mockSettingsCharacteristic, setCallbacks).Using(Ne(nullptr))).Once();
        }

        SECTION("should return the created settings NimBLEService")
        {

            auto *const service = settingsBleService.setup(&mockNimBLEServer.get());

            REQUIRE(service == &mockSettingsService.get());
        }
    }

    SECTION("broadcastSettings method should")
    {
        Fake(Method(mockSettingsCharacteristic, notify));

        settingsBleService.setup(&mockNimBLEServer.get());
        mockSettingsCharacteristic.ClearInvocationHistory();

        SECTION("get current settings state")
        {
            mockEEPROMService.ClearInvocationHistory();
            mockSdCardService.ClearInvocationHistory();

            settingsBleService.broadcastSettings();

            Verify(Method(mockEEPROMService, getLogToBluetooth)).Once();
            Verify(Method(mockEEPROMService, getLogToSdCard)).Once();
            Verify(Method(mockEEPROMService, getLogLevel)).Once();
            Verify(Method(mockEEPROMService, getMachineSettings)).Once();
            Verify(Method(mockEEPROMService, getSensorSignalSettings)).Once();
            Verify(Method(mockEEPROMService, getDragFactorSettings)).Once();
            Verify(Method(mockSdCardService, isLogFileOpen)).Once();

            SECTION("and split MachineSettings correctly into bytes")
            {
                float flywheelInertiaZero = 0.0F;
                float concept2MagicNumberZero = 0.0F;
                float sprocketRadiusZero = 0.0F;
                unsigned char impulsesPerRevolutionZero = 0;

                When(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::settingsPayloadSize>))).Do([&flywheelInertiaZero, &concept2MagicNumberZero, &sprocketRadiusZero, &impulsesPerRevolutionZero](const std::array<unsigned char, ISettingsBleService::settingsPayloadSize> &settings)
                                                                                                                                                                 {
                    std::memcpy(&flywheelInertiaZero, &settings[1], ISettingsBleService::flywheelInertiaPayloadSize);
                    concept2MagicNumberZero = static_cast<float>(settings[5]) / ISettingsBleService::magicNumberScale;
                    impulsesPerRevolutionZero = settings[6];
                    sprocketRadiusZero = static_cast<float>(settings[7] | settings[8] << 8) /ISettingsBleService::sprocketRadiusScale / mToCm; });

                settingsBleService.broadcastSettings();

                REQUIRE(flywheelInertiaZero == RowerProfile::Defaults::flywheelInertia);
                REQUIRE(concept2MagicNumberZero == RowerProfile::Defaults::concept2MagicNumber);
                REQUIRE(impulsesPerRevolutionZero == RowerProfile::Defaults::impulsesPerRevolution);
                REQUIRE(sprocketRadiusZero == RowerProfile::Defaults::sprocketRadius);
            }

            SECTION("and split SensorSignalSettings correctly into bytes")
            {
                unsigned short debounceTimeMin = 0;
                unsigned int rowingStoppedThresholdPeriodZero = 0;

                When(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::settingsPayloadSize>)))
                    .Do([&debounceTimeMin, &rowingStoppedThresholdPeriodZero](const std::array<unsigned char, ISettingsBleService::settingsPayloadSize> &settings)
                        {
                        debounceTimeMin = settings[9] * ISettingsBleService::debounceTimeScale;
                        rowingStoppedThresholdPeriodZero = settings[10] * ISettingsBleService::rowingStoppedThresholdScale; });

                settingsBleService.broadcastSettings();

                REQUIRE(debounceTimeMin == RowerProfile::Defaults::rotationDebounceTimeMin);
                REQUIRE(rowingStoppedThresholdPeriodZero == RowerProfile::Defaults::rowingStoppedThresholdPeriod);
            }

            SECTION("and split DragFactorSettings correctly into bytes")
            {
                float goodnessOfFitThreshold = 0.0F;
                unsigned int maxDragFactorRecoveryPeriod = 0;
                float lowerDragFactorThreshold = 0.0F;
                float upperDragFactorThreshold = 0.0F;
                unsigned char dragCoefficientsArrayLength = 0;

                When(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::settingsPayloadSize>)))
                    .Do([&goodnessOfFitThreshold,
                         &maxDragFactorRecoveryPeriod,
                         &lowerDragFactorThreshold,
                         &upperDragFactorThreshold,
                         &dragCoefficientsArrayLength](const std::array<unsigned char, ISettingsBleService::settingsPayloadSize> &settings)
                        {
                            goodnessOfFitThreshold = static_cast<float>(settings[11]) / ISettingsBleService::goodnessOfFitThresholdScale;
                            maxDragFactorRecoveryPeriod = settings[12] * ISettingsBleService::dragFactorRecoveryPeriodScale;
                            lowerDragFactorThreshold = static_cast<float>(settings[13] | settings[14] << 8) / ISettingsBleService::dragFactorThresholdScale;
                            upperDragFactorThreshold = static_cast<float>(settings[15] | settings[16] << 8) / ISettingsBleService::dragFactorThresholdScale;
                            dragCoefficientsArrayLength = settings[17]; });

                settingsBleService.broadcastSettings();

                REQUIRE(goodnessOfFitThreshold == expectedGoodnessOfFitThreshold);
                REQUIRE(maxDragFactorRecoveryPeriod == RowerProfile::Defaults::maxDragFactorRecoveryPeriod);
                REQUIRE(lowerDragFactorThreshold == RowerProfile::Defaults::lowerDragFactorThreshold);
                REQUIRE(upperDragFactorThreshold == RowerProfile::Defaults::upperDragFactorThreshold);
                REQUIRE(dragCoefficientsArrayLength == RowerProfile::Defaults::dragCoefficientsArrayLength);
            }

            SECTION("and calculate correct setting binary value")
            {
                mockSettingsCharacteristic.ClearInvocationHistory();

                SECTION("when logToBluetooth is disabled")
                {
                    const auto logToBluetoothTest = false;

                    std::array<unsigned char, ISettingsBleService::settingsPayloadSize> expectedSettings = expectedInitialSettings;
                    expectedSettings[0] =
                        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetoothTest) + 1 : 0) << 0U) |
                        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCard) + 1 : 0) << 2U) |
                        (static_cast<unsigned char>(logLevel) << 4U) |
                        (static_cast<unsigned char>(Configurations::isRuntimeSettingsEnabled) << 7);

                    When(Method(mockEEPROMService, getLogToBluetooth))
                        .AlwaysReturn(logToBluetoothTest);
                    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCard);
                    When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevel);
                    When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(logFileOpen);

                    settingsBleService.broadcastSettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::settingsPayloadSize>))
                               .Using(Eq(expectedSettings)));
                }

                SECTION("when logToBluetooth is disabled")
                {
                    const auto logToBluetoothTest = false;

                    std::array<unsigned char, ISettingsBleService::settingsPayloadSize> expectedSettings = expectedInitialSettings;
                    expectedSettings[0] =
                        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetoothTest) + 1 : 0) << 0U) |
                        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCard) + 1 : 0) << 2U) |
                        (static_cast<unsigned char>(logLevel) << 4U) |
                        (static_cast<unsigned char>(Configurations::isRuntimeSettingsEnabled) << 7);

                    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetoothTest);

                    settingsBleService.broadcastSettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::settingsPayloadSize>))
                               .Using(Eq(expectedSettings)));
                }

                SECTION("when logToSdCard is disabled")
                {
                    const auto logToSdCardTest = false;

                    std::array<unsigned char, ISettingsBleService::settingsPayloadSize> expectedSettings = expectedInitialSettings;
                    expectedSettings[0] =
                        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
                        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCardTest) + 1 : 0) << 2U) |
                        (static_cast<unsigned char>(logLevel) << 4U) |
                        (static_cast<unsigned char>(Configurations::isRuntimeSettingsEnabled) << 7);

                    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCardTest);

                    settingsBleService.broadcastSettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::settingsPayloadSize>))
                               .Using(Eq(expectedSettings)));
                }

                SECTION("when logToSdCard is disabled and logLevel is Verbose")
                {
                    const auto logLevelTest = ArduinoLogLevel::LogLevelVerbose;
                    const auto logToSdCardTest = false;

                    std::array<unsigned char, ISettingsBleService::settingsPayloadSize> expectedSettings = expectedInitialSettings;
                    expectedSettings[0] =
                        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
                        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCardTest) + 1 : 0) << 2U) |
                        (static_cast<unsigned char>(logLevelTest) << 4U) |
                        (static_cast<unsigned char>(Configurations::isRuntimeSettingsEnabled) << 7);

                    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCardTest);
                    When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevelTest);

                    settingsBleService.broadcastSettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::settingsPayloadSize>))
                               .Using(Eq(expectedSettings)));
                }

                SECTION("when isRuntimeSettingsEnabled is true")
                {
                    settingsBleService.broadcastSettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::settingsPayloadSize>))
                               .Using(Eq(expectedInitialSettings)));
                }
            }
        }

        SECTION("set new settings")
        {
            settingsBleService.broadcastSettings();

            Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::settingsPayloadSize>))
                       .Using(Eq(expectedInitialSettings)))
                .Once();
        }

        SECTION("notify")
        {
            settingsBleService.broadcastSettings();

            Verify(Method(mockSettingsCharacteristic, notify));
        }
    }

    SECTION("broadcastStrokeDetectionSettings method should")
    {
        Fake(Method(mockStrokeSettingsCharacteristic, notify));

        settingsBleService.setup(&mockNimBLEServer.get());
        mockStrokeSettingsCharacteristic.ClearInvocationHistory();

        SECTION("get current stroke detection settings state")
        {
            mockEEPROMService.ClearInvocationHistory();

            settingsBleService.broadcastStrokeDetectionSettings();

            Verify(Method(mockEEPROMService, getStrokePhaseDetectionSettings)).Once();

            SECTION("and split StrokePhaseDetectionSettings correctly into bytes")
            {
                StrokeDetectionType strokeDetectionType = StrokeDetectionType::Torque;
                unsigned char impulseDataArrayLength = 0;
                float minimumPoweredTorqueZero = 0.0F;
                float minimumDragTorqueZero = 0.0F;
                float minimumRecoverySlopeZero = 0.0F;
                unsigned int minimumRecoveryTime = 0;
                unsigned int minimumDriveTime = 0;
                unsigned char driveHandleForcesMaxCapacity = 0;

                When(OverloadedMethod(mockStrokeSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::strokeSettingsPayloadSize>)))
                    .Do([&](const std::array<unsigned char, ISettingsBleService::strokeSettingsPayloadSize> &settings)
                        {
                            strokeDetectionType = static_cast<StrokeDetectionType>(settings[0] & 0x03);
                            impulseDataArrayLength = (settings[0] >> 2) & 0x1F;
                            minimumPoweredTorqueZero = static_cast<float>(static_cast<short>(settings[1] | settings[2] << 8)) / ISettingsBleService::poweredTorqueScale;
                            minimumDragTorqueZero = static_cast<float>(static_cast<short>(settings[3] | settings[4] << 8)) / ISettingsBleService::dragTorqueScale;
                            minimumRecoverySlopeZero = static_cast<float>(static_cast<short>(settings[5] | settings[6] << 8)) / ISettingsBleService::recoverySlopeScale;
                            const auto strokeTimes = static_cast<unsigned int>(settings[7] | settings[8] << 8 | settings[9] << 16);
                            minimumRecoveryTime = strokeTimes & 0xFFF;
                            minimumDriveTime = strokeTimes >> 12;
                            driveHandleForcesMaxCapacity = settings[10]; });

                settingsBleService.broadcastStrokeDetectionSettings();

                REQUIRE(strokeDetectionType == RowerProfile::Defaults::strokeDetectionType);
                REQUIRE(impulseDataArrayLength == RowerProfile::Defaults::impulseDataArrayLength);
                REQUIRE(minimumPoweredTorqueZero == RowerProfile::Defaults::minimumPoweredTorque);
                REQUIRE(minimumDragTorqueZero == RowerProfile::Defaults::minimumDragTorque);
                REQUIRE(minimumRecoverySlopeZero == RowerProfile::Defaults::minimumRecoverySlope);
                REQUIRE(minimumRecoveryTime == RowerProfile::Defaults::minimumRecoveryTime / ISettingsBleService::minimumStrokeTimesScale);
                REQUIRE(minimumDriveTime == RowerProfile::Defaults::minimumDriveTime / ISettingsBleService::minimumStrokeTimesScale);
                REQUIRE(driveHandleForcesMaxCapacity == RowerProfile::Defaults::driveHandleForcesMaxCapacity);
            }
        }

        SECTION("set new stroke detection settings")
        {
            settingsBleService.broadcastStrokeDetectionSettings();

            Verify(OverloadedMethod(mockStrokeSettingsCharacteristic, setValue, void(const std::array<unsigned char, ISettingsBleService::strokeSettingsPayloadSize>))
                       .Using(Eq(expectedInitialStrokeSettings)))
                .Once();
        }

        SECTION("notify")
        {
            settingsBleService.broadcastStrokeDetectionSettings();

            Verify(Method(mockStrokeSettingsCharacteristic, notify)).Once();
        }
    }
}
// NOLINTEND(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while, modernize-type-traits)