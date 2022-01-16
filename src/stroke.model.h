#pragma once

namespace StrokeModel
{
    static byte const DRAG_COEFFICIENTS_ARRAY_LENGTH = 5U;

    struct CscData
    {
        unsigned long lastRevTime;
        unsigned int revCount;
        unsigned long lastStrokeTime;
        unsigned short strokeCount;
        unsigned int deltaTime;
        std::array<double, DRAG_COEFFICIENTS_ARRAY_LENGTH> dragCoefficients;
    };
}