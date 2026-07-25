#pragma once

#include "fakeit.hpp"

#include "../../../src/peripherals/peripherals.controller.interface.h"           // IWYU pragma: keep
#include "../../../src/rower/stroke.controller.interface.h"                      // IWYU pragma: keep
#include "../../../src/utils/EEPROM/EEPROM.service.interface.h"                  // IWYU pragma: keep
#include "../../../src/utils/configuration.h"                                    // IWYU pragma: keep
#include "../../../src/utils/ota-updater/ota-updater.service.interface.h"        // IWYU pragma: keep
#include "../../../src/utils/power-manager/power-manager.controller.interface.h" // IWYU pragma: keep

class Print;

namespace MainTestDoubles
{
    extern fakeit::Mock<IPeripheralsController> peripheralsControllerMock;
    extern fakeit::Mock<IPowerManagerController> powerManagerControllerMock;
    extern fakeit::Mock<IStrokeController> strokeControllerMock;
    extern fakeit::Mock<IEEPROMService> eepromServiceMock;
    extern fakeit::Mock<IOtaUpdaterService> otaUpdaterServiceMock;
}

extern IPeripheralsController &peripheralController;
extern IPowerManagerController &powerManagerController;
extern IStrokeController &strokeController;
extern IEEPROMService &eepromService;
extern IOtaUpdaterService &otaService;

void printPrefix(Print *_logOutput, int logLevel);