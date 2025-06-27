// NOLINTBEGIN(readability-magic-numbers)
#include <utility>

#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "./include/Preferences.h"

#include "../../src/utils/EEPROM/EEPROM.service.h"

using namespace fakeit;

TEST_CASE("EEPROMService", "[utils]")
{
    const auto *const logLevelAddress = "logLevel";
    const auto *const bleServiceAddress = "bleService";
    const auto *const bluetoothDeltaTimeLoggingAddress = "bleLogging";
    const auto *const sdCardLoggingAddress = "sdCardLogging";

    const auto *const flywheelInertiaAddress = "flywheelInertia";
    const auto *const concept2MagicNumberAddress = "magicNumber";
    const auto *const impulsesPerRevolutionAddress = "impulsesPerRev";
    const auto *const sprocketRadiusAddress = "sprocketRadius";

    SECTION("setup method")
    {
        Mock<Preferences> mockPreferences;

        When(Method(mockPreferences, begin)).Return(true);
        When(Method(mockPreferences, freeEntries)).Return(100);
        When(Method(mockPreferences, isKey)).AlwaysReturn(false);

        When(Method(mockPreferences, putUChar)).AlwaysReturn(1);
        When(Method(mockPreferences, putBool)).AlwaysReturn(1);
        When(Method(mockPreferences, putFloat)).AlwaysReturn(1);

        When(Method(mockPreferences, getBool).Using(StrEq(bluetoothDeltaTimeLoggingAddress), Configurations::enableBluetoothDeltaTimeLogging)).Return(Configurations::enableBluetoothDeltaTimeLogging);
        When(Method(mockPreferences, getBool).Using(StrEq(sdCardLoggingAddress), false)).Return(false);
        When(Method(mockPreferences, getUChar).Using(StrEq(logLevelAddress), std::to_underlying(Configurations::defaultLogLevel))).Return(std::to_underlying(Configurations::defaultLogLevel));
        When(Method(mockPreferences, getUChar).Using(StrEq(bleServiceAddress), std::to_underlying(Configurations::defaultBleServiceFlag))).Return(std::to_underlying(Configurations::defaultBleServiceFlag));
        When(Method(mockPreferences, getFloat).Using(StrEq(flywheelInertiaAddress), Configurations::flywheelInertia)).Return(Configurations::flywheelInertia);
        When(Method(mockPreferences, getFloat).Using(StrEq(concept2MagicNumberAddress), Configurations::concept2MagicNumber)).Return(Configurations::concept2MagicNumber);
        When(Method(mockPreferences, getUChar).Using(StrEq(impulsesPerRevolutionAddress), Configurations::impulsesPerRevolution)).Return(Configurations::impulsesPerRevolution);
        When(Method(mockPreferences, getFloat).Using(StrEq(sprocketRadiusAddress), Configurations::sprocketRadius)).Return(Configurations::sprocketRadius);

        EEPROMService eepromService(mockPreferences.get());
        eepromService.setup();

        SECTION("should start EEPROM")
        {
            Verify(Method(mockPreferences, begin)).Once();
        }

        SECTION("should initialize keys to their defaults if do not exist")
        {
            Verify(Method(mockPreferences, isKey).Using(StrEq(logLevelAddress))).Once();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(logLevelAddress), std::to_underlying(Configurations::defaultLogLevel))).Once();

            Verify(Method(mockPreferences, isKey).Using(StrEq(bleServiceAddress))).Once();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(bleServiceAddress), std::to_underlying(Configurations::defaultBleServiceFlag))).Once();

            Verify(Method(mockPreferences, isKey).Using(StrEq(bluetoothDeltaTimeLoggingAddress))).Once();
            Verify(Method(mockPreferences, putBool).Using(StrEq(bluetoothDeltaTimeLoggingAddress), Configurations::enableBluetoothDeltaTimeLogging)).Once();

            Verify(Method(mockPreferences, isKey).Using(StrEq(sdCardLoggingAddress))).Once();
            Verify(Method(mockPreferences, putBool).Using(StrEq(sdCardLoggingAddress), false)).Once();

#if ENABLE_RUNTIME_SETTINGS
            Verify(Method(mockPreferences, isKey).Using(StrEq(flywheelInertiaAddress))).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(flywheelInertiaAddress), Configurations::flywheelInertia)).Once();
            Verify(Method(mockPreferences, isKey).Using(StrEq(concept2MagicNumberAddress))).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(concept2MagicNumberAddress), Configurations::concept2MagicNumber)).Once();
            Verify(Method(mockPreferences, isKey).Using(StrEq(impulsesPerRevolutionAddress))).Once();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(impulsesPerRevolutionAddress), Configurations::impulsesPerRevolution)).Once();
            Verify(Method(mockPreferences, isKey).Using(StrEq(sprocketRadiusAddress))).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(sprocketRadiusAddress), Configurations::sprocketRadius)).Once();
