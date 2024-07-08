// NOLINTBEGIN(readability-magic-numbers)
#include <cmath>

#include <array>
#include <string>
#include <vector>

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

TEST_CASE("BluetoothServer", "[callbacks]")
{
    Mock<IEEPROMService> mockEEPROMService;
    Mock<ISdCardService> mockSdCardService;
    Mock<NimBLECharacteristic> mockBatteryLevelCharacteristic;
    Mock<NimBLECharacteristic> mockSettingsCharacteristic;
    Mock<NimBLECharacteristic> mockHandleForcesCharacteristic;

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

    When(OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const unsigned short, const unsigned int)).Using(Eq(CommonBleFlags::batteryLevelUuid), Any())).AlwaysReturn(&mockBatteryLevelCharacteristic.get());
    When(Method(mockBatteryLevelCharacteristic, getSubscribedCount)).AlwaysReturn(0);
    Fake(Method(mockBatteryLevelCharacteristic, notify));
    Fake(OverloadedMethod(mockBatteryLevelCharacteristic, setValue, void(const unsigned short)));

    When(OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int)).Using(Eq(CommonBleFlags::settingsUuid), Any())).AlwaysReturn(&mockSettingsCharacteristic.get());
    When(Method(mockSettingsCharacteristic, getSubscribedCount)).AlwaysReturn(0);
    Fake(Method(mockSettingsCharacteristic, notify));
    Fake(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>)));

    When(OverloadedMethod(mockNimBLEService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int)).Using(Eq(CommonBleFlags::handleForcesUuid), Any())).AlwaysReturn(&mockHandleForcesCharacteristic.get());

    BluetoothService bluetoothService(mockEEPROMService.get(), mockSdCardService.get());
    bluetoothService.setup();
    NimBLECharacteristicCallbacks *handleForcesCallback = std::move(mockHandleForcesCharacteristic.get().callbacks);

    SECTION("notifyBattery method should")
    {
        const auto expectedBatteryLevel = 66;
        SECTION("set new battery level")
        {
            bluetoothService.notifyBattery(expectedBatteryLevel);

            Verify(OverloadedMethod(mockBatteryLevelCharacteristic, setValue, void(const unsigned short)).Using(Eq(expectedBatteryLevel)));
        }

        SECTION("not notify if there are no subscribers")
        {
            When(Method(mockBatteryLevelCharacteristic, getSubscribedCount)).Return(0);

            bluetoothService.notifyBattery(expectedBatteryLevel);

            VerifyNoOtherInvocations(Method(mockBatteryLevelCharacteristic, notify));
        }

        SECTION("notify if there are subscribers")
        {
            When(Method(mockBatteryLevelCharacteristic, getSubscribedCount)).Return(1);

            bluetoothService.notifyBattery(expectedBatteryLevel);

            Verify(Method(mockBatteryLevelCharacteristic, notify));
        }
    }

    SECTION("notifySettings method should")
    {
        const auto logToBluetooth = true;
        const auto logToSdCard = true;
        const auto logLevel = ArduinoLogLevel::LogLevelSilent;
        const auto logFileOpen = true;
        const unsigned char expectedSettings =
            ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
            ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCard) + 1 : 0) << 2U) |
            (static_cast<unsigned char>(logLevel) << 4U);

        When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetooth);
        When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCard);
        When(Method(mockEEPROMService, getLogLevel)).AlwaysReturn(logLevel);
        When(Method(mockSdCardService, isLogFileOpen)).AlwaysReturn(logFileOpen);

        SECTION("get current settings state")
        {
            mockEEPROMService.ClearInvocationHistory();
            mockSdCardService.ClearInvocationHistory();

            bluetoothService.notifySettings();

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

                    bluetoothService.notifySettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                               .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})))
                        .Once();
                }

                SECTION("when logToBluetooth is disabled")
                {
                    const auto logToBluetoothTest = false;

                    const unsigned char expectedSettings =
                        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetoothTest) + 1 : 0) << 0U) |
                        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCard) + 1 : 0) << 2U) |
                        (static_cast<unsigned char>(logLevel) << 4U);

                    When(Method(mockEEPROMService, getLogToBluetooth)).AlwaysReturn(logToBluetoothTest);

                    bluetoothService.notifySettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                               .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})))
                        .Once();
                }

                SECTION("when logToSdCard is disabled")
                {
                    const auto logToSdCardTest = false;

                    const unsigned char expectedSettings =
                        ((Configurations::enableBluetoothDeltaTimeLogging ? static_cast<unsigned char>(logToBluetooth) + 1 : 0) << 0U) |
                        ((Configurations::supportSdCardLogging && logFileOpen ? static_cast<unsigned char>(logToSdCardTest) + 1 : 0) << 2U) |
                        (static_cast<unsigned char>(logLevel) << 4U);

                    When(Method(mockEEPROMService, getLogToSdCard)).AlwaysReturn(logToSdCardTest);

                    bluetoothService.notifySettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                               .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})))
                        .Once();
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

                    bluetoothService.notifySettings();

                    Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                               .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})))
                        .Once();
                }
            }
        }

        SECTION("set new settings")
        {
            mockSettingsCharacteristic.ClearInvocationHistory();

            bluetoothService.notifySettings();

            Verify(OverloadedMethod(mockSettingsCharacteristic, setValue, void(const std::array<unsigned char, 1U>))
                       .Using(Eq(std::array<unsigned char, 1U>{expectedSettings})))
                .Once();
        }

        SECTION("not notify if there are no subscribers")
        {
            When(Method(mockSettingsCharacteristic, getSubscribedCount)).Return(0);

            bluetoothService.notifySettings();

            Verify(Method(mockSettingsCharacteristic, notify)).Never();
        }

        SECTION("notify if there are subscribers")
        {
            When(Method(mockSettingsCharacteristic, getSubscribedCount)).Return(1);

            bluetoothService.notifySettings();

            Verify(Method(mockSettingsCharacteristic, notify));
        }
    }

    SECTION("notifyBaseMetrics method should")
    {
        const unsigned short revTime = 31'000U;
        const unsigned int revCount = 360'000U;
        const unsigned short strokeTime = 30'100U;
        const unsigned short strokeCount = 2'000U;
        const short avgStrokePower = 300;
        const auto expectedStackSize = 2'048U;

        When(Method(mockNimBLECharacteristic, getSubscribedCount)).AlwaysReturn(1);
        Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 11U>)));
        Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 14U>)));
        Fake(Method(mockArduino, xTaskCreatePinnedToCore));
        Fake(Method(mockArduino, vTaskDelete));

        SECTION("not notify if there are no subscribers")
        {
            mockEEPROMService.ClearInvocationHistory();
            When(Method(mockNimBLECharacteristic, getSubscribedCount)).Return(0);

            bluetoothService.notifyBaseMetrics(revTime, revCount, strokeTime, strokeCount, avgStrokePower);

            Verify(Method(mockEEPROMService, getBleServiceFlag)).Never();
            Verify(Method(mockNimBLECharacteristic, notify)).Never();
            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 11U>))).Never();
            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 14U>))).Never();
        }

        SECTION("select correct service for notification")
        {
            mockNimBLECharacteristic.ClearInvocationHistory();
            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);

            bluetoothService.notifyBaseMetrics(revTime, revCount, strokeTime, strokeCount, avgStrokePower);

            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 11U>))).Never();
            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 14U>))).Once();

            mockNimBLECharacteristic.ClearInvocationHistory();
            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CscService);

            bluetoothService.notifyBaseMetrics(revTime, revCount, strokeTime, strokeCount, avgStrokePower);

            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 11U>))).Once();
            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 14U>))).Never();
        }

        SECTION("notify PSC with the correct binary data")
        {
            const auto length = 14U;
            std::array<unsigned char, length> expectedData = {
                static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag),
                static_cast<unsigned char>(PSCSensorBleFlags::pscMeasurementFeaturesFlag >> 8),

                static_cast<unsigned char>(avgStrokePower),
                static_cast<unsigned char>(avgStrokePower >> 8),

                static_cast<unsigned char>(revCount),
                static_cast<unsigned char>(revCount >> 8),
                static_cast<unsigned char>(revCount >> 16),
                static_cast<unsigned char>(revCount >> 24),
                static_cast<unsigned char>(revTime),
                static_cast<unsigned char>(revTime >> 8),

                static_cast<unsigned char>(strokeCount),
                static_cast<unsigned char>(strokeCount >> 8),
                static_cast<unsigned char>(strokeTime),
                static_cast<unsigned char>(strokeTime >> 8),
            };
            mockNimBLECharacteristic.ClearInvocationHistory();
            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);

            bluetoothService.notifyBaseMetrics(revTime, revCount, strokeTime, strokeCount, avgStrokePower);

            Verify(Method(mockArduino, xTaskCreatePinnedToCore).Using(Ne(nullptr), StrEq("notifyClients"), Eq(expectedStackSize), Ne(nullptr), Eq(1U), Any(), Eq(0))).Once();
            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 11U>))).Never();
            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 14U>)).Using(Eq(expectedData))).Once();
        }

        SECTION("notify CSC with the correct binary data")
        {
            const auto length = 11U;
            std::array<unsigned char, length> expectedData = {
                CSCSensorBleFlags::cscMeasurementFeaturesFlag,

                static_cast<unsigned char>(revCount),
                static_cast<unsigned char>(revCount >> 8),
                static_cast<unsigned char>(revCount >> 16),
                static_cast<unsigned char>(revCount >> 24),

                static_cast<unsigned char>(revTime),
                static_cast<unsigned char>(revTime >> 8),

                static_cast<unsigned char>(strokeCount),
                static_cast<unsigned char>(strokeCount >> 8),
                static_cast<unsigned char>(strokeTime),
                static_cast<unsigned char>(strokeTime >> 8)};
            mockNimBLECharacteristic.ClearInvocationHistory();

            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CscService);

            bluetoothService.notifyBaseMetrics(revTime, revCount, strokeTime, strokeCount, avgStrokePower);

            Verify(Method(mockArduino, xTaskCreatePinnedToCore).Using(Ne(nullptr), StrEq("notifyClients"), Eq(expectedStackSize), Ne(nullptr), Eq(1U), Any(), Eq(0))).Once();
            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 11U>)).Using(Eq(expectedData))).Once();
            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 14U>))).Never();
        }

        SECTION("delete task")
        {
            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CpsService);
            mockArduino.ClearInvocationHistory();

            bluetoothService.notifyBaseMetrics(revTime, revCount, strokeTime, strokeCount, avgStrokePower);

            Verify(Method(mockArduino, vTaskDelete).Using(nullptr)).Once();

            When(Method(mockEEPROMService, getBleServiceFlag)).AlwaysReturn(BleServiceFlag::CscService);
            mockArduino.ClearInvocationHistory();

            bluetoothService.notifyBaseMetrics(revTime, revCount, strokeTime, strokeCount, avgStrokePower);

            Verify(Method(mockArduino, vTaskDelete).Using(nullptr)).Once();
        }
    }

    SECTION("notifyExtendedMetrics method should")
    {
        const auto secInMicroSec = 1e6;
        const unsigned int recoveryDuration = 4'000'000;
        const unsigned int driveDuration = 3'000'000;
        const unsigned char dragFactor = 110;
        const short avgStrokePower = 300;
        const unsigned short calculatedRecoveryDuration = lround(recoveryDuration / secInMicroSec * 4'096);
        const unsigned short calculatedDriveDuration = lround(driveDuration / secInMicroSec * 4'096);
        const auto expectedStackSize = 1'800U;

        When(Method(mockNimBLECharacteristic, getSubscribedCount)).AlwaysReturn(1);
        Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 7U>)));
        Fake(Method(mockArduino, xTaskCreatePinnedToCore));
        Fake(Method(mockArduino, vTaskDelete));

        SECTION("not notify if there are no subscribers")
        {
            mockEEPROMService.ClearInvocationHistory();

            When(Method(mockNimBLECharacteristic, getSubscribedCount)).Return(0);

            bluetoothService.notifyExtendedMetrics(avgStrokePower, recoveryDuration, driveDuration, dragFactor);

            Verify(Method(mockEEPROMService, getBleServiceFlag)).Never();
            Verify(Method(mockNimBLECharacteristic, notify)).Never();
            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 7U>))).Never();
        }

        SECTION("convert recovery and drive duration to a 16bit unsigned short in seconds with a resolution of 4096")
        {
            const unsigned short expectedRecoveryDuration = lroundl(recoveryDuration / secInMicroSec * 4'096);
            const unsigned short expectedDriveDuration = lroundl(driveDuration / secInMicroSec * 4'096);

            const auto length = 7U;
            std::array<unsigned char, length> expectedData = {
                static_cast<unsigned char>(avgStrokePower),
                static_cast<unsigned char>(avgStrokePower >> 8),

                static_cast<unsigned char>(expectedDriveDuration),
                static_cast<unsigned char>(expectedDriveDuration >> 8),
                static_cast<unsigned char>(expectedRecoveryDuration),
                static_cast<unsigned char>(expectedRecoveryDuration >> 8),

                dragFactor,
            };

            bluetoothService.notifyExtendedMetrics(avgStrokePower, recoveryDuration, driveDuration, dragFactor);

            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 7U>)).Using(Eq(expectedData))).Once();
        }

        SECTION("notify ExtendedMetrics with the correct binary data")
        {
            const auto length = 7U;
            std::array<unsigned char, length> expectedData = {
                static_cast<unsigned char>(avgStrokePower),
                static_cast<unsigned char>(avgStrokePower >> 8),

                static_cast<unsigned char>(calculatedDriveDuration),
                static_cast<unsigned char>(calculatedDriveDuration >> 8),
                static_cast<unsigned char>(calculatedRecoveryDuration),
                static_cast<unsigned char>(calculatedRecoveryDuration >> 8),

                dragFactor,
            };
            mockNimBLECharacteristic.ClearInvocationHistory();

            bluetoothService.notifyExtendedMetrics(avgStrokePower, recoveryDuration, driveDuration, dragFactor);

            Verify(
                Method(mockArduino, xTaskCreatePinnedToCore)
                    .Using(Ne(nullptr), StrEq("notifyExtendedMetrics"), Eq(expectedStackSize), Ne(nullptr), Eq(1U), Any(), Eq(0)))
                .Once();
            Verify(
                OverloadedMethod(mockNimBLECharacteristic, setValue, void(const std::array<unsigned char, 7U>))
                    .Using(Eq(expectedData)))
                .Once();
        }

        SECTION("delete task")
        {
            mockArduino.ClearInvocationHistory();

            bluetoothService.notifyExtendedMetrics(avgStrokePower, recoveryDuration, driveDuration, dragFactor);

            Verify(Method(mockArduino, vTaskDelete).Using(nullptr)).Once();
        }
    }

    SECTION("notifyDeltaTimes method should")
    {
        const auto coreStackSize = 1'850U;
        std::vector<unsigned long> expectedDeltaTimes{10000, 11000, 12000, 11000};
        const auto expectedStackSize = coreStackSize + expectedDeltaTimes.size() * sizeof(unsigned long) / 3;

        When(Method(mockNimBLECharacteristic, getSubscribedCount)).AlwaysReturn(1);
        Fake(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const unsigned char *data, size_t length)));
        Fake(Method(mockArduino, xTaskCreatePinnedToCore));
        Fake(Method(mockArduino, vTaskDelete));

        SECTION("not notify if there are no subscribers")
        {
            mockEEPROMService.ClearInvocationHistory();

            When(Method(mockNimBLECharacteristic, getSubscribedCount)).Return(0);

            bluetoothService.notifyDeltaTimes(expectedDeltaTimes);

            Verify(Method(mockEEPROMService, getBleServiceFlag)).Never();
            Verify(Method(mockNimBLECharacteristic, notify)).Never();
            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const unsigned char *data, size_t length))).Never();
        }

        SECTION("not notify if deltaTimes vector is empty")
        {
            mockEEPROMService.ClearInvocationHistory();

            bluetoothService.notifyDeltaTimes({});

            Verify(Method(mockEEPROMService, getBleServiceFlag)).Never();
            Verify(Method(mockNimBLECharacteristic, notify)).Never();
            Verify(OverloadedMethod(mockNimBLECharacteristic, setValue, void(const unsigned char *data, size_t length))).Never();
        }

        SECTION("calculate core stack size for the task")
        {
            mockEEPROMService.ClearInvocationHistory();

            bluetoothService.notifyDeltaTimes(expectedDeltaTimes);

            Verify(
                Method(mockArduino, xTaskCreatePinnedToCore)
                    .Using(Any(), Any(), Eq(expectedStackSize), Any(), Any(), Any(), Any()))
                .Once();
        }

        SECTION("start task and notify notifyDeltaTimes with the correct binary data")
        {
            mockNimBLECharacteristic.ClearInvocationHistory();

            bluetoothService.notifyDeltaTimes(expectedDeltaTimes);

            Verify(
                Method(mockArduino, xTaskCreatePinnedToCore)
                    .Using(Ne(nullptr), StrEq("notifyDeltaTimes"), Eq(expectedStackSize), Ne(nullptr), Eq(1U), Any(), Eq(0)))
                .Once();
            Verify(
                OverloadedMethod(mockNimBLECharacteristic, setValue, void(const unsigned char *data, size_t length))
                    // TODO: somehow match the expectedDeltaTimes.data() pointer to the object
                    .Using(Any(), expectedDeltaTimes.size() * sizeof(unsigned long)))
                .Once();
        }

        SECTION("delete task")
        {
            mockArduino.ClearInvocationHistory();

            bluetoothService.notifyDeltaTimes(expectedDeltaTimes);

            Verify(Method(mockArduino, vTaskDelete).Using(nullptr)).Once();
        }
    }

    SECTION("notifyHandleForces method should")
    {
        const auto coreStackSize = 2'048U;
        const std::vector<float> expectedHandleForces{1.1, 3.3, 500.4, 300.4};
        const std::vector<float> expectedBigHandleForces{1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12};

        When(Method(mockNimBLEService, getServer)).AlwaysReturn(&mockNimBLEServer.get());

        When(Method(mockHandleForcesCharacteristic, getService)).AlwaysReturn(&mockNimBLEService.get());
        When(Method(mockHandleForcesCharacteristic, getUUID)).AlwaysReturn(NimBLEUUID{CommonBleFlags::handleForcesUuid});
        When(Method(mockHandleForcesCharacteristic, getSubscribedCount)).AlwaysReturn(1);
        Fake(OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const unsigned char *data, size_t length)));
        Fake(Method(mockHandleForcesCharacteristic, notify));
        Fake(Method(mockArduino, xTaskCreatePinnedToCore));
        Fake(Method(mockArduino, vTaskDelete));

        SECTION("not notify if there are no subscribers")
        {
            mockEEPROMService.ClearInvocationHistory();

            When(Method(mockHandleForcesCharacteristic, getSubscribedCount)).Return(0);

            bluetoothService.notifyHandleForces(expectedHandleForces);

            Verify(Method(mockEEPROMService, getBleServiceFlag)).Never();
            Verify(Method(mockHandleForcesCharacteristic, notify)).Never();
            Verify(OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const unsigned char *data, size_t length))).Never();
        }

        SECTION("not notify if handleForces vector is empty")
        {
            mockEEPROMService.ClearInvocationHistory();

            bluetoothService.notifyHandleForces({});

            Verify(Method(mockEEPROMService, getBleServiceFlag)).Never();
            Verify(Method(mockHandleForcesCharacteristic, notify)).Never();
            Verify(OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const unsigned char *data, size_t length))).Never();
        }

        SECTION("calculate MTU")
        {
            SECTION("and return the lowest MTU")
            {
                const auto expectedMTU = 23U;
                ble_gap_conn_desc first = {0};
                ble_gap_conn_desc second = {1};

                When(Method(mockNimBLEServer, getPeerMTU)).Return(expectedMTU, 100, 100);

                handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &first, 0);
                handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &second, 0);

                bluetoothService.notifyHandleForces(expectedBigHandleForces);

                Verify(
                    OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const unsigned char *data, size_t length))
                        .Using(Any(), Eq(expectedMTU - 3U - 2U - ((expectedMTU - 3U) % sizeof(float)))))
                    .AtLeastOnce();
            }

            SECTION("and return 512 as MTU even if device reports higher")
            {
                const auto expectedMTU = 512U;
                ble_gap_conn_desc first = {0};
                ble_gap_conn_desc second = {1};

                When(Method(mockNimBLEServer, getPeerMTU)).Return(1200, 1000);

                handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &first, 0);
                handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &second, 0);

                bluetoothService.notifyHandleForces(expectedBigHandleForces);

                Verify(
                    OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const unsigned char *data, size_t length))
                        .Using(Any(), Eq(expectedMTU - 3U - 2U - ((expectedMTU - 3U) % sizeof(float)))))
                    .AtLeastOnce();
            }

            SECTION("and ignore zero MTU when calculating minimum")
            {
                const auto expectedMTU = 23U;
                ble_gap_conn_desc first = {0};
                ble_gap_conn_desc second = {1};

                When(Method(mockNimBLEServer, getPeerMTU)).Return(0, expectedMTU, 100);

                handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &first, 0);
                handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &second, 0);

                bluetoothService.notifyHandleForces(expectedBigHandleForces);

                Verify(
                    OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const unsigned char *data, size_t length))
                        .Using(Any(), Eq(expectedMTU - 3U - 2U - ((expectedMTU - 3U) % sizeof(float)))))
                    .AtLeastOnce();
            }
        }

        SECTION("calculate stack size for the task")
        {
            SECTION("when MTU is less than handleForces vector size")
            {
                const auto expectedMTU = 100U;
                const auto expectedStackSize = coreStackSize + expectedMTU / 3;
                ble_gap_conn_desc first = {0};

                When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(expectedMTU);

                handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &first, 0);

                bluetoothService.notifyHandleForces(expectedBigHandleForces);

                Verify(
                    Method(mockArduino, xTaskCreatePinnedToCore)
                        .Using(Any(), Any(), Eq(expectedStackSize), Any(), Any(), Any(), Any()))
                    .Once();
            }

            SECTION("when MTU is greater than handleForces vector size")
            {
                const auto expectedMTU = 100U;
                const auto expectedStackSize = coreStackSize + (expectedHandleForces.size() * sizeof(float)) / 3;
                ble_gap_conn_desc first = {0};

                When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(expectedMTU);

                handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &first, 0);

                bluetoothService.notifyHandleForces(expectedHandleForces);

                Verify(
                    Method(mockArduino, xTaskCreatePinnedToCore)
                        .Using(Any(), Any(), Eq(expectedStackSize), Any(), Any(), Any(), Any()))
                    .Once();
            }
        }

        SECTION("should start a task")
        {
            bluetoothService.notifyHandleForces(expectedHandleForces);

            Verify(
                Method(mockArduino, xTaskCreatePinnedToCore)
                    .Using(Ne(nullptr), StrEq("notifyHandleForces"), Any(), Ne(nullptr), Eq(1U), Any(), Eq(0)))
                .Once();
        }

        SECTION("send")
        {
            const auto expectedMTU = 100U;
            ble_gap_conn_desc first = {0};

            SECTION("one notify when all handleForces plus header can fit within the MTU")
            {
                When(Method(mockNimBLEServer, getPeerMTU)).Return(expectedMTU);
                handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &first, 0);

                bluetoothService.notifyHandleForces(expectedHandleForces);

                Verify(
                    OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const unsigned char *data, size_t length))
                        .Using(Any(), Eq(2U + expectedHandleForces.size() * sizeof(float))))
                    .Once();
            }

            SECTION("multiple consecutive notifies when handleForces plus header do not fit within the MTU")
            {
                handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &first, 0);
                const auto expectedMTU = 512U;

                for (size_t i = 0; expectedMTU - i > 23; i++)
                {
                    const unsigned short expectedChunkSize = (expectedMTU - i - 3U - 2U) / sizeof(float);
                    const unsigned char expectedNumberOfNotifies = expectedBigHandleForces.size() / expectedChunkSize;

                    INFO("MTU: " << expectedMTU - i);
                    mockHandleForcesCharacteristic.ClearInvocationHistory();
                    When(Method(mockNimBLEServer, getPeerMTU)).Return(expectedMTU - i);

                    bluetoothService.notifyHandleForces(expectedBigHandleForces);

                    Verify(
                        OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const unsigned char *data, size_t length))
                            .Using(Any(), Eq(2U + expectedChunkSize * sizeof(float))))
                        .Exactly(expectedNumberOfNotifies);

                    Verify(
                        OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const unsigned char *data, size_t length))
                            .Using(Any(), Eq(2U + (expectedBigHandleForces.size() % expectedChunkSize) * sizeof(float))))
                        .Exactly(expectedBigHandleForces.size() % expectedChunkSize == 0 ? 0 : 1);
                }
            }
        }

        SECTION("send the total number of chunks and the current chunk id as the first two bytes")
        {
            const auto expectedMTU = 100;
            ble_gap_conn_desc first = {0};
            std::vector<std::vector<unsigned char>> results;

            const unsigned short expectedChunkSize = (expectedMTU - 3U - 2U) / sizeof(float);
            const unsigned char expectedNumberOfNotifies = expectedBigHandleForces.size() / expectedChunkSize + (expectedBigHandleForces.size() % expectedChunkSize == 0 ? 0 : 1);

            mockHandleForcesCharacteristic.ClearInvocationHistory();
            When(Method(mockNimBLEServer, getPeerMTU)).Return(expectedMTU);
            Fake(
                OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const unsigned char *data, size_t length)).Matching([&results](const auto data, const auto length)
                                                                                                                                    { 
                    results.push_back(std::vector<unsigned char>(data, data + length));
  
                    return true; }));

            handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &first, 0);
            bluetoothService.notifyHandleForces(expectedBigHandleForces);

            for (unsigned char i = 0; i < expectedNumberOfNotifies; i++)
            {
                INFO("Number of total notifies: " << (int)expectedNumberOfNotifies << " Current notify: " << i + 1U);
                // NOLINTBEGIN(bugprone-chained-comparison)
                REQUIRE(results[i][0] == expectedNumberOfNotifies);
                REQUIRE(results[i][1] == i + 1);
                // NOLINTEND(bugprone-chained-comparison)
            }
        }

        SECTION("correctly chunk handleForces data between notifies")
        {
            const auto expectedMTU = 100;
            ble_gap_conn_desc first = {0};
            std::vector<std::vector<unsigned char>> results;

            const unsigned short expectedChunkSize = (expectedMTU - 3U - 2U) / sizeof(float);
            const unsigned char expectedNumberOfNotifies = expectedBigHandleForces.size() / expectedChunkSize + (expectedBigHandleForces.size() % expectedChunkSize == 0 ? 0 : 1);

            mockHandleForcesCharacteristic.ClearInvocationHistory();
            When(Method(mockNimBLEServer, getPeerMTU)).Return(expectedMTU);
            Fake(
                OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const unsigned char *data, size_t length)).Matching([&results](const auto data, const auto length)
                                                                                                                                    { 
                    results.push_back(std::vector<unsigned char>(data, data + length));
  
                    return true; }));

            handleForcesCallback->onSubscribe(&mockHandleForcesCharacteristic.get(), &first, 0);
            bluetoothService.notifyHandleForces(expectedBigHandleForces);

            for (unsigned char i = 0; i < expectedNumberOfNotifies; i++)
            {
                std::vector<float> parsedHandleForces;
                size_t numOfFloats = (results[i].size() - 2) / sizeof(float);
                for (size_t j = 0; j < numOfFloats; ++j)
                {
                    float value = NAN;
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    std::memcpy(&value, results[i].data() + 2 + j * sizeof(float), sizeof(float));
                    parsedHandleForces.push_back(value);
                }

                // NOLINTNEXTLINE
                const auto current = std::vector<float>(cbegin(expectedBigHandleForces) + expectedChunkSize * i, cbegin(expectedBigHandleForces) + expectedChunkSize * i + numOfFloats);

                INFO("Current notify: " << i + 1U);
                REQUIRE_THAT(current, Catch::Matchers::RangeEquals(parsedHandleForces, [](float first, float second)
                                                                   { return std::abs(first - second) < 0.00001F; }));
            }

            Verify(Method(mockHandleForcesCharacteristic, notify)).Exactly(expectedNumberOfNotifies);
        }

        SECTION("delete task")
        {
            mockArduino.ClearInvocationHistory();

            bluetoothService.notifyHandleForces(expectedHandleForces);

            Verify(Method(mockArduino, vTaskDelete).Using(nullptr)).Once();
        }
    }
}
// NOLINTEND(readability-magic-numbers)