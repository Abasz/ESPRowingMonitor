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

    static constexpr const char *rotationDebounceAddress = "signalDebounce";
    static constexpr const char *rowingStoppedPeriodAddress = "rowingStopped";

    static constexpr const char *goodnessOfFitAddress = "goodnessOfFit";
    static constexpr const char *maxDragFactorRecoveryPeriodAddress = "maxDragPeriod";
    static constexpr const char *lowerDragFactorThresholdAddress = "lowerDragFactor";
    static constexpr const char *upperDragFactorThresholdAddress = "upperDragFactor";
    static constexpr const char *dragCoefficientsArrayLengthAddress = "dragArraySize";

    SECTION("setup method")
    {
        Mock<Preferences> mockPreferences;

        When(Method(mockPreferences, begin)).Return(true);
        When(Method(mockPreferences, freeEntries)).Return(100);
        When(Method(mockPreferences, isKey)).AlwaysReturn(false);

        When(Method(mockPreferences, putUChar)).AlwaysReturn(1);
        When(Method(mockPreferences, putUShort)).AlwaysReturn(1);
        When(Method(mockPreferences, putUInt)).AlwaysReturn(1);
        When(Method(mockPreferences, putBool)).AlwaysReturn(1);
        When(Method(mockPreferences, putFloat)).AlwaysReturn(1);

        When(Method(mockPreferences, getBool).Using(StrEq(bluetoothDeltaTimeLoggingAddress), Configurations::enableBluetoothDeltaTimeLogging)).Return(Configurations::enableBluetoothDeltaTimeLogging);
        When(Method(mockPreferences, getBool).Using(StrEq(sdCardLoggingAddress), false)).Return(false);
        When(Method(mockPreferences, getUChar).Using(StrEq(logLevelAddress), std::to_underlying(Configurations::defaultLogLevel))).Return(std::to_underlying(Configurations::defaultLogLevel));
        When(Method(mockPreferences, getUChar).Using(StrEq(bleServiceAddress), std::to_underlying(Configurations::defaultBleServiceFlag))).Return(std::to_underlying(Configurations::defaultBleServiceFlag));
        When(Method(mockPreferences, getFloat).Using(StrEq(flywheelInertiaAddress), RowerProfile::Defaults::flywheelInertia)).Return(RowerProfile::Defaults::flywheelInertia);
        When(Method(mockPreferences, getFloat).Using(StrEq(concept2MagicNumberAddress), RowerProfile::Defaults::concept2MagicNumber)).Return(RowerProfile::Defaults::concept2MagicNumber);
        When(Method(mockPreferences, getUChar).Using(StrEq(impulsesPerRevolutionAddress), RowerProfile::Defaults::impulsesPerRevolution)).Return(RowerProfile::Defaults::impulsesPerRevolution);
        When(Method(mockPreferences, getFloat).Using(StrEq(sprocketRadiusAddress), RowerProfile::Defaults::sprocketRadius)).Return(RowerProfile::Defaults::sprocketRadius);
        When(Method(mockPreferences, getUShort).Using(StrEq(rotationDebounceAddress), RowerProfile::Defaults::rotationDebounceTimeMin)).Return(RowerProfile::Defaults::rotationDebounceTimeMin);
        When(Method(mockPreferences, getUInt).Using(StrEq(rowingStoppedPeriodAddress), RowerProfile::Defaults::rowingStoppedThresholdPeriod)).Return(RowerProfile::Defaults::rowingStoppedThresholdPeriod);
        When(Method(mockPreferences, getFloat).Using(StrEq(goodnessOfFitAddress), RowerProfile::Defaults::goodnessOfFitThreshold)).Return(RowerProfile::Defaults::goodnessOfFitThreshold);
        When(Method(mockPreferences, getUInt).Using(StrEq(maxDragFactorRecoveryPeriodAddress), RowerProfile::Defaults::maxDragFactorRecoveryPeriod)).Return(RowerProfile::Defaults::maxDragFactorRecoveryPeriod);
        When(Method(mockPreferences, getFloat).Using(StrEq(lowerDragFactorThresholdAddress), RowerProfile::Defaults::lowerDragFactorThreshold)).Return(RowerProfile::Defaults::lowerDragFactorThreshold);
        When(Method(mockPreferences, getFloat).Using(StrEq(upperDragFactorThresholdAddress), RowerProfile::Defaults::upperDragFactorThreshold)).Return(RowerProfile::Defaults::upperDragFactorThreshold);
        When(Method(mockPreferences, getUChar).Using(StrEq(dragCoefficientsArrayLengthAddress), RowerProfile::Defaults::dragCoefficientsArrayLength)).Return(RowerProfile::Defaults::dragCoefficientsArrayLength);

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
            Verify(Method(mockPreferences, putFloat).Using(StrEq(flywheelInertiaAddress), RowerProfile::Defaults::flywheelInertia)).Once();
            Verify(Method(mockPreferences, isKey).Using(StrEq(concept2MagicNumberAddress))).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(concept2MagicNumberAddress), RowerProfile::Defaults::concept2MagicNumber)).Once();
            Verify(Method(mockPreferences, isKey).Using(StrEq(impulsesPerRevolutionAddress))).Once();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(impulsesPerRevolutionAddress), RowerProfile::Defaults::impulsesPerRevolution)).Once();
            Verify(Method(mockPreferences, isKey).Using(StrEq(sprocketRadiusAddress))).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(sprocketRadiusAddress), RowerProfile::Defaults::sprocketRadius)).Once();

            Verify(Method(mockPreferences, isKey).Using(StrEq(rotationDebounceAddress))).Once();
            Verify(Method(mockPreferences, putUShort).Using(StrEq(rotationDebounceAddress), RowerProfile::Defaults::rotationDebounceTimeMin)).Once();
            Verify(Method(mockPreferences, isKey).Using(StrEq(rowingStoppedPeriodAddress))).Once();
            Verify(Method(mockPreferences, putUInt).Using(StrEq(rowingStoppedPeriodAddress), RowerProfile::Defaults::rowingStoppedThresholdPeriod)).Once();

            Verify(Method(mockPreferences, isKey).Using(StrEq(goodnessOfFitAddress))).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(goodnessOfFitAddress), RowerProfile::Defaults::goodnessOfFitThreshold)).Once();
            Verify(Method(mockPreferences, isKey).Using(StrEq(maxDragFactorRecoveryPeriodAddress))).Once();
            Verify(Method(mockPreferences, putUInt).Using(StrEq(maxDragFactorRecoveryPeriodAddress), RowerProfile::Defaults::maxDragFactorRecoveryPeriod)).Once();
            Verify(Method(mockPreferences, isKey).Using(StrEq(lowerDragFactorThresholdAddress))).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(lowerDragFactorThresholdAddress), RowerProfile::Defaults::lowerDragFactorThreshold)).Once();
            Verify(Method(mockPreferences, isKey).Using(StrEq(upperDragFactorThresholdAddress))).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(upperDragFactorThresholdAddress), RowerProfile::Defaults::upperDragFactorThreshold)).Once();
            Verify(Method(mockPreferences, isKey).Using(StrEq(dragCoefficientsArrayLengthAddress))).Once();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(dragCoefficientsArrayLengthAddress), RowerProfile::Defaults::dragCoefficientsArrayLength)).Once();
