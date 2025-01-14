// NOLINTBEGIN(readability-magic-numbers)
#include <span>
#include <vector>

#include "./include/catch_amalgamated.hpp"
#include "./include/fakeit.hpp"

#include "./include/Arduino.h"
#include "./include/NimBLEDevice.h"
#include "./include/Update.h"

#include "./include/globals.h"

#include "../../src/utils/ota-updater/ota-updater.service.h"

using namespace fakeit;

TEST_CASE("OtaUpdaterService", "[ota]")
{
    mockUpdate.Reset();
    mockGlobals.Reset();
    mockArduino.Reset();

    const unsigned char blePackageHeaderSize = 3;
    const unsigned int firmwareSize = 765;

    NimBLEAttValue beginRequest{static_cast<unsigned char>(OtaRequestOpCodes::Begin), static_cast<unsigned char>(firmwareSize), static_cast<unsigned char>(firmwareSize >> 8), static_cast<unsigned char>(firmwareSize >> 16), static_cast<unsigned char>(firmwareSize >> 24)};
    NimBLEAttValue invalidBeginRequest{static_cast<unsigned char>(OtaRequestOpCodes::Begin), 0, 2, 250};
    NimBLEAttValue abortRequest{static_cast<unsigned char>(OtaRequestOpCodes::Abort)};
    NimBLEAttValue packageRequest{static_cast<unsigned char>(OtaRequestOpCodes::Package), 1, 1};
    NimBLEAttValue endRequest{static_cast<unsigned char>(OtaRequestOpCodes::End), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    NimBLEAttValue invalidEndRequest{static_cast<unsigned char>(OtaRequestOpCodes::End), 0, 0};
    NimBLEAttValue invalidRequest{4};
    NimBLEAttValue emptyRequest{};

    Mock<NimBLECharacteristic> mockTxCharacteristic;

    Fake(Method(mockGlobals, attachRotationInterrupt));
    Fake(Method(mockGlobals, detachRotationInterrupt));
    Fake(Method(mockGlobals, restartWithDelay));

    Fake(Method(mockArduino, delay));

    When(Method(mockUpdate, isRunning)).AlwaysReturn(false);
    When(Method(mockUpdate, begin)).AlwaysReturn(true);
    When(Method(mockUpdate, hasError)).Return(false);
    Fake(Method(mockUpdate, abort));
    Fake(Method(mockUpdate, errorString));
    Fake(Method(mockUpdate, getError));
    Fake(Method(mockUpdate, write));
    Fake(Method(mockUpdate, remaining));
    Fake(Method(mockUpdate, progress));
    Fake(Method(mockUpdate, size));
    Fake(Method(mockUpdate, setMD5));
    Fake(Method(mockUpdate, end));

    Fake(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)));
    Fake(Method(mockTxCharacteristic, notify));

    OtaUpdaterService otaService;

    otaService.begin(&mockTxCharacteristic.get());

    SECTION("begin method should load txCharacteristic")
    {
        otaService.onData(invalidRequest, 256);

        Verify(Method(mockTxCharacteristic, notify)).Once();
    }

    SECTION("isUpdating method should return update running status")
    {
        const auto expected = true;
        When(Method(mockUpdate, isRunning)).Return(expected);

        const auto result = otaService.isUpdating();

        REQUIRE(result == expected);
    }

    SECTION("onData method")
    {
        SECTION("should not do anything when service is not setup via begin")
        {
            mockTxCharacteristic.ClearInvocationHistory();
            mockUpdate.ClearInvocationHistory();

            OtaUpdaterService otaNoopService;

            otaNoopService.onData(invalidRequest, 256);

            VerifyNoOtherInvocations(mockTxCharacteristic);
            VerifyNoOtherInvocations(mockUpdate);
        }

        SECTION("should send OtaResponseOpCodes::IncorrectFormat response and terminate the request on empty request")
        {
            const auto expectedResult = std::vector<unsigned char>{static_cast<unsigned char>(OtaResponseOpCodes::IncorrectFormat)};
            std::vector<unsigned char> resultResponse;

            When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                             { 
                            const auto temp = std::span<const unsigned char>(data, length);
                            resultResponse.push_back(temp[0]); });

            otaService.onData(emptyRequest, 256);

            Verify(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).Once();
            VerifyNoOtherInvocations(mockUpdate);

            REQUIRE_THAT(resultResponse, Catch::Matchers::Equals(expectedResult));
        }

        SECTION("should send OtaResponseOpCodes::IncorrectFormat response and terminate the request on invalid request OpCode")
        {
            const auto expectedResult = std::vector<unsigned char>{static_cast<unsigned char>(OtaResponseOpCodes::IncorrectFormat)};
            std::vector<unsigned char> resultResponse;

            When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                             { 
                            const auto temp = std::span<const unsigned char>(data, length);
                            resultResponse.push_back(temp[0]); });

            otaService.onData(invalidRequest, 256);

            Verify(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).Once();

            VerifyNoOtherInvocations(mockUpdate);

            REQUIRE_THAT(resultResponse, Catch::Matchers::Equals(expectedResult));
        }

        SECTION("should terminate update on OtaRequestOpCodes::Abort request OpCode and send OtaResponseOpCodes::Ok")
        {
            std::vector<unsigned char> otaResponseOpCodes;
            When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&otaResponseOpCodes](const unsigned char *data, size_t length)
                                                                                                                                             { 
                            const auto temp = std::span<const unsigned char>(data, length);
                            otaResponseOpCodes.insert(cend(otaResponseOpCodes), cbegin(temp), cend(temp)); });

            otaService.onData(abortRequest, 256);

            Verify(Method(mockUpdate, abort)).Once();
            Verify(Method(mockGlobals, attachRotationInterrupt)).Once();
            REQUIRE_THAT(otaResponseOpCodes, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
            REQUIRE(otaResponseOpCodes.at(0) == static_cast<unsigned char>(OtaResponseOpCodes::Ok));
            Verify(Method(mockTxCharacteristic, notify)).Once();
        }

        SECTION("with OtaRequestOpCodes::Begin")
        {
            SECTION("should set MTU")
            {
                const auto expectedMtu = 20 + blePackageHeaderSize + sizeof(OtaRequestOpCodes);
                std::vector<unsigned char> resultResponse;

                When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length))).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                { 
                            const auto temp = std::span<const unsigned char>(data, length);
                            resultResponse.insert(cend(resultResponse), cbegin(temp) + 1, cbegin(temp) + 1 + sizeof(unsigned int)); });

                otaService.onData(beginRequest, expectedMtu);

                const auto mtu = (resultResponse[0] | resultResponse[1] << 8 | resultResponse[2] << 16 | resultResponse[3] << 24) + blePackageHeaderSize + sizeof(OtaRequestOpCodes);

                REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(4));
                REQUIRE(mtu == expectedMtu);
            }

            SECTION("should terminate previous OTA update if already running")
            {
                When(Method(mockUpdate, isRunning)).AlwaysReturn(true);

                otaService.onData(beginRequest, 256);

                Verify(Method(mockUpdate, abort)).Once();
            }

            SECTION("should send OtaResponseOpCodes::IncorrectFormat response and terminate the request on invalid payload")
            {
                const auto expectedResult = static_cast<unsigned char>(OtaResponseOpCodes::IncorrectFormat);
                std::vector<unsigned char> resultResponse;

                When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                                 { 
                            const auto temp = std::span<const unsigned char>(data, length);
                            resultResponse.push_back(temp[0]); });

                otaService.onData(invalidBeginRequest, 256);

                Verify(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).Once();
                Verify(Method(mockUpdate, isRunning)).Any();
                VerifyNoOtherInvocations(mockUpdate);

                REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
                REQUIRE(resultResponse[0] == expectedResult);
            }

            SECTION("on valid OtaRequestOpCodes::Begin request")
            {
                SECTION("should begin OTA update process with correct arguments")
                {
                    otaService.onData(beginRequest, 256);

                    Verify(Method(mockUpdate, begin).Using(firmwareSize)).Once();
                }

                SECTION("when OTA update begin fails")
                {
                    SECTION("should terminate update")
                    {
                        When(Method(mockUpdate, begin)).Return(false);

                        otaService.onData(beginRequest, 256);

                        Verify(Method(mockGlobals, attachRotationInterrupt)).Once();
                        Verify(Method(mockUpdate, abort)).Once();
                    }

                    SECTION("with UPDATE_ERROR_SIZE should send OtaResponseOpCodes::IncorrectFirmwareSize response")
                    {
                        std::vector<unsigned char> resultResponse;

                        When(Method(mockUpdate, begin)).Return(false);
                        When(Method(mockUpdate, getError)).Return(UPDATE_ERROR_SIZE);

                        When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length))).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                        {
                         const auto temp = std::span<const unsigned char>(data, length);
                         resultResponse.insert(cend(resultResponse), cbegin(temp), cend(temp)); });

                        otaService.onData(beginRequest, 256);

                        REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
                        REQUIRE(resultResponse[0] == static_cast<unsigned char>(OtaResponseOpCodes::IncorrectFirmwareSize));
                    }

                    SECTION("with UPDATE_ERROR_NO_PARTITION should send OtaResponseOpCodes::InternalStorageError response")
                    {
                        std::vector<unsigned char> resultResponse;

                        When(Method(mockUpdate, begin)).Return(false);
                        When(Method(mockUpdate, getError)).Return(UPDATE_ERROR_NO_PARTITION);

                        When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length))).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                        {
                         const auto temp = std::span<const unsigned char>(data, length);
                         resultResponse.insert(cend(resultResponse), cbegin(temp), cend(temp)); });

                        otaService.onData(beginRequest, 256);

                        REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
                        REQUIRE(resultResponse[0] == static_cast<unsigned char>(OtaResponseOpCodes::InternalStorageError));
                    }

                    SECTION("with unmapped error should send OtaResponseOpCodes::NotOk response")
                    {
                        std::vector<unsigned char> resultResponse;

                        When(Method(mockUpdate, begin)).Return(false);
                        When(Method(mockUpdate, getError)).Return(UPDATE_ERROR_BAD_ARGUMENT);

                        When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length))).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                        { 
                         const auto temp = std::span<const unsigned char>(data, length);
                         resultResponse.insert(cend(resultResponse), cbegin(temp), cend(temp)); });

                        otaService.onData(beginRequest, 256);

                        REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
                        REQUIRE(resultResponse[0] == static_cast<unsigned char>(OtaResponseOpCodes::NotOk));
                    }
                }

                SECTION("when OTA update begin is successful")
                {
                    SECTION("should detach rotation interrupt")
                    {
                        otaService.onData(beginRequest, 256);

                        Verify(Method(mockGlobals, detachRotationInterrupt)).Once();
                    }

                    SECTION("should send OtaResponseOpCodes::Ok with buffer and per package size")
                    {
                        const auto mtu = 256U;
                        const auto bufferCapacity = 40U;
                        const auto expectedPerPackageSize = mtu - blePackageHeaderSize -
                                                            sizeof(OtaRequestOpCodes);
                        const auto expectedBufferSize = expectedPerPackageSize * bufferCapacity;
                        std::vector<unsigned char> resultResponse;

                        When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length))).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                        {
                         const auto temp = std::span<const unsigned char>(data, length);
                         resultResponse.insert(cend(resultResponse), cbegin(temp), cend(temp)); });

                        otaService.onData(beginRequest, mtu);

                        REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(unsigned int) * 2 + sizeof(OtaResponseOpCodes)));

                        const auto perPackageSize = (resultResponse[1] | resultResponse[2] << 8 | resultResponse[3] << 16 | resultResponse[4] << 24);
                        const auto bufferSize = (resultResponse[1 + sizeof(unsigned int)] | resultResponse[2 + sizeof(unsigned int)] << 8 | resultResponse[3 + sizeof(unsigned int)] << 16 | resultResponse[4 + sizeof(unsigned int)] << 24);

                        REQUIRE(resultResponse[0] == static_cast<unsigned char>(OtaResponseOpCodes::Ok));
                        REQUIRE(perPackageSize == expectedPerPackageSize);
                        REQUIRE(bufferSize == expectedBufferSize);
                    }
                }
            }
        }

        SECTION("with OtaRequestOpCodes::Package")
        {
            SECTION("should send OtaResponseOpCodes::NotOk when OTA updater is not running")
            {
                const auto expectedResult = static_cast<unsigned char>(OtaResponseOpCodes::NotOk);
                std::vector<unsigned char> resultResponse;

                When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                                 { 
                                 const auto temp = std::span<const unsigned char>(data, length);
                                 resultResponse.push_back(temp[0]); });
                When(Method(mockUpdate, isRunning)).AlwaysReturn(false);

                otaService.onData(packageRequest, 256);

                Verify(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).Once();
                Verify(Method(mockUpdate, isRunning)).Any();
                VerifyNoOtherInvocations(mockUpdate);

                REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
                REQUIRE(resultResponse[0] == expectedResult);
            }

            SECTION("on valid OtaRequestOpCodes::Package request")
            {
                const auto mtu = 256U;
                const auto bufferCapacity = 40U;
                const auto expectedBufferSize = (mtu - blePackageHeaderSize - sizeof(OtaRequestOpCodes)) * bufferCapacity;

                std::vector<unsigned char> packageData(expectedBufferSize);
                std::fill(begin(packageData), end(packageData), 1);
                NimBLEAttValue flushPackageRequest{static_cast<unsigned char>(OtaRequestOpCodes::Package)};
                flushPackageRequest.insert(packageData);
                std::vector<unsigned char> resultResponse;

                When(Method(mockUpdate, isRunning)).Return(false).AlwaysReturn(true);
                otaService.onData(beginRequest, mtu);

                SECTION("should write to the buffer until its full")
                {
                    std::vector<unsigned char> packageData(expectedBufferSize - 1);
                    std::fill(begin(packageData), end(packageData), 1);
                    NimBLEAttValue noFlushPackageRequest{static_cast<unsigned char>(OtaRequestOpCodes::Package)};
                    noFlushPackageRequest.insert(packageData);

                    otaService.onData(noFlushPackageRequest, mtu);

                    Verify(Method(mockUpdate, write)).Never();
                }

                SECTION("should flush the buffer to the update partition when its full")
                {
                    const auto mtu = 256U;
                    const auto bufferCapacity = 40U;
                    const auto expectedBufferSize = (mtu - blePackageHeaderSize - sizeof(OtaRequestOpCodes)) * bufferCapacity;

                    std::vector<unsigned char> packageData(expectedBufferSize);
                    std::fill(begin(packageData), end(packageData), 1);
                    NimBLEAttValue noFlushPackageRequest{static_cast<unsigned char>(OtaRequestOpCodes::Package)};
                    noFlushPackageRequest.insert(packageData);

                    When(Method(mockUpdate, isRunning)).Return(false).AlwaysReturn(true);
                    otaService.onData(beginRequest, mtu);

                    otaService.onData(noFlushPackageRequest, mtu);

                    Verify(Method(mockUpdate, write)).AtLeastOnce();
                }

                SECTION("should send OtaResponseOpCodes::Ok after writing to OTA partition")
                {
                    When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                                     { 
                                     const auto temp = std::span<const unsigned char>(data, length);
                                     resultResponse.push_back(temp[0]); });

                    otaService.onData(flushPackageRequest, mtu);

                    Verify(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).Once();

                    REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
                    REQUIRE(resultResponse[0] == static_cast<unsigned char>(OtaResponseOpCodes::Ok));
                }

                SECTION("should send OtaResponseOpCodes::ChecksumError on update partition write error with UPDATE_ERROR_MAGIC_BYTE")
                {
                    When(Method(mockUpdate, hasError)).Return(true);
                    When(Method(mockUpdate, getError)).Return(UPDATE_ERROR_MAGIC_BYTE);
                    When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                                     { 
                                     const auto temp = std::span<const unsigned char>(data, length);
                                     resultResponse.push_back(temp[0]); });

                    otaService.onData(flushPackageRequest, mtu);

                    Verify(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).Once();

                    REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
                    REQUIRE(resultResponse[0] == static_cast<unsigned char>(OtaResponseOpCodes::ChecksumError));
                }

                SECTION("should send OtaResponseOpCodes::InternalStorageError on update partition write error")
                {
                    When(Method(mockUpdate, hasError)).Return(true);
                    When(Method(mockUpdate, getError)).Return(UPDATE_ERROR_ERASE);
                    When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                                     { 
                                     const auto temp = std::span<const unsigned char>(data, length);
                                     resultResponse.push_back(temp[0]); });

                    otaService.onData(flushPackageRequest, mtu);

                    Verify(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).Once();

                    REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
                    REQUIRE(resultResponse[0] == static_cast<unsigned char>(OtaResponseOpCodes::InternalStorageError));
                }
            }
        }
    }

    SECTION("with OtaRequestOpCodes::End")
    {
        SECTION("should send OtaResponseOpCodes::NotOk when OTA updater is not running")
        {
            const auto expectedResult = static_cast<unsigned char>(OtaResponseOpCodes::NotOk);
            std::vector<unsigned char> resultResponse;

            When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                             { 
                             const auto temp = std::span<const unsigned char>(data, length);
                             resultResponse.push_back(temp[0]); });
            When(Method(mockUpdate, isRunning)).AlwaysReturn(false);

            otaService.onData(invalidEndRequest, 256);

            Verify(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).Once();
            Verify(Method(mockUpdate, isRunning)).Any();
            VerifyNoOtherInvocations(mockUpdate);

            REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
            REQUIRE(resultResponse[0] == expectedResult);
        }

        SECTION("should send OtaResponseOpCodes::IncorrectFormat response and terminate the request on invalid payload")
        {
            const auto expectedResult = static_cast<unsigned char>(OtaResponseOpCodes::IncorrectFormat);
            std::vector<unsigned char> resultResponse;
            std::vector<unsigned char> md5Hash(ESP_ROM_MD5_DIGEST_LEN + 1);
            std::fill(begin(md5Hash), end(md5Hash), 1);
            NimBLEAttValue tooLongEndRequest{static_cast<unsigned char>(OtaRequestOpCodes::End)};
            tooLongEndRequest.insert(md5Hash);

            When(Method(mockUpdate, isRunning)).AlwaysReturn(true);
            When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                             { 
                             const auto temp = std::span<const unsigned char>(data, length);
                             resultResponse.push_back(temp[0]); });

            otaService.onData(invalidEndRequest, 256);
            otaService.onData(tooLongEndRequest, 256);

            Verify(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).Twice();
            Verify(Method(mockUpdate, isRunning)).Any();
            VerifyNoOtherInvocations(mockUpdate);

            REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes) * 2));
            const auto tooShortResponse = resultResponse[0];
            REQUIRE(tooShortResponse == expectedResult);
            const auto tooLongResponse = resultResponse[1];
            REQUIRE(tooLongResponse == expectedResult);
        }

        SECTION("on valid OtaRequestOpCodes::End request")
        {
            When(Method(mockUpdate, isRunning)).AlwaysReturn(true);

            SECTION("should convert MD5 hash byte array to hex string and set it")
            {
                std::string expectedHash;
                When(Method(mockUpdate, setMD5)).Do([&expectedHash](const char *hash)
                                                    { expectedHash.assign(hash); 
                                                    return true; });

                otaService.onData(endRequest, 256);

                Verify(Method(mockUpdate, setMD5)).Once();
                REQUIRE_THAT("0102030405060708090a0b0c0d0e0f10", Catch::Matchers::Equals(expectedHash));
            }

            SECTION("should flush any remaining data from the buffer")
            {
                otaService.onData(beginRequest, 256);
                otaService.onData(packageRequest, 256);

                otaService.onData(endRequest, 256);

                Verify(Method(mockUpdate, write)).AtLeastOnce();
            }

            SECTION("when installing the new OTA")
            {
                SECTION("fails, should return")
                {
                    When(Method(mockUpdate, end)).AlwaysReturn(false);

                    SECTION("OtaResponseOpCodes::ChecksumError when MD5 hash is invalid")
                    {
                        const auto expectedResult = static_cast<unsigned char>(OtaResponseOpCodes::ChecksumError);
                        std::vector<unsigned char> resultResponse;

                        When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                                         { 
                                         const auto temp = std::span<const unsigned char>(data, length);
                                         resultResponse.push_back(temp[0]); });
                        When(Method(mockUpdate, getError)).Return(UPDATE_ERROR_MD5);

                        otaService.onData(endRequest, 256);

                        Verify(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).Once();

                        REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
                        REQUIRE(resultResponse[0] == expectedResult);
                    }

                    SECTION("OtaResponseOpCodes::NotOk when not an UPDATE_ERROR_MD5 occurs")
                    {
                        const auto expectedResult = static_cast<unsigned char>(OtaResponseOpCodes::NotOk);
                        std::vector<unsigned char> resultResponse;

                        When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                                         { 
                                         const auto temp = std::span<const unsigned char>(data, length);
                                         resultResponse.push_back(temp[0]); });
                        When(Method(mockUpdate, getError)).Return(UPDATE_ERROR_ACTIVATE);

                        otaService.onData(endRequest, 256);

                        Verify(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).Once();

                        REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
                        REQUIRE(resultResponse[0] == expectedResult);
                    }
                }

                SECTION("succeeds")
                {
                    When(Method(mockUpdate, end)).AlwaysReturn(true);

                    SECTION("should return OtaResponseOpCodes::Ok")
                    {
                        const auto expectedResult = static_cast<unsigned char>(OtaResponseOpCodes::Ok);
                        std::vector<unsigned char> resultResponse;

                        When(OverloadedMethod(mockTxCharacteristic, setValue, void(const unsigned char *data, size_t length)).Using(Any(), 1U)).AlwaysDo([&resultResponse](const unsigned char *data, size_t length)
                                                                                                                                                         { 
                                         const auto temp = std::span<const unsigned char>(data, length);
                                         resultResponse.push_back(temp[0]); });

                        otaService.onData(endRequest, 256);

                        REQUIRE_THAT(resultResponse, Catch::Matchers::SizeIs(sizeof(OtaResponseOpCodes)));
                        REQUIRE(resultResponse[0] == expectedResult);
                    }

                    SECTION("should restart device")
                    {
                        otaService.onData(endRequest, 256);

                        Verify(Method(mockGlobals, restartWithDelay)).Once();
                    }
                }
            }
        }
    }
}
// NOLINTEND(readability-magic-numbers)