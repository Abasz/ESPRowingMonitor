// NOLINTBEGIN(readability-magic-numbers)
#include "catch_amalgamated.hpp"
#include "fakeit.hpp"

#include "Preferences.h"

#include "../../src/utils/EEPROM.service.h"

using namespace fakeit;

TEST_CASE("EEPROMService", "[utils]")
{
    const auto *const logLevelAddress = "logLevel";
    const auto *const bleServiceAddress = "bleService";
    const auto *const bluetoothDeltaTimeLoggingAddress = "bleLogging";
    const auto *const sdCardLoggingAddress = "sdCardLogging";

    SECTION("setup method")
    {
        Mock<Preferences> mockPreferences;

        When(Method(mockPreferences, begin)).Return(true);
        When(Method(mockPreferences, isKey)).AlwaysReturn(false);

        When(Method(mockPreferences, putUChar)).AlwaysReturn(1);
        When(Method(mockPreferences, putBool)).AlwaysReturn(1);

        When(Method(mockPreferences, getBool).Using(StrEq(bluetoothDeltaTimeLoggingAddress), Configurations::enableBluetoothDeltaTimeLogging)).Return(Configurations::enableBluetoothDeltaTimeLogging);
        When(Method(mockPreferences, getBool).Using(StrEq(sdCardLoggingAddress), false)).Return(false);
        When(Method(mockPreferences, getUChar).Using(StrEq(logLevelAddress), static_cast<unsigned char>(Configurations::defaultLogLevel))).Return(static_cast<unsigned char>(Configurations::defaultLogLevel));
        When(Method(mockPreferences, getUChar).Using(StrEq(bleServiceAddress), static_cast<unsigned char>(Configurations::defaultBleServiceFlag))).Return(static_cast<unsigned char>(Configurations::defaultBleServiceFlag));

        EEPROMService eepromService(mockPreferences.get());
        eepromService.setup();

        SECTION("should start EEPROM")
        {
            Verify(Method(mockPreferences, begin)).Once();
        }

        SECTION("should initialize keys to their defaults if do not exist")
        {
            Verify(Method(mockPreferences, isKey).Using(StrEq(logLevelAddress))).Once();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(logLevelAddress), static_cast<unsigned char>(Configurations::defaultLogLevel))).Once();

            Verify(Method(mockPreferences, isKey).Using(StrEq(bleServiceAddress))).Once();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(bleServiceAddress), static_cast<unsigned char>(Configurations::defaultBleServiceFlag))).Once();

            Verify(Method(mockPreferences, isKey).Using(StrEq(bluetoothDeltaTimeLoggingAddress))).Once();
            Verify(Method(mockPreferences, putBool).Using(StrEq(bluetoothDeltaTimeLoggingAddress), Configurations::enableBluetoothDeltaTimeLogging)).Once();

            Verify(Method(mockPreferences, isKey).Using(StrEq(sdCardLoggingAddress))).Once();
            Verify(Method(mockPreferences, putBool).Using(StrEq(sdCardLoggingAddress), false)).Once();
        }

        SECTION("should get keys from EEPROM")
        {
            Verify(Method(mockPreferences, getBool).Using(StrEq(bluetoothDeltaTimeLoggingAddress), Configurations::enableBluetoothDeltaTimeLogging)).Once();

            Verify(Method(mockPreferences, getBool).Using(StrEq(sdCardLoggingAddress), false)).Once();

            Verify(Method(mockPreferences, getUChar).Using(StrEq(logLevelAddress), static_cast<unsigned char>(Configurations::defaultLogLevel))).Once();

            Verify(Method(mockPreferences, getUChar).Using(StrEq(bleServiceAddress), static_cast<unsigned char>(Configurations::defaultBleServiceFlag))).Once();
        }

        SECTION("should set initial values for getters")
        {
            REQUIRE(eepromService.getBleServiceFlag() == Configurations::defaultBleServiceFlag);
            REQUIRE(eepromService.getLogLevel() == Configurations::defaultLogLevel);
            REQUIRE(eepromService.getLogToBluetooth() == Configurations::enableBluetoothDeltaTimeLogging);
            REQUIRE(eepromService.getLogToSdCard() == false);
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
            Verify(Method(mockPreferences, putUChar).Using(StrEq(logLevelAddress), static_cast<unsigned char>(newLogLevel))).Once();
        }

        SECTION("should ignore invalid level")
        {
            mockPreferences.ClearInvocationHistory();

            eepromService.setLogLevel(static_cast<ArduinoLogLevel>(7));

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
            Verify(Method(mockPreferences, putUChar).Using(StrEq(bleServiceAddress), static_cast<unsigned char>(bleService))).Once();
        }

        SECTION("should ignore invalid flag")
        {
            mockPreferences.ClearInvocationHistory();

            eepromService.setBleServiceFlag(static_cast<BleServiceFlag>(7));

            Verify(Method(mockPreferences, putUChar)).Exactly(0);
        }
    }
}
// NOLINTEND(readability-magic-numbers)