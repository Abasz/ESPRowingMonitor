#include "../utils/configuration.h"
#include "flywheel.service.h"
#include "stroke.service.h"

class StrokeController
{
    StrokeService &strokeService;
    FlywheelService &flywheelService;

    unsigned int previousRevCount = 0;
    unsigned int previousStrokeCount = 0U;

    RowingDataModels::RowingMetrics rowerState{
        0.0,
        0ULL,
        0ULL,
        0U,
        0U,
        0U,
        0.0,
        0.0,
        std::vector<Configurations::precision>{}};

    RowingDataModels::FlywheelData flywheelData{
        0UL,
        0UL,
        0ULL,
        0UL,
        0UL};

public:
    StrokeController(StrokeService &_strokeService, FlywheelService &_flywheelService);

    static void begin();
    void update();

    const RowingDataModels::RowingMetrics &getAllData() const;
    unsigned int getPreviousRevCount() const;
    void setPreviousRevCount();
    unsigned int getPreviousStrokeCount() const;
    void setPreviousStrokeCount();

    unsigned long long getLastRevTime() const;
    unsigned int getRevCount() const;
    unsigned long long getLastStrokeTime() const;
    unsigned short getStrokeCount() const;
    Configurations::precision getDistance() const;
    Configurations::precision getRecoveryDuration() const;
    Configurations::precision getDriveDuration() const;
    short getAvgStrokePower() const;
    unsigned char getDragFactor() const;
};