#else
            Verify(Method(mockPreferences, isKey).Using(StrEq(flywheelInertiaAddress))).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(flywheelInertiaAddress), Any())).Never();
            Verify(Method(mockPreferences, isKey).Using(StrEq(concept2MagicNumberAddress))).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(concept2MagicNumberAddress), Any())).Never();
            Verify(Method(mockPreferences, isKey).Using(StrEq(impulsesPerRevolutionAddress))).Never();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(impulsesPerRevolutionAddress), Any())).Never();
            Verify(Method(mockPreferences, isKey).Using(StrEq(sprocketRadiusAddress))).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(sprocketRadiusAddress), Any())).Never();

            Verify(Method(mockPreferences, isKey).Using(StrEq(rotationDebounceAddress))).Never();
            Verify(Method(mockPreferences, putUShort).Using(StrEq(rotationDebounceAddress), Any())).Never();
            Verify(Method(mockPreferences, isKey).Using(StrEq(rowingStoppedPeriodAddress))).Never();
            Verify(Method(mockPreferences, putUInt).Using(StrEq(rowingStoppedPeriodAddress), Any())).Never();

            Verify(Method(mockPreferences, isKey).Using(StrEq(goodnessOfFitAddress))).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(goodnessOfFitAddress), Any())).Never();
            Verify(Method(mockPreferences, isKey).Using(StrEq(maxDragFactorRecoveryPeriodAddress))).Never();
            Verify(Method(mockPreferences, putUInt).Using(StrEq(maxDragFactorRecoveryPeriodAddress), Any())).Never();
            Verify(Method(mockPreferences, isKey).Using(StrEq(lowerDragFactorThresholdAddress))).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(lowerDragFactorThresholdAddress), Any())).Never();
            Verify(Method(mockPreferences, isKey).Using(StrEq(upperDragFactorThresholdAddress))).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(upperDragFactorThresholdAddress), Any())).Never();
            Verify(Method(mockPreferences, isKey).Using(StrEq(dragCoefficientsArrayLengthAddress))).Never();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(dragCoefficientsArrayLengthAddress), Any())).Never();
