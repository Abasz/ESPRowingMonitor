#include "regressor.service.h"

LinearRegressorService::LinearRegressorService() {}

void LinearRegressorService::resetData()
{
    sumX = 0;
    sumXSquare = 0;
    sumY = 0;
    sumYSquare = 0;
    sumXY = 0;
    count = 0;
}

void LinearRegressorService::addToDataset(volatile unsigned long long y)
{
    auto x = sumY + y;
    sumX += x;
    sumXSquare += x * x;
    sumY += y;
    sumYSquare += y * y;
    sumXY += x * y;
    count++;
}

double LinearRegressorService::slope()
{
    if (count > 0 && sumX > 0)
    {
        return (double)(count * sumXY - sumX * sumY) / (double)(count * sumXSquare - sumX * sumX);
    }
    else
    {
        return 0.0;
    }
}

double LinearRegressorService::goodnessOfFit()
{
    // This function returns the R^2 as a goodness of fit indicator
    if (count >= 2 && sumX > 0)
    {
        auto slope = (double)(count * sumXY - sumX * sumY) / (double)(count * sumXSquare - sumX * sumX);
        auto intercept = (double)(sumY - (slope * sumX)) / count;
        auto sse = sumYSquare - (intercept * sumY) - (slope * sumXY);
        auto sst = sumYSquare - (double)(sumY * sumY) / count;
        return 1 - (sse / sst);
    }
    else
    {
        return 0;
    }
}