#else
            Verify(Method(mockPreferences, isKey).Using(StrEq(flywheelInertiaAddress))).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(flywheelInertiaAddress), Any())).Never();
            Verify(Method(mockPreferences, isKey).Using(StrEq(concept2MagicNumberAddress))).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(concept2MagicNumberAddress), Any())).Never();
            Verify(Method(mockPreferences, isKey).Using(StrEq(impulsesPerRevolutionAddress))).Never();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(impulsesPerRevolutionAddress), Any())).Never();
            Verify(Method(mockPreferences, isKey).Using(StrEq(sprocketRadiusAddress))).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(sprocketRadiusAddress), Any())).Never();
#endif
        }

        SECTION("should get keys from EEPROM")
        {
            Verify(Method(mockPreferences, getBool).Using(StrEq(bluetoothDeltaTimeLoggingAddress), Configurations::enableBluetoothDeltaTimeLogging)).Once();

            Verify(Method(mockPreferences, getBool).Using(StrEq(sdCardLoggingAddress), false)).Once();

            Verify(Method(mockPreferences, getUChar).Using(StrEq(logLevelAddress), std::to_underlying(Configurations::defaultLogLevel))).Once();

            Verify(Method(mockPreferences, getUChar).Using(StrEq(bleServiceAddress), std::to_underlying(Configurations::defaultBleServiceFlag))).Once();

#if ENABLE_RUNTIME_SETTINGS
            Verify(Method(mockPreferences, getFloat).Using(StrEq(flywheelInertiaAddress), Configurations::flywheelInertia)).Once();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(concept2MagicNumberAddress), Configurations::concept2MagicNumber)).Once();
            Verify(Method(mockPreferences, getUChar).Using(StrEq(impulsesPerRevolutionAddress), Configurations::impulsesPerRevolution)).Once();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(sprocketRadiusAddress), Configurations::sprocketRadius)).Once();
#else
            Verify(Method(mockPreferences, getFloat).Using(StrEq(flywheelInertiaAddress), Any())).Never();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(concept2MagicNumberAddress), Any())).Never();
            Verify(Method(mockPreferences, getUChar).Using(StrEq(impulsesPerRevolutionAddress), Any())).Never();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(sprocketRadiusAddress), Any())).Never();
