#pragma once

#include "../utils/EEPROM/EEPROM.service.interface.h"
#include "../utils/configuration.h"
#include "./flywheel.service.interface.h"
#include "./stroke.controller.interface.h"
#include "./stroke.service.interface.h"

class StrokeController final : public IStrokeController
{
    IStrokeService &strokeService;
    IFlywheelService &flywheelService;
    IEEPROMService &eepromService;

    unsigned int previousRevCount = 0;
    unsigned int previousStrokeCount = 0U;
    unsigned long previousRawImpulseCount = 0U;

    RowingDataModels::RowingMetrics rowerState{
        0.0,
        0ULL,
        0ULL,
        0U,
        0U,
        0U,
        0.0,
        0.0,
        std::vector<float>{},
    };

    RowingDataModels::FlywheelData flywheelData{
        0UL,
        0UL,
        0ULL,
        0UL,
        0UL,
    };

public:
    StrokeController(IStrokeService &_strokeService, IFlywheelService &_flywheelService, IEEPROMService &eepromService);

    void begin() override;
    void update() override;

    const RowingDataModels::RowingMetrics &getAllData() const override;
    unsigned int getPreviousRevCount() const override;
    void setPreviousRevCount() override;
    unsigned int getPreviousStrokeCount() const override;
    void setPreviousStrokeCount() override;

    unsigned long getPreviousRawImpulseCount() const override;
    void setPreviousRawImpulseCount() override;
    unsigned long getRawImpulseCount() const override;
    unsigned long getLastImpulseTime() const override;

    unsigned long getDeltaTime() const override;
    unsigned long long getLastRevTime() const override;
    unsigned int getRevCount() const override;
    unsigned long long getLastStrokeTime() const override;
    unsigned short getStrokeCount() const override;
    Configurations::precision getDistance() const override;
    Configurations::precision getRecoveryDuration() const override;
    Configurations::precision getDriveDuration() const override;
    short getAvgStrokePower() const override;
    unsigned char getDragFactor() const override;
};