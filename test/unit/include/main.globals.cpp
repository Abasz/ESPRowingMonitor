
#include <fakeit.hpp>

#include "./main.globals.h"

#include "../../../src/peripherals/peripherals.controller.interface.h"
#include "../../../src/rower/stroke.controller.interface.h"
#include "../../../src/utils/EEPROM/EEPROM.service.interface.h"
#include "../../../src/utils/ota-updater/ota-updater.service.interface.h"
#include "../../../src/utils/power-manager/power-manager.controller.interface.h"

namespace MainTestDoubles
{
    fakeit::Mock<IPeripheralsController> peripheralsControllerMock;
    fakeit::Mock<IPowerManagerController> powerManagerControllerMock;
    fakeit::Mock<IStrokeController> strokeControllerMock;
    fakeit::Mock<IEEPROMService> eepromServiceMock;
    fakeit::Mock<IOtaUpdaterService> otaUpdaterServiceMock;
}

IPeripheralsController &peripheralController = MainTestDoubles::peripheralsControllerMock.get();
IPowerManagerController &powerManagerController = MainTestDoubles::powerManagerControllerMock.get();
IStrokeController &strokeController = MainTestDoubles::strokeControllerMock.get();
IEEPROMService &eepromService = MainTestDoubles::eepromServiceMock.get();
IOtaUpdaterService &otaService = MainTestDoubles::otaUpdaterServiceMock.get();

void printPrefix(Print * /*_logOutput*/, int /*logLevel*/)
{
}