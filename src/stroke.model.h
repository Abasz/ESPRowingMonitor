namespace StrokeModel
{
    struct CscData
    {
        unsigned long lastRevTime;
        unsigned int revCount;
        unsigned long lastStrokeTime;
        unsigned short strokeCount;
        unsigned int deltaTime;
        unsigned int cleanDeltaTime;
        unsigned long rawRevTime;
        unsigned int driveDuration;
        double distance;
        double avgStrokePower;
        double dragCoefficient;
    };
}