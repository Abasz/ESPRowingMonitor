// NOLINTBEGIN
#pragma once

#include <string>

#include "fakeit.hpp"

typedef unsigned char SdCsPin_t;
typedef int oflag_t;

#define O_RDONLY 0 /* +1 == FREAD */
#define O_WRONLY 1 /* +1 == FWRITE */
#define O_RDWR 2   /* +1 == FREAD|FWRITE */

#define O_READ O_RDONLY
#define O_WRITE O_WRONLY
#define O_AT_END 0x4000
#define O_CREAT 0x0200

#define SD_SCK_MHZ(maxMhz) (1000000UL * (maxMhz))

const unsigned char SHARED_SPI = 0;

class SdSpiConfig
{
public:
    SdSpiConfig(SdCsPin_t cs, unsigned char opt, unsigned int maxSpeed) : csPin(cs), options(opt), maxSck(maxSpeed) {}

    SdCsPin_t csPin;
    unsigned char options;
    unsigned int maxSck;
};

class File32;

class MockFile32
{
public:
    virtual bool isOpen() const = 0;
    virtual bool openNext(File32 dirFile, oflag_t oflag) const = 0;
    virtual bool open(const std::string path, oflag_t oflag) = 0;
    virtual bool close() = 0;
    virtual size_t write(const void *buf, size_t nbyte) = 0;
    virtual void println(unsigned long n) = 0;
    virtual void flush() = 0;
};
extern fakeit::Mock<MockFile32> mockFile32;

class File32
{
public:
    operator bool() const
    {
        return isOpen();
    }
    bool isOpen() const
    {
        return mockFile32.get().isOpen();
    };
    bool openNext(File32 *dirFile, oflag_t oflag) const
    {
        return mockFile32.get().openNext(*dirFile, oflag);
    };
    bool open(const char *path, oflag_t oflag)
    {

        return mockFile32.get().open(std::string(path), oflag);
    };
    bool close()
    {
        return mockFile32.get().close();
    };
    size_t write(const void *buf, size_t nbyte)
    {
        return mockFile32.get().write(buf, nbyte);
    };
    void println(unsigned long n)
    {
        mockFile32.get().println(n);
    };
    void flush()
    {
        mockFile32.get().flush();
    };
};

class MockSdFat32
{
public:
    virtual bool begin(SdSpiConfig spiConfig) = 0;
    virtual File32 open(const char *path, unsigned char mode) = 0;
    virtual void end() = 0;
};
extern fakeit::Mock<MockSdFat32> mockSdFat32;

class SdFat32
{
public:
    bool begin(SdSpiConfig spiConfig)
    {
        return mockSdFat32.get().begin(spiConfig);
    };
    File32 open(const char *path, unsigned char mode)
    {
        return mockSdFat32.get().open(path, mode);
    };
    void end()
    {
        mockSdFat32.get().end();
    };
};
// NOLINTEND
