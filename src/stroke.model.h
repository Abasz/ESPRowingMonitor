namespace StrokeModel
{
    struct CscData
    {
        unsigned long lastRevTime;
        unsigned int revCount;
        unsigned long lastStrokeTime;
        unsigned short strokeCount;
        unsigned int deltaTime;
        double dragFactor;
    };
}