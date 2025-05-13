// NOLINTBEGIN
#pragma once

#include "fakeit.hpp"

#define UPDATE_ERROR_OK (0)
#define UPDATE_ERROR_WRITE (1)
#define UPDATE_ERROR_ERASE (2)
#define UPDATE_ERROR_READ (3)
#define UPDATE_ERROR_SPACE (4)
#define UPDATE_ERROR_SIZE (5)
#define UPDATE_ERROR_STREAM (6)
#define UPDATE_ERROR_MD5 (7)
#define UPDATE_ERROR_MAGIC_BYTE (8)
#define UPDATE_ERROR_ACTIVATE (9)
#define UPDATE_ERROR_NO_PARTITION (10)
#define UPDATE_ERROR_BAD_ARGUMENT (11)
#define UPDATE_ERROR_ABORT (12)

#define ESP_ROM_MD5_DIGEST_LEN 16

class UpdateClass
{
public:
    UpdateClass();

    /*
      Call this to check the space needed for the update
      Will return false if there is not enough space
    */
    virtual bool begin(size_t size) { return true; };

    /*
      Writes a buffer to the flash and increments the address
      Returns the amount written
    */
    virtual size_t write(unsigned char *data, size_t len) { return 0; };

    /*
      If all bytes are written
      this call will write the config to eboot
      and return true
      If there is already an update running but is not finished and !evenIfRemaining
      or there is an error
      this will clear everything and return false
      the last error is available through getError()
      evenIfRemaining is helpfull when you update without knowing the final size first
    */
    virtual bool end(bool evenIfRemaining = false) { return true; };

    /*
      Aborts the running update
    */
    virtual void abort() {};

    virtual const char *errorString() { return 0; };

    /*
      sets the expected MD5 for the firmware (hexString)
    */
    virtual bool setMD5(const char *expected_md5) { return true; };

    // Helpers
    virtual bool hasError() { return false; };
    virtual unsigned char getError() { return 0; };
    virtual bool isRunning() { return true; };
    virtual size_t progress() { return 0; };
    virtual size_t remaining() { return 0; };
    virtual size_t size() { return 0; };
};

extern fakeit::Mock<UpdateClass> mockUpdate;
extern UpdateClass &Update;
// NOLINTEND
