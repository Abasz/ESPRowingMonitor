#pragma once

#include "stroke.service.h"

class StrokeController
{
    StrokeService strokeService;

    unsigned long lastRevReadTime = 0;

    StrokeModel::CscData cscData{
        0UL,
        0U,
        0UL,
        0U,
        0U,
        {}};

public:
    StrokeController();

    void begin() const;
    void readCscData();
    unsigned long getLastRevReadTime() const;
    void setLastRevReadTime();
    unsigned long getLastRevTime() const;
    unsigned int getRevCount() const;
    unsigned long getLastStrokeTime() const;
    unsigned short getStrokeCount() const;
    unsigned int getDeltaTime() const;
    double getDragCoefficient() const;
    void processRotation(unsigned long now);
};