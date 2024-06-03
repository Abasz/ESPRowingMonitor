#pragma once

#include <vector>

#include "SdFat.h"

#include "./sd-card.service.interface.h"

using std::vector;

class SdCardService final : public ISdCardService
{
    SdFat32 sd;
    File32 logFile;

    struct SdCardTaskParameters
    {
        File32 &logFile;
        vector<unsigned long> deltaTimes{};
    } sdCardTaskParameters;

    static void saveDeltaTimeTask(void *parameters);
    void initSdCard();

public:
    explicit SdCardService();
    ~SdCardService();

    SdCardService(const SdCardService &) = delete;
    SdCardService &operator=(const SdCardService &) = delete;
    SdCardService(SdCardService &&) = delete;
    SdCardService &operator=(SdCardService &&) = delete;

    void setup() override;
    void saveDeltaTime(const vector<unsigned long> &deltaTime) override;
    bool isLogFileOpen() const override;
};