#pragma once

#include "Arduino.h"

#include "../utils/configuration.h"
#include "../utils/settings.model.h"
#include "./flywheel.service.interface.h"

class FlywheelService final : public IFlywheelService
{
    Configurations::precision angularDisplacementPerImpulse = (2 * PI) / RowerProfile::Defaults::impulsesPerRevolution;

    unsigned short rotationDebounceTimeMin = RowerProfile::Defaults::rotationDebounceTimeMin;

    volatile unsigned long lastDeltaTime = 0;
    volatile unsigned long cleanDeltaTime = 0;
    volatile unsigned long lastRawImpulseTime = 0;
    volatile unsigned long lastCleanImpulseTime = 0;
    volatile double totalAngularDisplacement = 0;

    volatile unsigned long impulseCount = 0UL;
    volatile unsigned long long totalTime = 0ULL;

    volatile bool isDataChanged = false;

public:
    void setup(RowerProfile::MachineSettings newMachineSettings, RowerProfile::SensorSignalSettings newSensorSignalSettings) override;
    [[nodiscard]] bool hasDataChanged() const override;
    RowingDataModels::FlywheelData getData() override;
    void processRotation(unsigned long now) override;
};