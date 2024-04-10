#pragma once

#include <vector>

#include "SdFat.h"

using std::vector;

class SdCardService
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
    void setup();
    void saveDeltaTime(const vector<unsigned long> &deltaTime);
    bool isLogFileOpen() const;
};