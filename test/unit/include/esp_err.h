// NOLINTBEGIN
#pragma once

#include "Arduino.h"

using esp_err_t = int;

#define ESP_OK 0                /*!< esp_err_t value indicating success (no error) */
#define ESP_FAIL -1             /*!< Generic esp_err_t code indicating failure */
#define ESP_ERR_NOT_FOUND 0x105 /*!< Requested resource not found */

#define ESP_ERROR_CHECK(x)      \
    mockArduino.get().abort(x); \
    throw false;
// NOLINTEND