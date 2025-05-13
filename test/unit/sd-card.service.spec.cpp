// NOLINTBEGIN(readability-magic-numbers)
#include "catch2/catch_test_macros.hpp"
#include "fakeit.hpp"

#include "./include/Arduino.h"
#include "./include/SdFat.h"

#include "../../src/peripherals/sd-card/sd-card.service.h"
#include "../../src/utils/configuration.h"

using namespace fakeit;

TEST_CASE("SdCardService", "[peripheral]")
{
    mockSdFat32.Reset();
    mockFile32.Reset();
    mockArduino.Reset();

    SECTION("setup method")
    {
        File32 root;
        SdCardService sdCardService;

        When(Method(mockSdFat32, begin)).AlwaysReturn(true);
        When(Method(mockSdFat32, open)).AlwaysReturn(root);
        Fake(Method(mockSdFat32, end));
        When(Method(mockFile32, isOpen)).AlwaysReturn(true);
        When(Method(mockFile32, openNext)).AlwaysReturn(false);
        When(Method(mockFile32, open)).AlwaysReturn(true);
        When(Method(mockFile32, close)).AlwaysReturn(true);

        SECTION("should initialize SD card with correct parameters")
        {
            sdCardService.setup();

            Verify(Method(mockSdFat32, begin).Matching([](SdSpiConfig config)
                                                       { return config.csPin == Configurations::sdCardChipSelectPin && config.options == SHARED_SPI && config.maxSck == SD_SCK_MHZ(26); }))
                .Once();
        }

        SECTION("should handle SD card initialization failure")
        {
            When(Method(mockSdFat32, begin)).AlwaysReturn(false);

            sdCardService.setup();

            Verify(Method(mockSdFat32, begin).Matching([](SdSpiConfig config)
                                                       { return config.csPin == Configurations::sdCardChipSelectPin && config.options == SHARED_SPI && config.maxSck == SD_SCK_MHZ(26); }))
                .Once();
            Verify(Method(mockSdFat32, end)).Once();
            VerifyNoOtherInvocations(mockFile32);
        }

        SECTION("should open root directory on SD card")
        {
            sdCardService.setup();

            Verify(Method(mockSdFat32, open).Using(StrEq("/"), O_RDONLY)).Once();
            Verify(Method(mockFile32, isOpen)).Once();
        }

        SECTION("should handle opening root directory failure")
        {
            When(Method(mockFile32, isOpen)).AlwaysReturn(false);

            sdCardService.setup();

            Verify(Method(mockSdFat32, open).Using(StrEq("/"), O_RDONLY)).Once();
            Verify(Method(mockFile32, isOpen)).Once();
            Verify(Method(mockFile32, close)).Once();
            Verify(Method(mockSdFat32, end)).Once();
        }

        SECTION("should count number of files already on the SD card")
        {
            When(Method(mockFile32, openNext)).Return(true, true, false);

            sdCardService.setup();

            // TODO: Need to find a way to match to the root variable instance as that should be used exactly
            Verify(Method(mockFile32, openNext).Using(root, O_RDONLY)).Exactly(3);
            Verify(Method(mockFile32, close)).Exactly(3);
            Verify(Method(mockFile32, open).Using("2.txt", Eq(O_WRITE | O_CREAT | O_AT_END))).Once();
        }

        SECTION("should open the log file")
        {
            sdCardService.setup();

            // TODO: Need to find a way to match to the root variable instance as that should be used exactly
            Verify(Method(mockFile32, open).Using("0.txt", Eq(O_WRITE | O_CREAT | O_AT_END))).Once();
        }

        SECTION("should handle log file opening failure")
        {
            When(Method(mockFile32, open)).AlwaysReturn(false);

            sdCardService.setup();

            // TODO: Need to find a way to match to the root variable instance as that should be used exactly
            Verify(Method(mockSdFat32, end)).Once();
        }
    }

    SECTION("saveDeltaTime method")
    {
        const auto safeFlushPeriod = 30'000U;
        const auto expectedStackSize = 2'048U;

        SdCardService sdCardService;

        mockSdFat32.Reset();
        mockFile32.Reset();
        mockArduino.Reset();
        Fake(Method(mockSdFat32, end));
        When(Method(mockFile32, isOpen)).AlwaysReturn(true);
        When(Method(mockFile32, close)).AlwaysReturn(true);
        Fake(Method(mockFile32, println));
        Fake(Method(mockFile32, flush));
        When(Method(mockArduino, xTaskCreatePinnedToCore)).AlwaysReturn(1);
        Fake(Method(mockArduino, vTaskDelete));

        SECTION("should write delta times to the log file")
        {
            std::vector<unsigned long> deltaTimesNoFlush = {10000, 20000, safeFlushPeriod + 1000, 30000};

            sdCardService.saveDeltaTime(deltaTimesNoFlush);

            Verify(Method(mockFile32, println)).Exactly(4_Times);
            Verify(Method(mockArduino, xTaskCreatePinnedToCore).Using(Ne(nullptr), StrEq("saveDeltaTimeTask"), Eq(expectedStackSize), Ne(nullptr), Eq(1U), Any(), Eq(0))).Once();

            SECTION("without flushing if last delta time does not exceed safeFlushPeriod")
            {
                sdCardService.saveDeltaTime(deltaTimesNoFlush);

                VerifyNoOtherInvocations(Method(mockFile32, flush));
            }

            SECTION("with flush if last delta time exceeds safeFlushPeriod")
            {
                std::vector<unsigned long> deltaTimesFlush = {10000, 20000, 30000, safeFlushPeriod + 1};

                sdCardService.saveDeltaTime(deltaTimesFlush);

                Verify(Method(mockFile32, flush)).Once();
            }
        }

        SECTION("should handle empty delta times vector")
        {
            std::vector<unsigned long> emptyDeltaTimes;

            sdCardService.saveDeltaTime(emptyDeltaTimes);

            VerifyNoOtherInvocations(Method(mockFile32, println));
            VerifyNoOtherInvocations(Method(mockArduino, xTaskCreatePinnedToCore));
        }

        SECTION("should handle log file not open")
        {
            When(Method(mockFile32, isOpen)).Return(false);

            sdCardService.saveDeltaTime({10000});

            // No methods should be called since the log file is not open
            VerifyNoOtherInvocations(Method(mockFile32, println));
            VerifyNoOtherInvocations(Method(mockArduino, xTaskCreatePinnedToCore));
        }

        SECTION("should delete the task after saving delta times")
        {
            sdCardService.saveDeltaTime({10000});

            Verify(Method(mockArduino, vTaskDelete).Using(nullptr)).Once();
        }
    }

    SECTION("isLogFileOpen method should return isOpen method on File32")
    {
        SdCardService sdCardService;

        mockSdFat32.Reset();
        mockFile32.Reset();
        mockArduino.Reset();
        When(Method(mockFile32, close)).AlwaysReturn(true);
        Fake(Method(mockSdFat32, end));

        When(Method(mockFile32, isOpen)).Return(true);
        REQUIRE(sdCardService.isLogFileOpen() == true);

        When(Method(mockFile32, isOpen)).Return(false);
        REQUIRE(sdCardService.isLogFileOpen() == false);
    }

    SECTION("Destructor should close the log file and end SD card session")
    {
        mockSdFat32.Reset();
        mockFile32.Reset();
        mockArduino.Reset();
        When(Method(mockFile32, close)).AlwaysReturn(true);
        Fake(Method(mockSdFat32, end));

        {
            SdCardService sdCardService;
        }

        Verify(Method(mockFile32, close)).Once();
        Verify(Method(mockSdFat32, end)).Once();
    }
}
// NOLINTEND(readability-magic-numbers)