namespace StrokeModel
{
    struct CscData
    {
        unsigned long lastRevTime;
        unsigned int revCount;
        unsigned long lastStrokeTime;
        unsigned short strokeCount;
        unsigned long rawImpulseTime;
        unsigned int driveDuration;
        unsigned int recoveryDuration;
        double distance;
        double avgStrokePower;
        double dragCoefficient;
        unsigned int rawDeltaImpulseTime;
        unsigned int cleanDeltaImpulseTime;
    };
}