// NOLINTBEGIN(readability-magic-numbers)
#include <array>
#include <string>

#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "../../include/NimBLEDevice.h"

#include "../../../../src/peripherals/bluetooth/ble-services/settings.service.h"
#include "../../../../src/peripherals/sd-card/sd-card.service.interface.h"
#include "../../../../src/utils/EEPROM/EEPROM.service.interface.h"
#include "../../../../src/utils/configuration.h"
#include "../../../../src/utils/enums.h"

using namespace fakeit;

TEST_CASE("SettingsBleService", "[ble-service]")
{
    mockNimBLEServer.Reset();

    Mock<IEEPROMService> mockEEPROMService;
    Mock<ISdCardService> mockSdCardService;
    Mock<NimBLECharacteristic> mockSettingsCharacteristic;
    Mock<NimBLEService> mockSettingsService;

    const auto logToBluetooth = true;
    const auto logToSdCard = true;
    const auto logLevel = ArduinoLogLevel::LogLevelSilent;
    const auto logFileOpen = true;

    const unsigned char settings =
        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCard) + 1 : 0) << 2U) |
        (static_cast<unsigned char>(logLevel) << 4U);

    std::array<unsigned char, SettingsBleService::settingsArrayLength> expectedInitialSettings = {
        settings,
    };

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string))).AlwaysReturn(&mockSettingsService.get());

    When(OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))).AlwaysReturn(&mockSettingsCharacteristic.get());

    Fake(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>)));
    Fake(Method(mockSettingsCharacteristic, setCallbacks));

    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetooth);
    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCard);
    When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevel);
    When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(logFileOpen);

    SettingsBleService settingsBleService(mockSdCardService.get(), mockEEPROMService.get());

    SECTION("setup method should")
    {
        const unsigned int expectedSettingsProperty = NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ;
        const unsigned int expectedControlPointProperty = NIMBLE_PROPERTY::INDICATE | NIMBLE_PROPERTY::WRITE;

        SECTION("initialize settings BLE service with correct UUID")
        {
            settingsBleService.setup(&mockNimBLEServer.get());

            Verify(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string)).Using(CommonBleFlags::settingsServiceUuid)).Once();
        }

        SECTION("start settings BLE characteristic with correct UUID")
        {
            settingsBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockSettingsService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))
                    .Using(CommonBleFlags::settingsUuid, expectedSettingsProperty))
                .Once();
        }

        SECTION("set initial settings value")
        {
            settingsBleService.setup(&mockNimBLEServer.get());

            Verify(
                OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                    .Using(Eq(std::array<unsigned char, 1U>{expectedInitialSettings})))
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
            Verify(Method(mockSdCardService, isLogFileOpen)).Once();

            SECTION("and calculate correct setting binary value")
            {
                SECTION("when logToBluetooth is disabled")
                {
                    const auto logToBluetoothTest = false;

                    const unsigned char expectedSettings =
                        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetoothTest) + 1 : 0) << 0U) |
                        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCard) + 1 : 0) << 2U) |
                        (static_cast<unsigned char>(logLevel) << 4U);

                    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetoothTest);
                    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCard);
                    When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevel);
                    When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(logFileOpen);

                    settingsBleService.broadcastSettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                               .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})));
                }

                SECTION("when logToBluetooth is disabled")
                {
                    const auto logToBluetoothTest = false;

                    const unsigned char expectedSettings =
                        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetoothTest) + 1 : 0) << 0U) |
                        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCard) + 1 : 0) << 2U) |
                        (static_cast<unsigned char>(logLevel) << 4U);

                    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetoothTest);

                    settingsBleService.broadcastSettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                               .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})));
                }

                SECTION("when logToSdCard is disabled")
                {
                    const auto logToSdCardTest = false;

                    const unsigned char expectedSettings =
                        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
                        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCardTest) + 1 : 0) << 2U) |
                        (static_cast<unsigned char>(logLevel) << 4U);

                    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCardTest);

                    settingsBleService.broadcastSettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                               .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})));
                }

                SECTION("when logToSdCard is disabled and logLevel is Verbose")
                {
                    const auto logLevelTest = ArduinoLogLevel::LogLevelSilent;
                    const auto logToSdCardTest = false;

                    const unsigned char expectedSettings =
                        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
                        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCardTest) + 1 : 0) << 2U) |
                        (static_cast<unsigned char>(logLevelTest) << 4U);

                    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCardTest);
                    When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevelTest);

                    settingsBleService.broadcastSettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                               .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})));
                }
            }
        }

        SECTION("set new settings")
        {
            settingsBleService.broadcastSettings();

            Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                       .Using(Eq(std::array<unsigned char, 1U>{expectedInitialSettings})))
                .Once();
        }

        SECTION("notify")
        {
            settingsBleService.broadcastSettings();

            Verify(Method(mockSettingsCharacteristic, notify));
        }
    }
}
// NOLINTEND(readability-magic-numbers)