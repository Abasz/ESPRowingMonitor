#pragma once

#include "./configuration.h"

namespace RowerProfile
{
    struct MachineSettings
    {
        unsigned char impulsesPerRevolution = Configurations::impulsesPerRevolution;
        float flywheelInertia = Configurations::flywheelInertia;
        float concept2MagicNumber = Configurations::concept2MagicNumber;
        float sprocketRadius = Configurations::sprocketRadius;
    };
}