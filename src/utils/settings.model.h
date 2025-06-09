#pragma once

#include "./configuration.h"

namespace RowerProfile
{
    struct MachineSettings
    {
        float flywheelInertia = Configurations::flywheelInertia;
        float concept2MagicNumber = Configurations::concept2MagicNumber;
    };
}