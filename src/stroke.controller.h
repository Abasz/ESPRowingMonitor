#include "stroke.service.h"

class StrokeController
{
    StrokeService &strokeService;

    unsigned int previousRevCount = 0;

    StrokeModel::CscData cscData{
        0UL,
        0U,
        0UL,
        0U,
        0U,
        0};

public:
    StrokeController(StrokeService &_strokeService);

    void begin() const;
    void update();
    unsigned int getPreviousRevCount() const;
    void setPreviousRevCount();
    unsigned long getLastRevTime() const;
    unsigned int getRevCount() const;
    unsigned long getLastStrokeTime() const;
    unsigned short getStrokeCount() const;
    unsigned int getDeltaTime() const;
    double getDriveDuration() const;
    unsigned int getAvgStrokePower() const;
    byte getDragFactor() const;
};