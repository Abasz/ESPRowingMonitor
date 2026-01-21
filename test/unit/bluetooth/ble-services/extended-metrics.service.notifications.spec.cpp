// NOLINTBEGIN(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while, modernize-type-traits)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

#include <array>
#include <ranges>
#include <span>
#include <string>
#include <vector>

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_range_equals.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"
#include "fakeit.hpp"

#include "./esp_err.h"

#include "../../include/Arduino.h"
#include "../../include/NimBLEDevice.h"

#include "../../../../src/peripherals/bluetooth/ble-services/extended-metrics.service.h"
#include "../../../../src/peripherals/bluetooth/ble.enums.h"
#include "../../../../src/utils/configuration.h"

using namespace fakeit;

TEST_CASE("ExtendedMetricBleService broadcast", "[ble-service]")
{
    mockNimBLEServer.Reset();
    mockArduino.Reset();

    Mock<NimBLECharacteristic> mockExtendedMetricsCharacteristic;
    Mock<NimBLEService> mockExtendedMetricService;

    When(OverloadedMethod(mockNimBLEServer, createService, NimBLEService * (const std::string))).AlwaysReturn(&mockExtendedMetricService.get());

    When(OverloadedMethod(mockExtendedMetricService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int))).AlwaysReturn(&mockExtendedMetricsCharacteristic.get());

    Fake(OverloadedMethod(mockExtendedMetricsCharacteristic, setValue, void(const unsigned short)));
    Fake(Method(mockExtendedMetricsCharacteristic, setCallbacks));

    SECTION("ExtendedMetrics method should")
    {
        const auto secInMicroSec = 1e6;
        const unsigned int recoveryDuration = 4'000'000;
        const unsigned int driveDuration = 3'000'000;
        const Configurations::precision dragCoefficient = 110 / 1e6;
        const Configurations::precision avgStrokePower = 301.123455;

        const unsigned short expectedRecoveryDuration = std::lroundl(recoveryDuration / secInMicroSec * 4'096);
        const unsigned short expectedDriveDuration = std::lroundl(driveDuration / secInMicroSec * 4'096);
        const auto expectedAvgStrokePower = static_cast<short>(std::lround(avgStrokePower));
        const auto expectedDragFactor = static_cast<unsigned short>(std::lround(dragCoefficient * 1e6));
        const auto expectedStackSize = 2'368U;

        Fake(OverloadedMethod(mockExtendedMetricsCharacteristic, setValue, void(const std::array<unsigned char, 8U>)));
        Fake(Method(mockExtendedMetricsCharacteristic, notify));
        Fake(Method(mockArduino, xTaskCreatePinnedToCore));
        Fake(Method(mockArduino, vTaskDelete));

        ExtendedMetricBleService extendedMetricBleService;
        extendedMetricBleService.setup(&mockNimBLEServer.get());

        SECTION("convert recovery and drive duration to a 16bit unsigned short in seconds with a resolution of 4096")
        {
            const auto length = 8U;
            std::array<unsigned char, length> expectedData = {
                static_cast<unsigned char>(expectedAvgStrokePower),
                static_cast<unsigned char>(expectedAvgStrokePower >> 8),

                static_cast<unsigned char>(expectedDriveDuration),
                static_cast<unsigned char>(expectedDriveDuration >> 8),
                static_cast<unsigned char>(expectedRecoveryDuration),
                static_cast<unsigned char>(expectedRecoveryDuration >> 8),

                static_cast<unsigned char>(expectedDragFactor),
                static_cast<unsigned char>(expectedDragFactor >> 8),
            };

            extendedMetricBleService.broadcastExtendedMetrics(avgStrokePower, recoveryDuration, driveDuration, dragCoefficient);

            Verify(OverloadedMethod(mockExtendedMetricsCharacteristic, setValue, void(const std::array<unsigned char, 8U>)).Using(Eq(expectedData))).Once();
        }

        SECTION("notify ExtendedMetrics with the correct binary data")
        {
            const auto length = 8U;
            std::array<unsigned char, length> expectedData = {
                static_cast<unsigned char>(expectedAvgStrokePower),
                static_cast<unsigned char>(expectedAvgStrokePower >> 8),

                static_cast<unsigned char>(expectedDriveDuration),
                static_cast<unsigned char>(expectedDriveDuration >> 8),
                static_cast<unsigned char>(expectedRecoveryDuration),
                static_cast<unsigned char>(expectedRecoveryDuration >> 8),

                static_cast<unsigned char>(expectedDragFactor),
                static_cast<unsigned char>(expectedDragFactor >> 8),
            };

            extendedMetricBleService.broadcastExtendedMetrics(avgStrokePower, recoveryDuration, driveDuration, dragCoefficient);

            Verify(
                Method(mockArduino, xTaskCreatePinnedToCore)
                    .Using(Ne(nullptr), StrEq("notifyExtendedMetrics"), Eq(expectedStackSize), Ne(nullptr), Eq(1U), Any(), Eq(0)))
                .Once();
            Verify(
                OverloadedMethod(mockExtendedMetricsCharacteristic, setValue, void(const std::array<unsigned char, 8U>))
                    .Using(Eq(expectedData)))
                .Once();
        }

        SECTION("delete task")
        {
            mockArduino.ClearInvocationHistory();

            extendedMetricBleService.broadcastExtendedMetrics(avgStrokePower, recoveryDuration, driveDuration, dragCoefficient);

            Verify(Method(mockArduino, vTaskDelete).Using(nullptr)).Once();
        }

        SECTION("trigger ESP_ERR_NOT_FOUND if ExtendedMetricBleService setup() method was not called")
        {
            mockArduino.ClearInvocationHistory();

            ExtendedMetricBleService extendedMetricBleServiceNoSetup;
            Fake(Method(mockArduino, abort));

            REQUIRE_THROWS(extendedMetricBleServiceNoSetup.broadcastExtendedMetrics(avgStrokePower, recoveryDuration, driveDuration, dragCoefficient));

            Verify(Method(mockArduino, abort).Using(ESP_ERR_NOT_FOUND)).Once();
        }
    }

    SECTION("HandleForces method should")
    {
        const auto coreStackSize = 2'240U;
        const std::vector<float> expectedHandleForces{1.1, 3.3, 500.4, 300.4};
        const std::vector<float> expectedBigHandleForces{1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 1.1, 3.3, 10.4, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12, 30.999, 80.323, 500.4, 300.4, 200.8474, 100.12};

        Mock<NimBLECharacteristic> mockHandleForcesCharacteristic;

        When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(100);

        When(OverloadedMethod(mockExtendedMetricService, createCharacteristic, NimBLECharacteristic * (const std::string, const unsigned int)).Using(CommonBleFlags::handleForcesUuid, Any())).AlwaysReturn(&mockHandleForcesCharacteristic.get());
        When(Method(mockHandleForcesCharacteristic, setCallbacks)).Do([&mockHandleForcesCharacteristic](NimBLECharacteristicCallbacks *callbacks)
                                                                      { mockHandleForcesCharacteristic.get().callbacks = callbacks; });

        Fake(OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const std::span<const std::byte> data)));
        Fake(Method(mockHandleForcesCharacteristic, notify));

        Fake(Method(mockArduino, xTaskCreatePinnedToCore));
        Fake(Method(mockArduino, vTaskDelete));

        ExtendedMetricBleService extendedMetricBleService;
        extendedMetricBleService.setup(&mockNimBLEServer.get());
        mockHandleForcesCharacteristic.get().subscribe(0, 1);

        SECTION("calculate stack size for the task")
        {
            SECTION("when MTU is less than handleForces vector size")
            {
                const auto expectedMTU = 100U;
                const auto expectedStackSize = coreStackSize + expectedMTU / 3;

                When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(expectedMTU);

                extendedMetricBleService.broadcastHandleForces(expectedBigHandleForces);

                Verify(
                    Method(mockArduino, xTaskCreatePinnedToCore)
                        .Using(Any(), Any(), Eq(expectedStackSize), Any(), Any(), Any(), Any()))
                    .Once();
            }

            SECTION("when MTU is greater than handleForces vector size")
            {
                const auto expectedMTU = 100U;
                const auto expectedStackSize = coreStackSize + (expectedHandleForces.size() * sizeof(float)) / 3;

                When(Method(mockNimBLEServer, getPeerMTU)).AlwaysReturn(expectedMTU);

                extendedMetricBleService.broadcastHandleForces(expectedHandleForces);

                Verify(
                    Method(mockArduino, xTaskCreatePinnedToCore)
                        .Using(Any(), Any(), Eq(expectedStackSize), Any(), Any(), Any(), Any()))
                    .Once();
            }
        }

        SECTION("should start a task")
        {
            extendedMetricBleService.broadcastHandleForces(expectedHandleForces);

            Verify(
                Method(mockArduino, xTaskCreatePinnedToCore)
                    .Using(Ne(nullptr), StrEq("notifyHandleForces"), Any(), Ne(nullptr), Eq(1U), Any(), Eq(0)))
                .Once();
        }

        SECTION("send")
        {
            const auto expectedMTU = 100U;

            SECTION("one notify when all handleForces plus header can fit within the MTU")
            {
                When(Method(mockNimBLEServer, getPeerMTU)).Return(expectedMTU);

                extendedMetricBleService.broadcastHandleForces(expectedHandleForces);

                Verify(
                    OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const std::span<const std::byte> data)).Matching([expectedSize = 2U + expectedHandleForces.size() * sizeof(float)](const std::span<const std::byte> data)
                                                                                                                                     { return data.size_bytes() == expectedSize; }))
                    .Once();
            }

            SECTION("multiple consecutive notifies when handleForces plus header do not fit within the MTU")
            {
                const auto expectedMTU = 512U;

                for (size_t i = 0; expectedMTU - i > 23; ++i)
                {
                    const unsigned short expectedChunkSize = (expectedMTU - i - 3U - 2U) / sizeof(float);
                    const unsigned char expectedNumberOfNotifies = expectedBigHandleForces.size() / expectedChunkSize;

                    INFO("MTU: " << expectedMTU - i);
                    mockHandleForcesCharacteristic.ClearInvocationHistory();
                    When(Method(mockNimBLEServer, getPeerMTU)).Return(expectedMTU - i);

                    extendedMetricBleService.broadcastHandleForces(expectedBigHandleForces);

                    Verify(
                        OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const std::span<const std::byte> data)).Matching([expectedSize = 2U + expectedChunkSize * sizeof(float)](const std::span<const std::byte> data)
                                                                                                                                         { return data.size_bytes() == expectedSize; }))
                        .Exactly(expectedNumberOfNotifies);

                    Verify(
                        OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const std::span<const std::byte> data)).Matching([expectedSize = 2U + (expectedBigHandleForces.size() % expectedChunkSize) * sizeof(float)](const std::span<const std::byte> data)
                                                                                                                                         { return data.size_bytes() == expectedSize; }))
                        .Exactly(expectedBigHandleForces.size() % expectedChunkSize == 0 ? 0 : 1);
                }
            }
        }

        SECTION("send the total number of chunks and the current chunk id as the first two bytes")
        {
            const auto expectedMTU = 100;
            std::vector<std::vector<std::byte>> results{};

            const unsigned short expectedChunkSize = (expectedMTU - 3U - 2U) / sizeof(float);
            const unsigned char expectedNumberOfNotifies = expectedBigHandleForces.size() / expectedChunkSize + (expectedBigHandleForces.size() % expectedChunkSize == 0 ? 0 : 1);

            When(Method(mockNimBLEServer, getPeerMTU)).Return(expectedMTU);
            Fake(
                OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const std::span<const std::byte> data)).Matching([&results](const std::span<const std::byte> data)
                                                                                                                                 {
                    results.emplace_back(data.begin(), data.end());

                    return true; }));

            extendedMetricBleService.broadcastHandleForces(expectedBigHandleForces);

            for (unsigned char i = 0; i < expectedNumberOfNotifies; ++i)
            {
                INFO("Number of total notifies: " << (int)expectedNumberOfNotifies << " Current notify: " << i + 1U);
                REQUIRE(results[i][0] == std::byte(expectedNumberOfNotifies));
                REQUIRE(results[i][1] == std::byte(i + 1));
            }
        }

        SECTION("correctly chunk handleForces data between notifies")
        {
            const auto expectedMTU = 100;
            std::vector<std::vector<std::byte>> results{};

            const size_t expectedChunkSize = (expectedMTU - 3U - 2U) / sizeof(float);
            const unsigned char expectedNumberOfNotifies = expectedBigHandleForces.size() / expectedChunkSize + (expectedBigHandleForces.size() % expectedChunkSize == 0 ? 0 : 1);

            mockHandleForcesCharacteristic.ClearInvocationHistory();
            When(Method(mockNimBLEServer, getPeerMTU)).Return(expectedMTU);
            Fake(
                OverloadedMethod(mockHandleForcesCharacteristic, setValue, void(const std::span<const std::byte> data)).Matching([&results](const std::span<const std::byte> data)
                                                                                                                                 {
                        results.emplace_back(data.begin(), data.end());
                        return true; }));

            extendedMetricBleService.broadcastHandleForces(expectedBigHandleForces);

            Verify(Method(mockHandleForcesCharacteristic, notify)).Exactly(expectedNumberOfNotifies);

            auto index = 0;
            for (const auto &result : results)
            {
                std::vector<float> parsedHandleForces{};
                const auto floatElements = std::span(result).subspan(2);
                for (auto floatBytes : floatElements | std::views::chunk(sizeof(float)))
                {
                    float value = NAN;
                    std::memcpy(&value, floatBytes.data(), sizeof(float));
                    parsedHandleForces.push_back(value);
                }
                const auto numOfFloats = parsedHandleForces.size();
                const auto beginOffset = expectedChunkSize * index;
                auto view = expectedBigHandleForces | std::views::drop(beginOffset) | std::views::take(numOfFloats);

                const auto current = std::vector<float>(view.begin(), view.end());

                INFO("Current notify: " << index + 1U);
                REQUIRE_THAT(current, Catch::Matchers::RangeEquals(parsedHandleForces, [](float first, float second)
                                                                   { return std::abs(first - second) < 0.00001F; }));
                ++index;
            }
        }

        SECTION("delete task")
        {
            mockArduino.ClearInvocationHistory();

            extendedMetricBleService.broadcastHandleForces(expectedHandleForces);

            Verify(Method(mockArduino, vTaskDelete).Using(nullptr)).Once();
        }

        SECTION("trigger ESP_ERR_NOT_FOUND if ExtendedMetricBleService setup() method was not called")
        {
            mockArduino.ClearInvocationHistory();

            ExtendedMetricBleService extendedMetricBleServiceNoSetup;
            Fake(Method(mockArduino, abort));

            REQUIRE_THROWS(extendedMetricBleServiceNoSetup.broadcastHandleForces(expectedHandleForces));

            Verify(Method(mockArduino, abort).Using(ESP_ERR_NOT_FOUND)).Once();
        }
    }

    SECTION("DeltaTimes method should")
    {
        const auto coreStackSize = 2'368U;
        std::vector<unsigned long> expectedDeltaTimes{10000, 11000, 12000, 11000};
        const auto expectedStackSize = coreStackSize + expectedDeltaTimes.size() * sizeof(unsigned long) / 3;

        Fake(OverloadedMethod(mockExtendedMetricsCharacteristic, setValue, void(const std::span<const std::byte> data)));
        Fake(Method(mockExtendedMetricsCharacteristic, notify));

        Fake(Method(mockArduino, xTaskCreatePinnedToCore));
        Fake(Method(mockArduino, vTaskDelete));

        ExtendedMetricBleService extendedMetricBleService;
        extendedMetricBleService.setup(&mockNimBLEServer.get());

        SECTION("calculate core stack size for the task")
        {
            extendedMetricBleService.broadcastDeltaTimes(expectedDeltaTimes);

            Verify(
                Method(mockArduino, xTaskCreatePinnedToCore)
                    .Using(Any(), Any(), Eq(expectedStackSize), Any(), Any(), Any(), Any()))
                .Once();
        }

        SECTION("start task and notify broadcastDeltaTimes with the correct binary data")
        {
            std::vector<std::byte> results;
            When(OverloadedMethod(mockExtendedMetricsCharacteristic, setValue, void(const std::span<const std::byte> data)))
                .Do([&results](const std::span<const std::byte> data)
                    { std::ranges::copy(data, std::back_inserter(results)); });

            extendedMetricBleService.broadcastDeltaTimes(expectedDeltaTimes);

            Verify(
                Method(mockArduino, xTaskCreatePinnedToCore)
                    .Using(Ne(nullptr), StrEq("notifyDeltaTimes"), Eq(expectedStackSize), Ne(nullptr), Eq(1U), Any(), Eq(0)))
                .Once();
            Verify(
                OverloadedMethod(mockExtendedMetricsCharacteristic, setValue, void(const std::span<const std::byte> data))
                    .Using(Any()))
                .Once();

            std::vector<std::byte> expectedData;
            std::ranges::copy(std::as_bytes(std::span(expectedDeltaTimes)), std::back_inserter(expectedData));
            REQUIRE_THAT(results, Catch::Matchers::Equals(expectedData));
        }

        SECTION("delete task")
        {
            extendedMetricBleService.broadcastDeltaTimes(expectedDeltaTimes);

            Verify(Method(mockArduino, vTaskDelete).Using(nullptr)).Once();
        }

        SECTION("trigger ESP_ERR_NOT_FOUND if ExtendedMetricBleService setup() method was not called")
        {
            mockArduino.ClearInvocationHistory();

            ExtendedMetricBleService extendedMetricBleServiceNoSetup;
            Fake(Method(mockArduino, abort));

            REQUIRE_THROWS(extendedMetricBleServiceNoSetup.broadcastDeltaTimes(expectedDeltaTimes));

            Verify(Method(mockArduino, abort).Using(ESP_ERR_NOT_FOUND)).Once();
        }
    }
}
#pragma GCC diagnostic pop
// NOLINTEND(readability-magic-numbers, readability-function-cognitive-complexity, cppcoreguidelines-avoid-do-while, modernize-type-traits)