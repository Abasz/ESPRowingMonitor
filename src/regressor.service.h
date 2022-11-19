class LinearRegressorService
{
    volatile unsigned long long sumX = 0;
    volatile unsigned long long sumXSquare = 0;
    volatile unsigned long long sumY = 0;
    volatile unsigned long long sumYSquare = 0;
    volatile unsigned long long sumXY = 0;
    volatile unsigned long long count = 0;

public:
    LinearRegressorService();

    void resetData();
    void addToDataset(unsigned long long y);
    double slope() const;
    double goodnessOfFit() const;
};