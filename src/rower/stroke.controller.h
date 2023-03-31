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
        std::vector<double>{}};

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

    RowingDataModels::RowingMetrics getAllData() const;
    unsigned int getPreviousRevCount() const;
    void setPreviousRevCount();
    unsigned int getPreviousStrokeCount() const;
    void setPreviousStrokeCount();
    unsigned long getRawImpulseTime() const;

    unsigned long long getLastRevTime() const;
    unsigned int getRevCount() const;
    unsigned long long getLastStrokeTime() const;
    unsigned short getStrokeCount() const;
    double getDistance() const;
    double getRecoveryDuration() const;
    double getDriveDuration() const;
    short getAvgStrokePower() const;
    unsigned char getDragFactor() const;
};