namespace StrokeModel
{
    struct CscData
    {
        unsigned long lastRevTime;
        unsigned int revCount;
        unsigned long lastStrokeTime;
        unsigned short strokeCount;
        unsigned int deltaTime;
        unsigned long rawRevTime;
        unsigned int driveDuration;
        double avgStrokePower;
        double dragCoefficient;
    };
}