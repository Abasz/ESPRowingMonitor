#include "stroke.service.h"

class StrokeController
{
    StrokeService &strokeService;

    unsigned int previousRevCount = 0;
    unsigned int previousStrokeCount = 0U;
    unsigned long lastDeltaRead = 0;

    StrokeModel::CscData cscData{
        0UL,
        0U,
        0UL,
        0U,
        0U,
        0U,
        0};

public:
    StrokeController(StrokeService &_strokeService);

    void begin() const;
    void update();
    unsigned int getPreviousRevCount() const;
    void setPreviousRevCount();
    unsigned int getPreviousStrokeCount() const;
    void setPreviousStrokeCount();
    unsigned long getLastRevTime() const;
    unsigned int getRevCount() const;
    unsigned long getLastStrokeTime() const;
    unsigned short getStrokeCount() const;
    unsigned int getDistance() const;
    double getRecoveryDuration() const;
    double getDriveDuration() const;
    short getAvgStrokePower() const;
    byte getDragFactor() const;
};