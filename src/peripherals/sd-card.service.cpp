#include "Arduino.h"
#include "ArduinoLog.h"

#include "../utils/configuration.h"
#include "sd-card.service.h"

SdCardService::SdCardService(EEPROMService &_eepromService) : eepromService(_eepromService), sdCardTaskParameters{logFile, {}} {}

void SdCardService::setup()
{
    if (!eepromService.getLogToSdCard())
    {
        Log.verboseln("SD card logging is disabled");

        return;
    }

    Log.verboseln("Initialize SD card");
    initSdCard();
}

void SdCardService::initSdCard()
{
    if (!sd.begin(SdSpiConfig(Configurations::sdCardChipSelectPin, SHARED_SPI, SD_SCK_MHZ(26))))
    {
        Log.errorln("Error while initializing SD Card. May be its not mounted?");

        return;
    }

    File32 root;

    int rootFileCount = 0U;
    if (!root.open("/"))
    {
        Log.errorln("Error while opening root");

        return;
    }

    while (logFile.openNext(&root, O_RDONLY))
    {
        rootFileCount++;
        logFile.close();
    }
    Log.verboseln("Number of files in root: %d", rootFileCount);

    std::string fileName = std::to_string(rootFileCount);
    fileName.append(".txt");

    if (!logFile.open(fileName.c_str(), O_WRITE | O_CREAT | O_AT_END))
    {
        Log.errorln("Error while creating logfile");

        return;
    }

    Log.infoln("Logfile %d.txt was opened", rootFileCount);
}

void SdCardService::saveDeltaTimeTask(void *parameters)
{
    {
        const auto *const params = static_cast<const SdCardService::SdCardTaskParameters *>(parameters);

        if (params->logFile)
        {
            for (const auto &deltaTime : params->deltaTimes)
            {
                params->logFile.println(deltaTime);
            }
            const auto safeFlushPeriod = 30'000U;
            if (params->deltaTimes.back() > safeFlushPeriod)
            {
                params->logFile.flush();
            }
        }
    }
    vTaskDelete(nullptr);
}

void SdCardService::saveDeltaTime(const vector<unsigned long> &deltaTimes)
{
    if (deltaTimes.empty() || !logFile)
    {
        return;
    }

    sdCardTaskParameters.deltaTimes = deltaTimes;

    const auto stackCoreSize = 2048;

    xTaskCreatePinnedToCore(
        saveDeltaTimeTask,
        "saveDeltaTimeTask",
        stackCoreSize,
        &sdCardTaskParameters,
        1,
        NULL,
        0);
}

bool SdCardService::isLogFileOpen() const
{
    return logFile.isOpen();
}