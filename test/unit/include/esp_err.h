// NOLINTBEGIN
#pragma once

#include "Arduino.h"

#define ESP_ERR_NOT_FOUND 0x105 /*!< Requested resource not found */

#define ESP_ERROR_CHECK(x)      \
    mockArduino.get().abort(x); \
    throw false;
// NOLINTEND