#endif
        }

        SECTION("should get keys from EEPROM")
        {
            Verify(Method(mockPreferences, getBool).Using(StrEq(bluetoothDeltaTimeLoggingAddress), Configurations::enableBluetoothDeltaTimeLogging)).Once();

            Verify(Method(mockPreferences, getBool).Using(StrEq(sdCardLoggingAddress), false)).Once();

            Verify(Method(mockPreferences, getUChar).Using(StrEq(logLevelAddress), std::to_underlying(Configurations::defaultLogLevel))).Once();

            Verify(Method(mockPreferences, getUChar).Using(StrEq(bleServiceAddress), std::to_underlying(Configurations::defaultBleServiceFlag))).Once();

#if ENABLE_RUNTIME_SETTINGS
            Verify(Method(mockPreferences, getFloat).Using(StrEq(flywheelInertiaAddress), RowerProfile::Defaults::flywheelInertia)).Once();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(concept2MagicNumberAddress), RowerProfile::Defaults::concept2MagicNumber)).Once();
            Verify(Method(mockPreferences, getUChar).Using(StrEq(impulsesPerRevolutionAddress), RowerProfile::Defaults::impulsesPerRevolution)).Once();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(sprocketRadiusAddress), RowerProfile::Defaults::sprocketRadius)).Once();

            Verify(Method(mockPreferences, getUShort).Using(StrEq(rotationDebounceAddress), RowerProfile::Defaults::rotationDebounceTimeMin)).Once();
            Verify(Method(mockPreferences, getUInt).Using(StrEq(rowingStoppedPeriodAddress), RowerProfile::Defaults::rowingStoppedThresholdPeriod)).Once();

            Verify(Method(mockPreferences, getFloat).Using(StrEq(goodnessOfFitAddress), RowerProfile::Defaults::goodnessOfFitThreshold)).Once();
            Verify(Method(mockPreferences, getUInt).Using(StrEq(maxDragFactorRecoveryPeriodAddress), RowerProfile::Defaults::maxDragFactorRecoveryPeriod)).Once();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(lowerDragFactorThresholdAddress), RowerProfile::Defaults::lowerDragFactorThreshold)).Once();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(upperDragFactorThresholdAddress), RowerProfile::Defaults::upperDragFactorThreshold)).Once();
            Verify(Method(mockPreferences, getUChar).Using(StrEq(dragCoefficientsArrayLengthAddress), RowerProfile::Defaults::dragCoefficientsArrayLength)).Once();
