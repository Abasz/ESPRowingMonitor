namespace StrokeModel
{
    struct CscData
    {
        unsigned int lastDeltaRevTime;
        unsigned int revCount;
        unsigned int lastDeltaStrokeTime;
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