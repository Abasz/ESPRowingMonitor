#include "stroke.service.h"

class StrokeController
{
    StrokeService strokeService;

    unsigned long lastRevReadTime = 0;

public:
    StrokeController();

    void begin() const;
    StrokeModel::CscData getCscData() const;
    unsigned long getLastRevReadTime() const;
    void setLastRevReadTime();
    unsigned long getLastRevTime() const;
    void processRotation(unsigned long now);
};