#else
            Verify(Method(mockPreferences, getFloat).Using(StrEq(flywheelInertiaAddress), Any())).Never();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(concept2MagicNumberAddress), Any())).Never();
            Verify(Method(mockPreferences, getUChar).Using(StrEq(impulsesPerRevolutionAddress), Any())).Never();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(sprocketRadiusAddress), Any())).Never();

            Verify(Method(mockPreferences, getUShort).Using(StrEq(rotationDebounceAddress), Any())).Never();
            Verify(Method(mockPreferences, getUInt).Using(StrEq(rowingStoppedPeriodAddress), Any())).Never();

            Verify(Method(mockPreferences, getFloat).Using(StrEq(goodnessOfFitAddress), Any())).Never();
            Verify(Method(mockPreferences, getUInt).Using(StrEq(maxDragFactorRecoveryPeriodAddress), Any())).Never();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(lowerDragFactorThresholdAddress), Any())).Never();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(upperDragFactorThresholdAddress), Any())).Never();
            Verify(Method(mockPreferences, getUChar).Using(StrEq(dragCoefficientsArrayLengthAddress), Any())).Never();
#endif
        }

        SECTION("should set initial values for getters")
        {
            REQUIRE(eepromService.getBleServiceFlag() == Configurations::defaultBleServiceFlag);
            REQUIRE(eepromService.getLogLevel() == Configurations::defaultLogLevel);
            REQUIRE(eepromService.getLogToBluetooth() == Configurations::enableBluetoothDeltaTimeLogging);
            REQUIRE(eepromService.getLogToSdCard() == false);

            REQUIRE(eepromService.getMachineSettings().flywheelInertia == RowerProfile::Defaults::flywheelInertia);
            REQUIRE(eepromService.getMachineSettings().concept2MagicNumber == RowerProfile::Defaults::concept2MagicNumber);
            REQUIRE(eepromService.getMachineSettings().impulsesPerRevolution == RowerProfile::Defaults::impulsesPerRevolution);
            REQUIRE(eepromService.getMachineSettings().sprocketRadius == RowerProfile::Defaults::sprocketRadius);

            REQUIRE(eepromService.getSensorSignalSettings().rotationDebounceTimeMin == RowerProfile::Defaults::rotationDebounceTimeMin);
            REQUIRE(eepromService.getSensorSignalSettings().rowingStoppedThresholdPeriod == RowerProfile::Defaults::rowingStoppedThresholdPeriod);

            REQUIRE(eepromService.getDragFactorSettings().goodnessOfFitThreshold == RowerProfile::Defaults::goodnessOfFitThreshold);
            REQUIRE(eepromService.getDragFactorSettings().maxDragFactorRecoveryPeriod == RowerProfile::Defaults::maxDragFactorRecoveryPeriod);
            REQUIRE(eepromService.getDragFactorSettings().lowerDragFactorThreshold == RowerProfile::Defaults::lowerDragFactorThreshold);
            REQUIRE(eepromService.getDragFactorSettings().upperDragFactorThreshold == RowerProfile::Defaults::upperDragFactorThreshold);
            REQUIRE(eepromService.getDragFactorSettings().dragCoefficientsArrayLength == RowerProfile::Defaults::dragCoefficientsArrayLength);
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

    SECTION("setSensorSignalSettings method should")
    {
        Mock<Preferences> mockPreferences;
        When(Method(mockPreferences, putUShort)).AlwaysReturn(1);
        When(Method(mockPreferences, putUInt)).AlwaysReturn(1);
        EEPROMService eepromService(mockPreferences.get());

#if ENABLE_RUNTIME_SETTINGS
        SECTION("not save if any value is invalid")
        {
            const auto invalidRowingStoppedThresholdPeriodTooLow = RowerProfile::SensorSignalSettings{
                .rotationDebounceTimeMin = 1'000,
                .rowingStoppedThresholdPeriod = 3'000'000,
            };

            eepromService.setSensorSignalSettings(invalidRowingStoppedThresholdPeriodTooLow);

            Verify(Method(mockPreferences, putUShort).Using(StrEq(rotationDebounceAddress), Any())).Never();
            Verify(Method(mockPreferences, putUInt).Using(StrEq(rowingStoppedPeriodAddress), Any())).Never();
        }

        SECTION("save new sensor signal settings without updating backing fields")
        {
            const auto newSensorSignalSettings = RowerProfile::SensorSignalSettings{
                .rotationDebounceTimeMin = 1'000,
                .rowingStoppedThresholdPeriod = 4'000'000,
            };

            eepromService.setSensorSignalSettings(newSensorSignalSettings);

            const auto sensorSignalSettings = eepromService.getSensorSignalSettings();

            Verify(Method(mockPreferences, putUShort).Using(StrEq(rotationDebounceAddress), Any())).Once();
            Verify(Method(mockPreferences, putUInt).Using(StrEq(rowingStoppedPeriodAddress), Any())).Once();
            REQUIRE(sensorSignalSettings.rotationDebounceTimeMin != newSensorSignalSettings.rotationDebounceTimeMin);
            REQUIRE(sensorSignalSettings.rowingStoppedThresholdPeriod != newSensorSignalSettings.rowingStoppedThresholdPeriod);
        }
#else
        SECTION("not save any value if runtime settings are not enabled")
        {
            eepromService.setMachineSettings(RowerProfile::MachineSettings{});

            Verify(Method(mockPreferences, putUShort).Using(StrEq(rotationDebounceAddress), Any())).Never();
            Verify(Method(mockPreferences, putUInt).Using(StrEq(rowingStoppedPeriodAddress), Any())).Never();
        }
#endif
    }

    SECTION("setDragFactorSettings method should")
    {
        Mock<Preferences> mockPreferences;
        When(Method(mockPreferences, putFloat)).AlwaysReturn(1);
        When(Method(mockPreferences, putUChar)).AlwaysReturn(1);
        When(Method(mockPreferences, putUInt)).AlwaysReturn(1);
        EEPROMService eepromService(mockPreferences.get());

#if ENABLE_RUNTIME_SETTINGS
        SECTION("not save if any value is invalid")
        {
            const auto invalidGoodnessOfFitLow = RowerProfile::DragFactorSettings{
                .goodnessOfFitThreshold = -1.0F,
                .maxDragFactorRecoveryPeriod = 1'000'000,
                .lowerDragFactorThreshold = 100.0F,
                .upperDragFactorThreshold = 200.0F,
                .dragCoefficientsArrayLength = 10,
            };

            const auto invalidGoodnessOfFitHigh = RowerProfile::DragFactorSettings{
                .goodnessOfFitThreshold = 1.1F,
                .maxDragFactorRecoveryPeriod = 1'000'000,
                .lowerDragFactorThreshold = 100.0F,
                .upperDragFactorThreshold = 200.0F,
                .dragCoefficientsArrayLength = 10,
            };

            const auto invalidLowerDragFactor = RowerProfile::DragFactorSettings{
                .goodnessOfFitThreshold = 0.9F,
                .maxDragFactorRecoveryPeriod = 1'000'000,
                .lowerDragFactorThreshold = -1.0F,
                .upperDragFactorThreshold = 1.0F,
                .dragCoefficientsArrayLength = 1,
            };

            const auto invalidHigherDragFactor = RowerProfile::DragFactorSettings{
                .goodnessOfFitThreshold = 0.9F,
                .maxDragFactorRecoveryPeriod = 1'000'000,
                .lowerDragFactorThreshold = 0.0F,
                .upperDragFactorThreshold = -1.0F,
                .dragCoefficientsArrayLength = 1,
            };

            const auto invalidDragFactorRecoveryPeriod = RowerProfile::DragFactorSettings{
                .goodnessOfFitThreshold = 0.9F,
                .maxDragFactorRecoveryPeriod = RowerProfile::Defaults::rotationDebounceTimeMin * 1'000U + 1U,
                .lowerDragFactorThreshold = 0.0F,
                .upperDragFactorThreshold = -1.0F,
                .dragCoefficientsArrayLength = 1,
            };

            const auto invalidDragCoefficientsArrayLength = RowerProfile::DragFactorSettings{
                .goodnessOfFitThreshold = 0.9F,
                .maxDragFactorRecoveryPeriod = 1'000'000,
                .lowerDragFactorThreshold = 100.0F,
                .upperDragFactorThreshold = 200.0F,
                .dragCoefficientsArrayLength = 0,
            };

            eepromService.setDragFactorSettings(invalidGoodnessOfFitLow);
            eepromService.setDragFactorSettings(invalidGoodnessOfFitHigh);
            eepromService.setDragFactorSettings(invalidLowerDragFactor);
            eepromService.setDragFactorSettings(invalidHigherDragFactor);
            eepromService.setDragFactorSettings(invalidDragFactorRecoveryPeriod);
            eepromService.setDragFactorSettings(invalidDragCoefficientsArrayLength);

            Verify(Method(mockPreferences, putFloat).Using(StrEq(goodnessOfFitAddress), Any())).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(goodnessOfFitAddress), Any())).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(lowerDragFactorThresholdAddress), Any())).Never();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(upperDragFactorThresholdAddress), Any())).Never();
            Verify(Method(mockPreferences, putUInt).Using(StrEq(maxDragFactorRecoveryPeriodAddress), Any())).Never();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(dragCoefficientsArrayLengthAddress), Any())).Never();
        }

        SECTION("save new drag factor settings without updating backing fields")
        {
            const auto expectedDragFactorSettings = RowerProfile::DragFactorSettings{
                .goodnessOfFitThreshold = 0.9F,
                .maxDragFactorRecoveryPeriod = 1'000'000,
                .lowerDragFactorThreshold = 100.0F,
                .upperDragFactorThreshold = 200.0F,
                .dragCoefficientsArrayLength = 10,
            };

            eepromService.setDragFactorSettings(expectedDragFactorSettings);

            const auto dragFactorSettings = eepromService.getDragFactorSettings();

            Verify(Method(mockPreferences, putFloat).Using(StrEq(goodnessOfFitAddress), expectedDragFactorSettings.goodnessOfFitThreshold)).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(lowerDragFactorThresholdAddress), expectedDragFactorSettings.lowerDragFactorThreshold)).Once();
            Verify(Method(mockPreferences, putFloat).Using(StrEq(upperDragFactorThresholdAddress), expectedDragFactorSettings.upperDragFactorThreshold)).Once();
            Verify(Method(mockPreferences, putUInt).Using(StrEq(maxDragFactorRecoveryPeriodAddress), expectedDragFactorSettings.maxDragFactorRecoveryPeriod)).Once();
            Verify(Method(mockPreferences, putUChar).Using(StrEq(dragCoefficientsArrayLengthAddress), expectedDragFactorSettings.dragCoefficientsArrayLength)).Once();

            REQUIRE(dragFactorSettings.goodnessOfFitThreshold != expectedDragFactorSettings.goodnessOfFitThreshold);
            REQUIRE(dragFactorSettings.maxDragFactorRecoveryPeriod != expectedDragFactorSettings.maxDragFactorRecoveryPeriod);
            REQUIRE(dragFactorSettings.lowerDragFactorThreshold != expectedDragFactorSettings.lowerDragFactorThreshold);
            REQUIRE(dragFactorSettings.upperDragFactorThreshold != expectedDragFactorSettings.upperDragFactorThreshold);
            REQUIRE(dragFactorSettings.dragCoefficientsArrayLength != expectedDragFactorSettings.dragCoefficientsArrayLength);
        }
#else
        SECTION("not save any value if runtime settings are not enabled")
        {
            eepromService.setDragFactorSettings(RowerProfile::DragFactorSettings{});

            Verify(Method(mockPreferences, getFloat).Using(StrEq(goodnessOfFitAddress), Any())).Never();
            Verify(Method(mockPreferences, getUInt).Using(StrEq(maxDragFactorRecoveryPeriodAddress), Any())).Never();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(lowerDragFactorThresholdAddress), Any())).Never();
            Verify(Method(mockPreferences, getFloat).Using(StrEq(upperDragFactorThresholdAddress), Any())).Never();
            Verify(Method(mockPreferences, getUChar).Using(StrEq(dragCoefficientsArrayLengthAddress), Any())).Never();
        }
#endif
    }
}
// NOLINTEND(readability-magic-numbers)