#endif
        }

        SECTION("should set initial values for getters")
        {
            REQUIRE(eepromService.getBleServiceFlag() == Configurations::defaultBleServiceFlag);
            REQUIRE(eepromService.getLogLevel() == Configurations::defaultLogLevel);
            REQUIRE(eepromService.getLogToBluetooth() == Configurations::enableBluetoothDeltaTimeLogging);
            REQUIRE(eepromService.getLogToSdCard() == false);

            REQUIRE(eepromService.getMachineSettings().flywheelInertia == Configurations::flywheelInertia);
            REQUIRE(eepromService.getMachineSettings().concept2MagicNumber == Configurations::concept2MagicNumber);
            REQUIRE(eepromService.getMachineSettings().impulsesPerRevolution == Configurations::impulsesPerRevolution);
            REQUIRE(eepromService.getMachineSettings().sprocketRadius == Configurations::sprocketRadius);
        }
    }

    SECTION("setLogLevel method")
    {
        Mock<Preferences> mockPreferences;
        When(Method(mockPreferences, putUChar)).AlwaysReturn(1);
        const auto newLogLevel = ArduinoLogLevel::LogLevelVerbose;

        EEPROMService eepromService(mockPreferences.get());

        SECTION("should set and save new log level")
        {
            eepromService.setLogLevel(newLogLevel);

            REQUIRE(eepromService.getLogLevel() == newLogLevel);
            Verify(Method(mockPreferences, putUChar).Using(StrEq(logLevelAddress), std::to_underlying(newLogLevel))).Once();
        }

        SECTION("should ignore invalid level")
        {
            mockPreferences.ClearInvocationHistory();

            eepromService.setLogLevel(ArduinoLogLevel{7});

            Verify(Method(mockPreferences, putUChar)).Exactly(0);
        }
    }

    SECTION("setLogToBluetooth method should set and save new log to bluetooth flag")
    {
        Mock<Preferences> mockPreferences;
        When(Method(mockPreferences, putBool)).AlwaysReturn(1);
        EEPROMService eepromService(mockPreferences.get());
        const auto newLogToBluetooth = !eepromService.getLogToBluetooth();

        eepromService.setLogToBluetooth(newLogToBluetooth);

        REQUIRE(eepromService.getLogToBluetooth() == newLogToBluetooth);
        Verify(Method(mockPreferences, putBool).Using(StrEq(bluetoothDeltaTimeLoggingAddress), newLogToBluetooth)).Once();
    }

    SECTION("setLogToSdCard method should set and save new log to SdCard flag")
    {
        Mock<Preferences> mockPreferences;
        When(Method(mockPreferences, putBool)).AlwaysReturn(1);
        EEPROMService eepromService(mockPreferences.get());
        const auto newLogToSdCard = !eepromService.getLogToSdCard();

        eepromService.setLogToSdCard(newLogToSdCard);

        REQUIRE(eepromService.getLogToSdCard() == newLogToSdCard);
        Verify(Method(mockPreferences, putBool).Using(StrEq(sdCardLoggingAddress), newLogToSdCard)).Once();
    }

    SECTION("setBleServiceFlag method")
    {
        Mock<Preferences> mockPreferences;
        When(Method(mockPreferences, putUChar)).AlwaysReturn(1);
        const auto bleService = BleServiceFlag::CscService;

        EEPROMService eepromService(mockPreferences.get());

        SECTION("should set and save new log level")
        {
            eepromService.setBleServiceFlag(bleService);

            REQUIRE(eepromService.getBleServiceFlag() == bleService);
            Verify(Method(mockPreferences, putUChar).Using(StrEq(bleServiceAddress), std::to_underlying(bleService))).Once();
        }

        SECTION("should ignore invalid flag")
        {
            mockPreferences.ClearInvocationHistory();

            eepromService.setBleServiceFlag(BleServiceFlag{7});

            Verify(Method(mockPreferences, putUChar)).Exactly(0);
        }
    }

    SECTION("setMachineSettings method should")
    {
        Mock<Preferences> mockPreferences;
        When(Method(mockPreferences, putFloat)).AlwaysReturn(1);
        When(Method(mockPreferences, putUChar)).AlwaysReturn(1);
        EEPROMService eepromService(mockPreferences.get());

#if ENABLE_RUNTIME_SETTINGS
        SECTION("not save if any value is invalid")
        {
            const auto invalidFlywheel = RowerProfile::MachineSettings{
                .impulsesPerRevolution = 1,
                .flywheelInertia = -1,
                .concept2MagicNumber = 1,
                .sprocketRadius = 1.0F,
            };

            const auto invalidMagicNumber = RowerProfile::MachineSettings{
                .impulsesPerRevolution = 1,
                .flywheelInertia = 1,
                .concept2MagicNumber = -1,
                .sprocketRadius = 1.0F,
            };

            const auto invalidSprocketRadius = RowerProfile::MachineSettings{
                .impulsesPerRevolution = 1,
                .flywheelInertia = 1,
                .concept2MagicNumber = 1,
                .sprocketRadius = -1.0F,
            };

            const auto invalidImpulsesPerRevolutionTooLow = RowerProfile::MachineSettings{
                .impulsesPerRevolution = 0,
                .flywheelInertia = 1,
                .concept2MagicNumber = 1,
                .sprocketRadius = 1.0F,
            };

            const auto invalidImpulsesPerRevolutionTooHigh = RowerProfile::MachineSettings{
                .impulsesPerRevolution = 13,
                .flywheelInertia = 1,
                .concept2MagicNumber = 1,
                .sprocketRadius = 1.0F,
            };

            eepromService.setMachineSettings(invalidFlywheel);
            eepromService.setMachineSettings(invalidMagicNumber);
            eepromService.setMachineSettings(invalidSprocketRadius);
            eepromService.setMachineSettings(invalidImpulsesPerRevolutionTooLow);
            eepromService.setMachineSettings(invalidImpulsesPerRevolutionTooHigh);

            Verify(Method(mockPreferences, putFloat).Using(StrEq(flywheelInertiaAddress), Any())).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(concept2MagicNumberAddress), Any())).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(sprocketRadiusAddress), Any())).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(impulsesPerRevolutionAddress), Any())).Never();
        }

        SECTION("save new machine settings without updating backing fields")
        {
            const auto newMachineSettings = RowerProfile::MachineSettings{
                .impulsesPerRevolution = 1,
                .flywheelInertia = 1,
                .concept2MagicNumber = 1,
                .sprocketRadius = 1,
            };

            eepromService.setMachineSettings(newMachineSettings);

            const auto machineSettings = eepromService.getMachineSettings();

            Verify(Method(mockPreferences, putFloat).Using(StrEq(flywheelInertiaAddress), newMachineSettings.flywheelInertia)).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(concept2MagicNumberAddress), newMachineSettings.concept2MagicNumber)).Once();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(impulsesPerRevolutionAddress), newMachineSettings.impulsesPerRevolution)).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(sprocketRadiusAddress), newMachineSettings.sprocketRadius)).Once();
            REQUIRE(machineSettings.flywheelInertia != newMachineSettings.flywheelInertia);
            REQUIRE(machineSettings.concept2MagicNumber != newMachineSettings.flywheelInertia);
            REQUIRE(machineSettings.impulsesPerRevolution != newMachineSettings.impulsesPerRevolution);
            REQUIRE(machineSettings.sprocketRadius != newMachineSettings.sprocketRadius);
        }
#else
        SECTION("not save any value if runtime settings are not enabled")
        {
            eepromService.setMachineSettings(RowerProfile::MachineSettings{});

            Verify(Method(mockPreferences, putFloat).Using(StrEq(flywheelInertiaAddress), Any())).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(concept2MagicNumberAddress), Any())).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(sprocketRadiusAddress), Any())).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(impulsesPerRevolutionAddress), Any())).Never();
        }
#endif
    }
}
// NOLINTEND(readability-magic-numbers)